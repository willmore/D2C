/*
 * utils.c: common, generic utility functions
 *
 * Copyright (C) 2006-2011 Red Hat, Inc.
 * Copyright (C) 2006 Daniel P. Berrange
 * Copyright (C) 2006, 2007 Binary Karma
 * Copyright (C) 2006 Shuveb Hussain
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
 *
 * Author: Daniel P. Berrange <berrange@redhat.com>
 * File created Jul 18, 2007 - Shuveb Hussain <shuveb@binarykarma.com>
 */

#include <config.h>

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <poll.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <sys/time.h>
#if HAVE_MMAP
# include <sys/mman.h>
#endif
#include <string.h>
#include <signal.h>
#include <termios.h>
#include "c-ctype.h"

#ifdef HAVE_PATHS_H
# include <paths.h>
#endif
#include <netdb.h>
#ifdef HAVE_GETPWUID_R
# include <pwd.h>
# include <grp.h>
#endif
#if HAVE_CAPNG
# include <cap-ng.h>
#endif
#if defined HAVE_MNTENT_H && defined HAVE_GETMNTENT_R
# include <mntent.h>
#endif

#include "dirname.h"
#include "virterror_internal.h"
#include "logging.h"
#include "event.h"
#include "ignore-value.h"
#include "buf.h"
#include "util.h"
#include "memory.h"
#include "threads.h"
#include "verify.h"
#include "files.h"

#ifndef NSIG
# define NSIG 32
#endif

verify(sizeof(gid_t) <= sizeof (unsigned int) &&
       sizeof(uid_t) <= sizeof (unsigned int));

#define VIR_FROM_THIS VIR_FROM_NONE

#define virUtilError(code, ...)                                            \
        virReportErrorHelper(NULL, VIR_FROM_NONE, code, __FILE__,          \
                             __FUNCTION__, __LINE__, __VA_ARGS__)

/* Like read(), but restarts after EINTR */
ssize_t
saferead(int fd, void *buf, size_t count)
{
    size_t nread = 0;
    while (count > 0) {
        ssize_t r = read(fd, buf, count);
        if (r < 0 && errno == EINTR)
            continue;
        if (r < 0)
            return r;
        if (r == 0)
            return nread;
        buf = (char *)buf + r;
        count -= r;
        nread += r;
    }
    return nread;
}

/* Like write(), but restarts after EINTR */
ssize_t
safewrite(int fd, const void *buf, size_t count)
{
    size_t nwritten = 0;
    while (count > 0) {
        ssize_t r = write(fd, buf, count);

        if (r < 0 && errno == EINTR)
            continue;
        if (r < 0)
            return r;
        if (r == 0)
            return nwritten;
        buf = (const char *)buf + r;
        count -= r;
        nwritten += r;
    }
    return nwritten;
}

#ifdef HAVE_POSIX_FALLOCATE
int safezero(int fd, int flags ATTRIBUTE_UNUSED, off_t offset, off_t len)
{
    return posix_fallocate(fd, offset, len);
}
#else

# ifdef HAVE_MMAP
int safezero(int fd, int flags ATTRIBUTE_UNUSED, off_t offset, off_t len)
{
    int r;
    char *buf;

    /* memset wants the mmap'ed file to be present on disk so create a
     * sparse file
     */
    r = ftruncate(fd, offset + len);
    if (r < 0)
        return -1;

    buf = mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, offset);
    if (buf == MAP_FAILED)
        return -1;

    memset(buf, 0, len);
    munmap(buf, len);

    return 0;
}

# else /* HAVE_MMAP */

int safezero(int fd, int flags ATTRIBUTE_UNUSED, off_t offset, off_t len)
{
    int r;
    char *buf;
    unsigned long long remain, bytes;

    if (lseek(fd, offset, SEEK_SET) < 0)
        return -1;

    /* Split up the write in small chunks so as not to allocate lots of RAM */
    remain = len;
    bytes = 1024 * 1024;

    r = VIR_ALLOC_N(buf, bytes);
    if (r < 0) {
        errno = ENOMEM;
        return -1;
    }

    while (remain) {
        if (bytes > remain)
            bytes = remain;

        r = safewrite(fd, buf, bytes);
        if (r < 0) {
            VIR_FREE(buf);
            return -1;
        }

        /* safewrite() guarantees all data will be written */
        remain -= bytes;
    }
    VIR_FREE(buf);
    return 0;
}
# endif /* HAVE_MMAP */
#endif /* HAVE_POSIX_FALLOCATE */

int virFileStripSuffix(char *str,
                       const char *suffix)
{
    int len = strlen(str);
    int suffixlen = strlen(suffix);

    if (len < suffixlen)
        return 0;

    if (!STREQ(str + len - suffixlen, suffix))
        return 0;

    str[len-suffixlen] = '\0';

    return 1;
}

char *
virArgvToString(const char *const *argv)
{
    int len, i;
    char *ret, *p;

    for (len = 1, i = 0; argv[i]; i++)
        len += strlen(argv[i]) + 1;

    if (VIR_ALLOC_N(ret, len) < 0)
        return NULL;
    p = ret;

    for (i = 0; argv[i]; i++) {
        if (i != 0)
            *(p++) = ' ';

        strcpy(p, argv[i]);
        p += strlen(argv[i]);
    }

    *p = '\0';

    return ret;
}

int virSetNonBlock(int fd) {
#ifndef WIN32
    int flags;
    if ((flags = fcntl(fd, F_GETFL)) < 0)
        return -1;
    flags |= O_NONBLOCK;
    if ((fcntl(fd, F_SETFL, flags)) < 0)
        return -1;
#else
    unsigned long flag = 1;

    /* This is actually Gnulib's replacement rpl_ioctl function.
     * We can't call ioctlsocket directly in any case.
     */
    if (ioctl (fd, FIONBIO, (void *) &flag) == -1)
        return -1;
#endif
    return 0;
}


#ifndef WIN32

int virSetCloseExec(int fd) {
    int flags;
    if ((flags = fcntl(fd, F_GETFD)) < 0)
        return -1;
    flags |= FD_CLOEXEC;
    if ((fcntl(fd, F_SETFD, flags)) < 0)
        return -1;
    return 0;
}


# if HAVE_CAPNG
static int virClearCapabilities(void)
{
    int ret;

    capng_clear(CAPNG_SELECT_BOTH);

    if ((ret = capng_apply(CAPNG_SELECT_BOTH)) < 0) {
        virUtilError(VIR_ERR_INTERNAL_ERROR,
                     _("cannot clear process capabilities %d"), ret);
        return -1;
    }

    return 0;
}
# else
static int virClearCapabilities(void)
{
//    VIR_WARN0("libcap-ng support not compiled in, unable to clear capabilities");
    return 0;
}
# endif


/* virFork() - fork a new process while avoiding various race/deadlock conditions

   @pid - a pointer to a pid_t that will receive the return value from
          fork()

   on return from virFork(), if *pid < 0, the fork failed and there is
   no new process. Otherwise, just like fork(), if *pid == 0, it is the
   child process returning, and if *pid > 0, it is the parent.

   Even if *pid >= 0, if the return value from virFork() is < 0, it
   indicates a failure that occurred in the parent or child process
   after the fork. In this case, the child process should call
   _exit(EXIT_FAILURE) after doing any additional error reporting.

 */
int virFork(pid_t *pid) {
# ifdef HAVE_PTHREAD_SIGMASK
    sigset_t oldmask, newmask;
# endif
    struct sigaction sig_action;
    int saved_errno, ret = -1;

    *pid = -1;

    /*
     * Need to block signals now, so that child process can safely
     * kill off caller's signal handlers without a race.
     */
# ifdef HAVE_PTHREAD_SIGMASK
    sigfillset(&newmask);
    if (pthread_sigmask(SIG_SETMASK, &newmask, &oldmask) != 0) {
        saved_errno = errno;
        virReportSystemError(errno,
                             "%s", _("cannot block signals"));
        goto cleanup;
    }
# endif

    /* Ensure we hold the logging lock, to protect child processes
     * from deadlocking on another thread's inherited mutex state */
    virLogLock();

    *pid = fork();
    saved_errno = errno; /* save for caller */

    /* Unlock for both parent and child process */
    virLogUnlock();

    if (*pid < 0) {
# ifdef HAVE_PTHREAD_SIGMASK
        /* attempt to restore signal mask, but ignore failure, to
           avoid obscuring the fork failure */
        ignore_value (pthread_sigmask(SIG_SETMASK, &oldmask, NULL));
# endif
        virReportSystemError(saved_errno,
                             "%s", _("cannot fork child process"));
        goto cleanup;
    }

    if (*pid) {

        /* parent process */

# ifdef HAVE_PTHREAD_SIGMASK
        /* Restore our original signal mask now that the child is
           safely running */
        if (pthread_sigmask(SIG_SETMASK, &oldmask, NULL) != 0) {
            saved_errno = errno; /* save for caller */
            virReportSystemError(errno, "%s", _("cannot unblock signals"));
            goto cleanup;
        }
# endif
        ret = 0;

    } else {

        /* child process */

        int logprio;
        int i;

        /* Remove any error callback so errors in child now
           get sent to stderr where they stand a fighting chance
           of being seen / logged */
        virSetErrorFunc(NULL, NULL);
        virSetErrorLogPriorityFunc(NULL);

        /* Make sure any hook logging is sent to stderr, since child
         * process may close the logfile FDs */
        logprio = virLogGetDefaultPriority();
        virLogReset();
        virLogSetDefaultPriority(logprio);

        /* Clear out all signal handlers from parent so nothing
           unexpected can happen in our child once we unblock
           signals */
        sig_action.sa_handler = SIG_DFL;
        sig_action.sa_flags = 0;
        sigemptyset(&sig_action.sa_mask);

        for (i = 1; i < NSIG; i++) {
            /* Only possible errors are EFAULT or EINVAL
               The former wont happen, the latter we
               expect, so no need to check return value */

            sigaction(i, &sig_action, NULL);
        }

# ifdef HAVE_PTHREAD_SIGMASK
        /* Unmask all signals in child, since we've no idea
           what the caller's done with their signal mask
           and don't want to propagate that to children */
        sigemptyset(&newmask);
        if (pthread_sigmask(SIG_SETMASK, &newmask, NULL) != 0) {
            saved_errno = errno; /* save for caller */
            virReportSystemError(errno, "%s", _("cannot unblock signals"));
            goto cleanup;
        }
# endif
        ret = 0;
    }

cleanup:
    if (ret < 0)
        errno = saved_errno;
    return ret;
}

/*
 * @argv argv to exec
 * @envp optional environment to use for exec
 * @keepfd options fd_ret to keep open for child process
 * @retpid optional pointer to store child process pid
 * @infd optional file descriptor to use as child input, otherwise /dev/null
 * @outfd optional pointer to communicate output fd behavior
 *        outfd == NULL : Use /dev/null
 *        *outfd == -1  : Use a new fd
 *        *outfd != -1  : Use *outfd
 * @errfd optional pointer to communcate error fd behavior. See outfd
 * @flags possible combination of the following:
 *        VIR_EXEC_NONE     : Default function behavior
 *        VIR_EXEC_NONBLOCK : Set child process output fd's as non-blocking
 *        VIR_EXEC_DAEMON   : Daemonize the child process (don't use directly,
 *                            use virExecDaemonize wrapper)
 * @hook optional virExecHook function to call prior to exec
 * @data data to pass to the hook function
 * @pidfile path to use as pidfile for daemonized process (needs DAEMON flag)
 */
static int
__virExec(const char *const*argv,
          const char *const*envp,
          const fd_set *keepfd,
          pid_t *retpid,
          int infd, int *outfd, int *errfd,
          int flags,
          virExecHook hook,
          void *data,
          char *pidfile)
{
    pid_t pid;
    int null, i, openmax;
    int pipeout[2] = {-1,-1};
    int pipeerr[2] = {-1,-1};
    int childout = -1;
    int childerr = -1;
    int tmpfd;

    if ((null = open("/dev/null", O_RDWR)) < 0) {
        virReportSystemError(errno,
                             _("cannot open %s"),
                             "/dev/null");
        goto cleanup;
    }

    if (outfd != NULL) {
        if (*outfd == -1) {
            if (pipe(pipeout) < 0) {
                virReportSystemError(errno,
                                     "%s", _("cannot create pipe"));
                goto cleanup;
            }

            if ((flags & VIR_EXEC_NONBLOCK) &&
                virSetNonBlock(pipeout[0]) == -1) {
                virReportSystemError(errno,
                                     "%s", _("Failed to set non-blocking file descriptor flag"));
                goto cleanup;
            }

            if (virSetCloseExec(pipeout[0]) == -1) {
                virReportSystemError(errno,
                                     "%s", _("Failed to set close-on-exec file descriptor flag"));
                goto cleanup;
            }

            childout = pipeout[1];
        } else {
            childout = *outfd;
        }
    } else {
        childout = null;
    }

    if (errfd != NULL) {
        if (*errfd == -1) {
            if (pipe(pipeerr) < 0) {
                virReportSystemError(errno,
                                     "%s", _("Failed to create pipe"));
                goto cleanup;
            }

            if ((flags & VIR_EXEC_NONBLOCK) &&
                virSetNonBlock(pipeerr[0]) == -1) {
                virReportSystemError(errno,
                                     "%s", _("Failed to set non-blocking file descriptor flag"));
                goto cleanup;
            }

            if (virSetCloseExec(pipeerr[0]) == -1) {
                virReportSystemError(errno,
                                     "%s", _("Failed to set close-on-exec file descriptor flag"));
                goto cleanup;
            }

            childerr = pipeerr[1];
        } else {
            childerr = *errfd;
        }
    } else {
        childerr = null;
    }

    int forkRet = virFork(&pid);

    if (pid < 0) {
        goto cleanup;
    }

    if (pid) { /* parent */
        VIR_FORCE_CLOSE(null);
        if (outfd && *outfd == -1) {
            VIR_FORCE_CLOSE(pipeout[1]);
            *outfd = pipeout[0];
        }
        if (errfd && *errfd == -1) {
            VIR_FORCE_CLOSE(pipeerr[1]);
            *errfd = pipeerr[0];
        }

        if (forkRet < 0) {
            goto cleanup;
        }

        *retpid = pid;
        return 0;
    }

    /* child */

    if (forkRet < 0) {
        /* The fork was sucessful, but after that there was an error
         * in the child (which was already logged).
        */
        goto fork_error;
    }

    openmax = sysconf (_SC_OPEN_MAX);
    for (i = 3; i < openmax; i++)
        if (i != infd &&
            i != null &&
            i != childout &&
            i != childerr &&
            (!keepfd || i >= FD_SETSIZE || !FD_ISSET(i, keepfd))) {
            tmpfd = i;
            VIR_FORCE_CLOSE(tmpfd);
        }

    if (dup2(infd >= 0 ? infd : null, STDIN_FILENO) < 0) {
        virReportSystemError(errno,
                             "%s", _("failed to setup stdin file handle"));
        goto fork_error;
    }
    if (childout > 0 &&
        dup2(childout, STDOUT_FILENO) < 0) {
        virReportSystemError(errno,
                             "%s", _("failed to setup stdout file handle"));
        goto fork_error;
    }
    if (childerr > 0 &&
        dup2(childerr, STDERR_FILENO) < 0) {
        virReportSystemError(errno,
                             "%s", _("failed to setup stderr file handle"));
        goto fork_error;
    }

    if (infd != STDIN_FILENO)
        VIR_FORCE_CLOSE(infd);
    VIR_FORCE_CLOSE(null);
    if (childout > STDERR_FILENO) {
        tmpfd = childout;   /* preserve childout value */
        VIR_FORCE_CLOSE(tmpfd);
    }
    if (childerr > STDERR_FILENO &&
        childerr != childout) {
        VIR_FORCE_CLOSE(childerr);
    }

    /* Initialize full logging for a while */
    virLogSetFromEnv();

    /* Daemonize as late as possible, so the parent process can detect
     * the above errors with wait* */
    if (flags & VIR_EXEC_DAEMON) {
        if (setsid() < 0) {
            virReportSystemError(errno,
                                 "%s", _("cannot become session leader"));
            goto fork_error;
        }

        if (chdir("/") < 0) {
            virReportSystemError(errno,
                                 "%s", _("cannot change to root directory"));
            goto fork_error;
        }

        pid = fork();
        if (pid < 0) {
            virReportSystemError(errno,
                                 "%s", _("cannot fork child process"));
            goto fork_error;
        }

        if (pid > 0) {
            if (pidfile && virFileWritePidPath(pidfile,pid)) {
                kill(pid, SIGTERM);
                usleep(500*1000);
                kill(pid, SIGTERM);
                virReportSystemError(errno,
                                     _("could not write pidfile %s for %d"),
                                     pidfile, pid);
                goto fork_error;
            }
            _exit(0);
        }
    }

    if (hook) {
        /* virFork reset all signal handlers to the defaults.
         * This is good for the child process, but our hook
         * risks running something that generates SIGPIPE,
         * so we need to temporarily block that again
         */
        struct sigaction waxon, waxoff;
        waxoff.sa_handler = SIG_IGN;
        waxoff.sa_flags = 0;
        memset(&waxon, 0, sizeof(waxon));
        if (sigaction(SIGPIPE, &waxoff, &waxon) < 0) {
            virReportSystemError(errno, "%s",
                                 _("Could not disable SIGPIPE"));
            goto fork_error;
        }

        if ((hook)(data) != 0) {
            VIR_DEBUG0("Hook function failed.");
            goto fork_error;
        }

        if (sigaction(SIGPIPE, &waxon, NULL) < 0) {
            virReportSystemError(errno, "%s",
                                 _("Could not re-enable SIGPIPE"));
            goto fork_error;
        }
    }

    /* The steps above may need todo something privileged, so
     * we delay clearing capabilities until the last minute */
    if ((flags & VIR_EXEC_CLEAR_CAPS) &&
        virClearCapabilities() < 0)
        goto fork_error;

    /* Close logging again to ensure no FDs leak to child */
    virLogReset();

    if (envp)
        execve(argv[0], (char **) argv, (char**)envp);
    else
        execvp(argv[0], (char **) argv);

    virReportSystemError(errno,
                         _("cannot execute binary %s"),
                         argv[0]);

 fork_error:
    virDispatchError(NULL);
    _exit(EXIT_FAILURE);

 cleanup:
    /* This is cleanup of parent process only - child
       should never jump here on error */

    /* NB we don't virUtilError() on any failures here
       because the code which jumped hre already raised
       an error condition which we must not overwrite */
    VIR_FORCE_CLOSE(pipeerr[0]);
    VIR_FORCE_CLOSE(pipeerr[1]);
    VIR_FORCE_CLOSE(pipeout[0]);
    VIR_FORCE_CLOSE(pipeout[1]);
    VIR_FORCE_CLOSE(null);
    return -1;
}

int
virExecWithHook(const char *const*argv,
                const char *const*envp,
                const fd_set *keepfd,
                pid_t *retpid,
                int infd, int *outfd, int *errfd,
                int flags,
                virExecHook hook,
                void *data,
                char *pidfile)
{
    char *argv_str;
    char *envp_str;

    if ((argv_str = virArgvToString(argv)) == NULL) {
        virReportOOMError();
        return -1;
    }

    if (envp) {
        if ((envp_str = virArgvToString(envp)) == NULL) {
            VIR_FREE(argv_str);
            virReportOOMError();
            return -1;
        }
        VIR_DEBUG("%s %s", envp_str, argv_str);
        VIR_FREE(envp_str);
    } else {
        VIR_DEBUG0(argv_str);
    }
    VIR_FREE(argv_str);

    return __virExec(argv, envp, keepfd, retpid, infd, outfd, errfd,
                     flags, hook, data, pidfile);
}

/*
 * See __virExec for explanation of the arguments.
 *
 * Wrapper function for __virExec, with a simpler set of parameters.
 * Used to insulate the numerous callers from changes to __virExec argument
 * list.
 */
int
virExec(const char *const*argv,
        const char *const*envp,
        const fd_set *keepfd,
        pid_t *retpid,
        int infd, int *outfd, int *errfd,
        int flags)
{
    return virExecWithHook(argv, envp, keepfd, retpid,
                           infd, outfd, errfd,
                           flags, NULL, NULL, NULL);
}

/*
 * See __virExec for explanation of the arguments.
 *
 * This function will wait for the intermediate process (between the caller
 * and the daemon) to exit. retpid will be the pid of the daemon, which can
 * be checked for example to see if the daemon crashed immediately.
 *
 * Returns 0 on success
 *         -1 if initial fork failed (will have a reported error)
 *         -2 if intermediate process failed
 *         (won't have a reported error. pending on where the failure
 *          occured and when in the process occured, the error output
 *          could have gone to stderr or the passed errfd).
 */
int virExecDaemonize(const char *const*argv,
                     const char *const*envp,
                     const fd_set *keepfd,
                     pid_t *retpid,
                     int infd, int *outfd, int *errfd,
                     int flags,
                     virExecHook hook,
                     void *data,
                     char *pidfile) {
    int ret;
    int childstat = 0;

    ret = virExecWithHook(argv, envp, keepfd, retpid,
                          infd, outfd, errfd,
                          flags | VIR_EXEC_DAEMON,
                          hook, data, pidfile);

    /* __virExec should have set an error */
    if (ret != 0)
        return -1;

    /* Wait for intermediate process to exit */
    while (waitpid(*retpid, &childstat, 0) == -1 &&
                   errno == EINTR);

    if (childstat != 0) {
        virUtilError(VIR_ERR_INTERNAL_ERROR,
                     _("Intermediate daemon process exited with status %d."),
                     WEXITSTATUS(childstat));
        ret = -2;
    }

    return ret;
}

/**
 * @argv NULL terminated argv to run
 * @status optional variable to return exit status in
 *
 * Run a command without using the shell.
 *
 * If status is NULL, then return 0 if the command run and
 * exited with 0 status; Otherwise return -1
 *
 * If status is not-NULL, then return 0 if the command ran.
 * The status variable is filled with the command exit status
 * and should be checked by caller for success. Return -1
 * only if the command could not be run.
 */
int
virRunWithHook(const char *const*argv,
               virExecHook hook,
               void *data,
               int *status) {
    pid_t childpid;
    int exitstatus, execret, waitret;
    int ret = -1;
    int errfd = -1, outfd = -1;
    char *outbuf = NULL;
    char *errbuf = NULL;
    char *argv_str = NULL;

    if ((argv_str = virArgvToString(argv)) == NULL) {
        virReportOOMError();
        goto error;
    }
    DEBUG0(argv_str);

    if ((execret = __virExec(argv, NULL, NULL,
                             &childpid, -1, &outfd, &errfd,
                             VIR_EXEC_NONE, hook, data, NULL)) < 0) {
        ret = execret;
        goto error;
    }

    if (virPipeReadUntilEOF(outfd, errfd, &outbuf, &errbuf) < 0) {
        while (waitpid(childpid, &exitstatus, 0) == -1 && errno == EINTR)
            ;
        goto error;
    }

    if (outbuf)
        DEBUG("Command stdout: %s", outbuf);
    if (errbuf)
        DEBUG("Command stderr: %s", errbuf);

    while ((waitret = waitpid(childpid, &exitstatus, 0) == -1) &&
            errno == EINTR);
    if (waitret == -1) {
        virReportSystemError(errno,
                             _("cannot wait for '%s'"),
                             argv[0]);
        goto error;
    }

    if (status == NULL) {
        errno = EINVAL;
        if (WIFEXITED(exitstatus) && WEXITSTATUS(exitstatus) != 0) {
            virUtilError(VIR_ERR_INTERNAL_ERROR,
                         _("'%s' exited with non-zero status %d and "
                           "signal %d: %s"), argv_str,
                         WIFEXITED(exitstatus) ? WEXITSTATUS(exitstatus) : 0,
                         WIFSIGNALED(exitstatus) ? WTERMSIG(exitstatus) : 0,
                         (errbuf ? errbuf : ""));
            goto error;
        }
    } else {
        *status = exitstatus;
    }

    ret = 0;

  error:
    VIR_FREE(outbuf);
    VIR_FREE(errbuf);
    VIR_FREE(argv_str);
    VIR_FORCE_CLOSE(outfd);
    VIR_FORCE_CLOSE(errfd);
    return ret;
}

#else /* WIN32 */

int virSetCloseExec(int fd ATTRIBUTE_UNUSED)
{
    return -1;
}

int
virRunWithHook(const char *const *argv ATTRIBUTE_UNUSED,
               virExecHook hook ATTRIBUTE_UNUSED,
               void *data ATTRIBUTE_UNUSED,
               int *status)
{
    if (status)
        *status = ENOTSUP;
    else
        virUtilError(VIR_ERR_INTERNAL_ERROR,
                     "%s", _("virRunWithHook is not implemented for WIN32"));
    return -1;
}

int
virExec(const char *const*argv ATTRIBUTE_UNUSED,
        const char *const*envp ATTRIBUTE_UNUSED,
        const fd_set *keepfd ATTRIBUTE_UNUSED,
        int *retpid ATTRIBUTE_UNUSED,
        int infd ATTRIBUTE_UNUSED,
        int *outfd ATTRIBUTE_UNUSED,
        int *errfd ATTRIBUTE_UNUSED,
        int flags ATTRIBUTE_UNUSED)
{
    virUtilError(VIR_ERR_INTERNAL_ERROR,
                 "%s", _("virExec is not implemented for WIN32"));
    return -1;
}

int
virExecWithHook(const char *const*argv ATTRIBUTE_UNUSED,
                const char *const*envp ATTRIBUTE_UNUSED,
                const fd_set *keepfd ATTRIBUTE_UNUSED,
                pid_t *retpid ATTRIBUTE_UNUSED,
                int infd ATTRIBUTE_UNUSED,
                int *outfd ATTRIBUTE_UNUSED,
                int *errfd ATTRIBUTE_UNUSED,
                int flags ATTRIBUTE_UNUSED,
                virExecHook hook ATTRIBUTE_UNUSED,
                void *data ATTRIBUTE_UNUSED,
                char *pidfile ATTRIBUTE_UNUSED)
{
    /* XXX: Some day we can implement pieces of virCommand/virExec on
     * top of _spawn() or CreateProcess(), but we can't implement
     * everything, since mingw completely lacks fork(), so we cannot
     * run hook code in the child.  */
    virUtilError(VIR_ERR_INTERNAL_ERROR,
                 "%s", _("virExec is not implemented for WIN32"));
    return -1;
}

int
virExecDaemonize(const char *const*argv ATTRIBUTE_UNUSED,
                 const char *const*envp ATTRIBUTE_UNUSED,
                 const fd_set *keepfd ATTRIBUTE_UNUSED,
                 pid_t *retpid ATTRIBUTE_UNUSED,
                 int infd ATTRIBUTE_UNUSED,
                 int *outfd ATTRIBUTE_UNUSED,
                 int *errfd ATTRIBUTE_UNUSED,
                 int flags ATTRIBUTE_UNUSED,
                 virExecHook hook ATTRIBUTE_UNUSED,
                 void *data ATTRIBUTE_UNUSED,
                 char *pidfile ATTRIBUTE_UNUSED)
{
    virUtilError(VIR_ERR_INTERNAL_ERROR,
                 "%s", _("virExecDaemonize is not implemented for WIN32"));

    return -1;
}

int
virFork(pid_t *pid)
{
    *pid = -1;
    errno = ENOTSUP;

    return -1;
}

#endif /* WIN32 */

int
virPipeReadUntilEOF(int outfd, int errfd,
                    char **outbuf, char **errbuf) {

    struct pollfd fds[2];
    int i;
    int finished[2];

    fds[0].fd = outfd;
    fds[0].events = POLLIN;
    fds[0].revents = 0;
    finished[0] = 0;
    fds[1].fd = errfd;
    fds[1].events = POLLIN;
    fds[1].revents = 0;
    finished[1] = 0;

    while(!(finished[0] && finished[1])) {

        if (poll(fds, ARRAY_CARDINALITY(fds), -1) < 0) {
            if ((errno == EAGAIN) || (errno == EINTR))
                continue;
            goto pollerr;
        }

        for (i = 0; i < ARRAY_CARDINALITY(fds); ++i) {
            char data[1024], **buf;
            int got, size;

            if (!(fds[i].revents))
                continue;
            else if (fds[i].revents & POLLHUP)
                finished[i] = 1;

            if (!(fds[i].revents & POLLIN)) {
                if (fds[i].revents & POLLHUP)
                    continue;

                virUtilError(VIR_ERR_INTERNAL_ERROR,
                             "%s", _("Unknown poll response."));
                goto error;
            }

            got = read(fds[i].fd, data, sizeof(data));

            if (got == sizeof(data))
                finished[i] = 0;

            if (got == 0) {
                finished[i] = 1;
                continue;
            }
            if (got < 0) {
                if (errno == EINTR)
                    continue;
                if (errno == EAGAIN)
                    break;
                goto pollerr;
            }

            buf = ((fds[i].fd == outfd) ? outbuf : errbuf);
            size = (*buf ? strlen(*buf) : 0);
            if (VIR_REALLOC_N(*buf, size+got+1) < 0) {
                virReportOOMError();
                goto error;
            }
            memmove(*buf+size, data, got);
            (*buf)[size+got] = '\0';
        }
        continue;

    pollerr:
        virReportSystemError(errno,
                             "%s", _("poll error"));
        goto error;
    }

    return 0;

error:
    VIR_FREE(*outbuf);
    VIR_FREE(*errbuf);
    return -1;
}

int
virRun(const char *const*argv,
       int *status) {
    return virRunWithHook(argv, NULL, NULL, status);
}

/* Like gnulib's fread_file, but read no more than the specified maximum
   number of bytes.  If the length of the input is <= max_len, and
   upon error while reading that data, it works just like fread_file.  */
static char *
saferead_lim (int fd, size_t max_len, size_t *length)
{
    char *buf = NULL;
    size_t alloc = 0;
    size_t size = 0;
    int save_errno;

    for (;;) {
        int count;
        int requested;

        if (size + BUFSIZ + 1 > alloc) {
            alloc += alloc / 2;
            if (alloc < size + BUFSIZ + 1)
                alloc = size + BUFSIZ + 1;

            if (VIR_REALLOC_N(buf, alloc) < 0) {
                save_errno = errno;
                break;
            }
        }

        /* Ensure that (size + requested <= max_len); */
        requested = MIN (size < max_len ? max_len - size : 0,
                         alloc - size - 1);
        count = saferead (fd, buf + size, requested);
        size += count;

        if (count != requested || requested == 0) {
            save_errno = errno;
            if (count < 0)
                break;
            buf[size] = '\0';
            *length = size;
            return buf;
        }
    }

    VIR_FREE(buf);
    errno = save_errno;
    return NULL;
}

/* A wrapper around saferead_lim that maps a failure due to
   exceeding the maximum size limitation to EOVERFLOW.  */
int
virFileReadLimFD(int fd, int maxlen, char **buf)
{
    size_t len;
    char *s;

    if (maxlen <= 0) {
        errno = EINVAL;
        return -1;
    }
    s = saferead_lim (fd, maxlen+1, &len);
    if (s == NULL)
        return -1;
    if (len > maxlen || (int)len != len) {
        VIR_FREE(s);
        /* There was at least one byte more than MAXLEN.
           Set errno accordingly. */
        errno = EOVERFLOW;
        return -1;
    }
    *buf = s;
    return len;
}

int virFileReadAll(const char *path, int maxlen, char **buf)
{
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        virReportSystemError(errno, _("Failed to open file '%s'"), path);
        return -1;
    }

    int len = virFileReadLimFD(fd, maxlen, buf);
    VIR_FORCE_CLOSE(fd);
    if (len < 0) {
        virReportSystemError(errno, _("Failed to read file '%s'"), path);
        return -1;
    }

    return len;
}

/* Truncate @path and write @str to it.  If @mode is 0, ensure that
   @path exists; otherwise, use @mode if @path must be created.
   Return 0 for success, nonzero for failure.
   Be careful to preserve any errno value upon failure. */
int virFileWriteStr(const char *path, const char *str, mode_t mode)
{
    int fd;

    if (mode)
        fd = open(path, O_WRONLY|O_TRUNC|O_CREAT, mode);
    else
        fd = open(path, O_WRONLY|O_TRUNC);
    if (fd == -1)
        return -1;

    if (safewrite(fd, str, strlen(str)) < 0) {
        VIR_FORCE_CLOSE(fd);
        return -1;
    }

    /* Use errno from failed close only if there was no write error.  */
    if (VIR_CLOSE(fd) != 0)
        return -1;

    return 0;
}

int virFileMatchesNameSuffix(const char *file,
                             const char *name,
                             const char *suffix)
{
    int filelen = strlen(file);
    int namelen = strlen(name);
    int suffixlen = strlen(suffix);

    if (filelen == (namelen + suffixlen) &&
        STREQLEN(file, name, namelen) &&
        STREQLEN(file + namelen, suffix, suffixlen))
        return 1;
    else
        return 0;
}

int virFileHasSuffix(const char *str,
                     const char *suffix)
{
    int len = strlen(str);
    int suffixlen = strlen(suffix);

    if (len < suffixlen)
        return 0;

    return STRCASEEQ(str + len - suffixlen, suffix);
}

#define SAME_INODE(Stat_buf_1, Stat_buf_2) \
  ((Stat_buf_1).st_ino == (Stat_buf_2).st_ino \
   && (Stat_buf_1).st_dev == (Stat_buf_2).st_dev)

/* Return nonzero if checkLink and checkDest
   refer to the same file.  Otherwise, return 0.  */
int virFileLinkPointsTo(const char *checkLink,
                        const char *checkDest)
{
    struct stat src_sb;
    struct stat dest_sb;

    return (stat (checkLink, &src_sb) == 0
            && stat (checkDest, &dest_sb) == 0
            && SAME_INODE (src_sb, dest_sb));
}



/*
 * Attempt to resolve a symbolic link, returning an
 * absolute path where only the last component is guaranteed
 * not to be a symlink.
 *
 * Return 0 if path was not a symbolic, or the link was
 * resolved. Return -1 with errno set upon error
 */
int virFileResolveLink(const char *linkpath,
                       char **resultpath)
{
    struct stat st;

    *resultpath = NULL;

    /* We don't need the full canonicalization of intermediate
     * directories, if linkpath is absolute and the basename is
     * already a non-symlink.  */
    if (IS_ABSOLUTE_FILE_NAME(linkpath)) {
        if (lstat(linkpath, &st) < 0)
            return -1;

        if (!S_ISLNK(st.st_mode)) {
            if (!(*resultpath = strdup(linkpath)))
                return -1;
            return 0;
        }
    }

    *resultpath = canonicalize_file_name(linkpath);

    return *resultpath == NULL ? -1 : 0;
}

/*
 * Finds a requested executable file in the PATH env. e.g.:
 * "kvm-img" will return "/usr/bin/kvm-img"
 *
 * You must free the result
 */
char *virFindFileInPath(const char *file)
{
    char *path;
    char *pathiter;
    char *pathseg;
    char *fullpath = NULL;

    if (file == NULL)
        return NULL;

    /* if we are passed an absolute path (starting with /), return a
     * copy of that path, after validating that it is executable
     */
    if (IS_ABSOLUTE_FILE_NAME(file)) {
        if (virFileIsExecutable(file))
            return strdup(file);
        else
            return NULL;
    }

    /* copy PATH env so we can tweak it */
    path = getenv("PATH");

    if (path == NULL || (path = strdup(path)) == NULL)
        return NULL;

    /* for each path segment, append the file to search for and test for
     * it. return it if found.
     */
    pathiter = path;
    while ((pathseg = strsep(&pathiter, ":")) != NULL) {
        if (virAsprintf(&fullpath, "%s/%s", pathseg, file) < 0 ||
            virFileIsExecutable(fullpath))
            break;
        VIR_FREE(fullpath);
    }

    VIR_FREE(path);
    return fullpath;
}

bool virFileExists(const char *path)
{
    return access(path, F_OK) == 0;
}

/* Check that a file is regular and has executable bits.
 *
 * Note: In the presence of ACLs, this may return true for a file that
 * would actually fail with EACCES for a given user, or false for a
 * file that the user could actually execute, but setups with ACLs
 * that weird are unusual. */
bool
virFileIsExecutable(const char *file)
{
    struct stat sb;

    /* We would also want to check faccessat if we cared about ACLs,
     * but we don't.  */
    return (stat(file, &sb) == 0 &&
            S_ISREG(sb.st_mode) &&
            (sb.st_mode & 0111) != 0);
}

#ifndef WIN32
/* return -errno on failure, or 0 on success */
static int virFileOperationNoFork(const char *path, int openflags, mode_t mode,
                                  uid_t uid, gid_t gid,
                                  virFileOperationHook hook, void *hookdata,
                                  unsigned int flags) {
    int fd = -1;
    int ret = 0;
    struct stat st;

    if ((fd = open(path, openflags, mode)) < 0) {
        ret = -errno;
        virReportSystemError(errno, _("failed to create file '%s'"),
                             path);
        goto error;
    }
    if (fstat(fd, &st) == -1) {
        ret = -errno;
        virReportSystemError(errno, _("stat of '%s' failed"), path);
        goto error;
    }
    if (((st.st_uid != uid) || (st.st_gid != gid))
        && (fchown(fd, uid, gid) < 0)) {
        ret = -errno;
        virReportSystemError(errno, _("cannot chown '%s' to (%u, %u)"),
                             path, (unsigned int) uid, (unsigned int) gid);
        goto error;
    }
    if ((flags & VIR_FILE_OP_FORCE_PERMS)
        && (fchmod(fd, mode) < 0)) {
        ret = -errno;
        virReportSystemError(errno,
                             _("cannot set mode of '%s' to %04o"),
                             path, mode);
        goto error;
    }
    if ((hook) && ((ret = hook(fd, hookdata)) != 0)) {
        goto error;
    }
    if (VIR_CLOSE(fd) < 0) {
        ret = -errno;
        virReportSystemError(errno, _("failed to close new file '%s'"),
                             path);
        fd = -1;
        goto error;
    }
    fd = -1;
error:
    VIR_FORCE_CLOSE(fd);
    return ret;
}

/* return -errno on failure, or 0 on success */
static int virDirCreateNoFork(const char *path, mode_t mode, uid_t uid, gid_t gid,
                              unsigned int flags) {
    int ret = 0;
    struct stat st;

    if ((mkdir(path, mode) < 0)
        && !((errno == EEXIST) && (flags & VIR_DIR_CREATE_ALLOW_EXIST)))
       {
        ret = -errno;
        virReportSystemError(errno, _("failed to create directory '%s'"),
                             path);
        goto error;
    }

    if (stat(path, &st) == -1) {
        ret = -errno;
        virReportSystemError(errno, _("stat of '%s' failed"), path);
        goto error;
    }
    if (((st.st_uid != uid) || (st.st_gid != gid))
        && (chown(path, uid, gid) < 0)) {
        ret = -errno;
        virReportSystemError(errno, _("cannot chown '%s' to (%u, %u)"),
                             path, (unsigned int) uid, (unsigned int) gid);
        goto error;
    }
    if ((flags & VIR_DIR_CREATE_FORCE_PERMS)
        && (chmod(path, mode) < 0)) {
        ret = -errno;
        virReportSystemError(errno,
                             _("cannot set mode of '%s' to %04o"),
                             path, mode);
        goto error;
    }
error:
    return ret;
}

/* return -errno on failure, or 0 on success */
int virFileOperation(const char *path, int openflags, mode_t mode,
                     uid_t uid, gid_t gid,
                     virFileOperationHook hook, void *hookdata,
                     unsigned int flags) {
    struct stat st;
    pid_t pid;
    int waitret, status, ret = 0;
    int fd;

    if ((!(flags & VIR_FILE_OP_AS_UID))
        || (getuid() != 0)
        || ((uid == 0) && (gid == 0))) {
        return virFileOperationNoFork(path, openflags, mode, uid, gid,
                                      hook, hookdata, flags);
    }

    /* parent is running as root, but caller requested that the
     * file be created as some other user and/or group). The
     * following dance avoids problems caused by root-squashing
     * NFS servers. */

    int forkRet = virFork(&pid);

    if (pid < 0) {
        ret = -errno;
        return ret;
    }

    if (pid) { /* parent */
        /* wait for child to complete, and retrieve its exit code */
        while ((waitret = waitpid(pid, &status, 0) == -1)
               && (errno == EINTR));
        if (waitret == -1) {
            ret = -errno;
            virReportSystemError(errno,
                                 _("failed to wait for child creating '%s'"),
                                 path);
            goto parenterror;
        }
        ret = -WEXITSTATUS(status);
        if (!WIFEXITED(status) || (ret == -EACCES)) {
            /* fall back to the simpler method, which works better in
             * some cases */
            return virFileOperationNoFork(path, openflags, mode, uid, gid,
                                          hook, hookdata, flags);
        }
parenterror:
        return ret;
    }


    /* child */

    if (forkRet < 0) {
        /* error encountered and logged in virFork() after the fork. */
        goto childerror;
    }

    /* set desired uid/gid, then attempt to create the file */

    if ((gid != 0) && (setgid(gid) != 0)) {
        ret = -errno;
        virReportSystemError(errno,
                             _("cannot set gid %u creating '%s'"),
                             (unsigned int) gid, path);
        goto childerror;
    }
    if  ((uid != 0) && (setuid(uid) != 0)) {
        ret = -errno;
        virReportSystemError(errno,
                             _("cannot set uid %u creating '%s'"),
                             (unsigned int) uid, path);
        goto childerror;
    }
    if ((fd = open(path, openflags, mode)) < 0) {
        ret = -errno;
        if (ret != -EACCES) {
            /* in case of EACCES, the parent will retry */
            virReportSystemError(errno,
                                 _("child failed to create file '%s'"),
                                 path);
        }
        goto childerror;
    }
    if (fstat(fd, &st) == -1) {
        ret = -errno;
        virReportSystemError(errno, _("stat of '%s' failed"), path);
        goto childerror;
    }
    if ((st.st_gid != gid)
        && (fchown(fd, -1, gid) < 0)) {
        ret = -errno;
        virReportSystemError(errno, _("cannot chown '%s' to (%u, %u)"),
                             path, (unsigned int) uid, (unsigned int) gid);
        goto childerror;
    }
    if ((flags & VIR_FILE_OP_FORCE_PERMS)
        && (fchmod(fd, mode) < 0)) {
        ret = -errno;
        virReportSystemError(errno,
                             _("cannot set mode of '%s' to %04o"),
                             path, mode);
        goto childerror;
    }
    if ((hook) && ((ret = hook(fd, hookdata)) != 0)) {
        goto childerror;
    }
    if (VIR_CLOSE(fd) < 0) {
        ret = -errno;
        virReportSystemError(errno, _("child failed to close new file '%s'"),
                             path);
        goto childerror;
    }
childerror:
    _exit(ret);

}

/* return -errno on failure, or 0 on success */
int virDirCreate(const char *path, mode_t mode,
                 uid_t uid, gid_t gid, unsigned int flags) {
    struct stat st;
    pid_t pid;
    int waitret;
    int status, ret = 0;

    if ((!(flags & VIR_DIR_CREATE_AS_UID))
        || (getuid() != 0)
        || ((uid == 0) && (gid == 0))
        || ((flags & VIR_DIR_CREATE_ALLOW_EXIST) && (stat(path, &st) >= 0))) {
        return virDirCreateNoFork(path, mode, uid, gid, flags);
    }

    int forkRet = virFork(&pid);

    if (pid < 0) {
        ret = -errno;
        return ret;
    }

    if (pid) { /* parent */
        /* wait for child to complete, and retrieve its exit code */
        while ((waitret = waitpid(pid, &status, 0) == -1)  && (errno == EINTR));
        if (waitret == -1) {
            ret = -errno;
            virReportSystemError(errno,
                                 _("failed to wait for child creating '%s'"),
                                 path);
            goto parenterror;
        }
        ret = -WEXITSTATUS(status);
        if (!WIFEXITED(status) || (ret == -EACCES)) {
            /* fall back to the simpler method, which works better in
             * some cases */
            return virDirCreateNoFork(path, mode, uid, gid, flags);
        }
        if (ret < 0) {
            goto parenterror;
        }
parenterror:
        return ret;
    }

    /* child */

    if (forkRet < 0) {
        /* error encountered and logged in virFork() after the fork. */
        goto childerror;
    }

    /* set desired uid/gid, then attempt to create the directory */

    if ((gid != 0) && (setgid(gid) != 0)) {
        ret = -errno;
        virReportSystemError(errno, _("cannot set gid %u creating '%s'"),
                             (unsigned int) gid, path);
        goto childerror;
    }
    if  ((uid != 0) && (setuid(uid) != 0)) {
        ret = -errno;
        virReportSystemError(errno, _("cannot set uid %u creating '%s'"),
                             (unsigned int) uid, path);
        goto childerror;
    }
    if (mkdir(path, mode) < 0) {
        ret = -errno;
        if (ret != -EACCES) {
            /* in case of EACCES, the parent will retry */
            virReportSystemError(errno, _("child failed to create directory '%s'"),
                                 path);
        }
        goto childerror;
    }
    /* check if group was set properly by creating after
     * setgid. If not, try doing it with chown */
    if (stat(path, &st) == -1) {
        ret = -errno;
        virReportSystemError(errno,
                             _("stat of '%s' failed"), path);
        goto childerror;
    }
    if ((st.st_gid != gid) && (chown(path, -1, gid) < 0)) {
        ret = -errno;
        virReportSystemError(errno,
                             _("cannot chown '%s' to group %u"),
                             path, (unsigned int) gid);
        goto childerror;
    }
    if ((flags & VIR_DIR_CREATE_FORCE_PERMS)
        && chmod(path, mode) < 0) {
        virReportSystemError(errno,
                             _("cannot set mode of '%s' to %04o"),
                             path, mode);
        goto childerror;
    }
childerror:
    _exit(ret);
}

#else /* WIN32 */

/* return -errno on failure, or 0 on success */
int virFileOperation(const char *path ATTRIBUTE_UNUSED,
                     int openflags ATTRIBUTE_UNUSED,
                     mode_t mode ATTRIBUTE_UNUSED,
                     uid_t uid ATTRIBUTE_UNUSED,
                     gid_t gid ATTRIBUTE_UNUSED,
                     virFileOperationHook hook ATTRIBUTE_UNUSED,
                     void *hookdata ATTRIBUTE_UNUSED,
                     unsigned int flags ATTRIBUTE_UNUSED)
{
    virUtilError(VIR_ERR_INTERNAL_ERROR,
                 "%s", _("virFileOperation is not implemented for WIN32"));

    return -1;
}

int virDirCreate(const char *path ATTRIBUTE_UNUSED,
                 mode_t mode ATTRIBUTE_UNUSED,
                 uid_t uid ATTRIBUTE_UNUSED,
                 gid_t gid ATTRIBUTE_UNUSED,
                 unsigned int flags ATTRIBUTE_UNUSED)
{
    virUtilError(VIR_ERR_INTERNAL_ERROR,
                 "%s", _("virDirCreate is not implemented for WIN32"));

    return -1;
}
#endif /* WIN32 */

static int virFileMakePathHelper(char *path) {
    struct stat st;
    char *p = NULL;
    int err;

    if (stat(path, &st) >= 0)
        return 0;

    if ((p = strrchr(path, '/')) == NULL)
        return EINVAL;

    if (p != path) {
        *p = '\0';
        err = virFileMakePathHelper(path);
        *p = '/';
        if (err != 0)
            return err;
    }

    if (mkdir(path, 0777) < 0 && errno != EEXIST) {
        return errno;
    }
    return 0;
}

int virFileMakePath(const char *path)
{
    struct stat st;
    char *parent = NULL;
    char *p;
    int err = 0;

    if (stat(path, &st) >= 0)
        goto cleanup;

    if ((parent = strdup(path)) == NULL) {
        err = ENOMEM;
        goto cleanup;
    }

    if ((p = strrchr(parent, '/')) == NULL) {
        err = EINVAL;
        goto cleanup;
    }

    if (p != parent) {
        *p = '\0';
        if ((err = virFileMakePathHelper(parent)) != 0) {
            goto cleanup;
        }
    }

    if (mkdir(path, 0777) < 0 && errno != EEXIST) {
        err = errno;
        goto cleanup;
    }

cleanup:
    VIR_FREE(parent);
    return err;
}

/* Build up a fully qualfiied path for a config file to be
 * associated with a persistent guest or network */
int virFileBuildPath(const char *dir,
                     const char *name,
                     const char *ext,
                     char *buf,
                     unsigned int buflen)
{
    if ((strlen(dir) + 1 + strlen(name) + (ext ? strlen(ext) : 0) + 1) >= (buflen-1))
        return -1;

    strcpy(buf, dir);
    strcat(buf, "/");
    strcat(buf, name);
    if (ext)
        strcat(buf, ext);
    return 0;
}


int virFileOpenTty(int *ttymaster,
                   char **ttyName,
                   int rawmode)
{
    return virFileOpenTtyAt("/dev/ptmx",
                            ttymaster,
                            ttyName,
                            rawmode);
}

#ifdef __linux__
int virFileOpenTtyAt(const char *ptmx,
                     int *ttymaster,
                     char **ttyName,
                     int rawmode)
{
    int rc = -1;

    if ((*ttymaster = open(ptmx, O_RDWR|O_NOCTTY|O_NONBLOCK)) < 0)
        goto cleanup;

    if (unlockpt(*ttymaster) < 0)
        goto cleanup;

    if (grantpt(*ttymaster) < 0)
        goto cleanup;

    if (rawmode) {
        struct termios ttyAttr;
        if (tcgetattr(*ttymaster, &ttyAttr) < 0)
            goto cleanup;

        cfmakeraw(&ttyAttr);

        if (tcsetattr(*ttymaster, TCSADRAIN, &ttyAttr) < 0)
            goto cleanup;
    }

    if (ttyName) {
        char tempTtyName[PATH_MAX];
        if (ptsname_r(*ttymaster, tempTtyName, sizeof(tempTtyName)) < 0)
            goto cleanup;

        if ((*ttyName = strdup(tempTtyName)) == NULL) {
            errno = ENOMEM;
            goto cleanup;
        }
    }

    rc = 0;

cleanup:
    if (rc != 0)
        VIR_FORCE_CLOSE(*ttymaster);

    return rc;

}
#else
int virFileOpenTtyAt(const char *ptmx ATTRIBUTE_UNUSED,
                     int *ttymaster ATTRIBUTE_UNUSED,
                     char **ttyName ATTRIBUTE_UNUSED,
                     int rawmode ATTRIBUTE_UNUSED)
{
    return -1;
}
#endif

char* virFilePid(const char *dir, const char* name)
{
    char *pidfile;
    if (virAsprintf(&pidfile, "%s/%s.pid", dir, name) < 0)
        return NULL;
    return pidfile;
}

int virFileWritePid(const char *dir,
                    const char *name,
                    pid_t pid)
{
    int rc;
    char *pidfile = NULL;

    if (name == NULL || dir == NULL) {
        rc = EINVAL;
        goto cleanup;
    }

    if ((rc = virFileMakePath(dir)))
        goto cleanup;

    if (!(pidfile = virFilePid(dir, name))) {
        rc = ENOMEM;
        goto cleanup;
    }

    rc = virFileWritePidPath(pidfile, pid);

cleanup:
    VIR_FREE(pidfile);
    return rc;
}

int virFileWritePidPath(const char *pidfile,
                        pid_t pid)
{
    int rc;
    int fd;
    FILE *file = NULL;

    if ((fd = open(pidfile,
                   O_WRONLY | O_CREAT | O_TRUNC,
                   S_IRUSR | S_IWUSR)) < 0) {
        rc = errno;
        goto cleanup;
    }

    if (!(file = VIR_FDOPEN(fd, "w"))) {
        rc = errno;
        VIR_FORCE_CLOSE(fd);
        goto cleanup;
    }

    if (fprintf(file, "%d", pid) < 0) {
        rc = errno;
        goto cleanup;
    }

    rc = 0;

cleanup:
    if (VIR_FCLOSE(file) < 0)
        rc = errno;

    return rc;
}

int virFileReadPid(const char *dir,
                   const char *name,
                   pid_t *pid)
{
    int rc;
    FILE *file;
    char *pidfile = NULL;
    *pid = 0;

    if (name == NULL || dir == NULL) {
        rc = EINVAL;
        goto cleanup;
    }

    if (!(pidfile = virFilePid(dir, name))) {
        rc = ENOMEM;
        goto cleanup;
    }

    if (!(file = fopen(pidfile, "r"))) {
        rc = errno;
        goto cleanup;
    }

    if (fscanf(file, "%d", pid) != 1) {
        rc = EINVAL;
        VIR_FORCE_FCLOSE(file);
        goto cleanup;
    }

    if (VIR_FCLOSE(file) < 0) {
        rc = errno;
        goto cleanup;
    }

    rc = 0;

 cleanup:
    VIR_FREE(pidfile);
    return rc;
}

int virFileDeletePid(const char *dir,
                     const char *name)
{
    int rc = 0;
    char *pidfile = NULL;

    if (name == NULL || dir == NULL) {
        rc = EINVAL;
        goto cleanup;
    }

    if (!(pidfile = virFilePid(dir, name))) {
        rc = ENOMEM;
        goto cleanup;
    }

    if (unlink(pidfile) < 0 && errno != ENOENT)
        rc = errno;

cleanup:
    VIR_FREE(pidfile);
    return rc;
}


/*
 * Creates an absolute path for a potentialy realtive path.
 * Return 0 if the path was not relative, or on success.
 * Return -1 on error.
 *
 * You must free the result.
 */
int virFileAbsPath(const char *path, char **abspath)
{
    char *buf;
    int cwdlen;

    if (path[0] == '/') {
        buf = strdup(path);
        if (buf == NULL)
            return(-1);
    } else {
        buf = getcwd(NULL, 0);
        if (buf == NULL)
            return(-1);

        cwdlen = strlen(buf);
        /* cwdlen includes the null terminator */
        if (VIR_REALLOC_N(buf, cwdlen + strlen(path) + 1) < 0) {
            VIR_FREE(buf);
            errno = ENOMEM;
            return(-1);
        }

        buf[cwdlen] = '/';
        strcpy(&buf[cwdlen + 1], path);
    }

    *abspath = buf;
    return 0;
}

/* Remove spurious / characters from a path. The result must be freed */
char *
virFileSanitizePath(const char *path)
{
    const char *cur = path;
    char *cleanpath;
    int idx = 0;

    cleanpath = strdup(path);
    if (!cleanpath) {
        virReportOOMError();
        return NULL;
    }

    /* Need to sanitize:
     * //           -> //
     * ///          -> /
     * /../foo      -> /../foo
     * /foo///bar/  -> /foo/bar
     */

    /* Starting with // is valid posix, but ///foo == /foo */
    if (cur[0] == '/' && cur[1] == '/' && cur[2] != '/') {
        idx = 2;
        cur += 2;
    }

    /* Sanitize path in place */
    while (*cur != '\0') {
        if (*cur != '/') {
            cleanpath[idx++] = *cur++;
            continue;
        }

        /* Skip all extra / */
        while (*++cur == '/')
            continue;

        /* Don't add a trailing / */
        if (idx != 0 && *cur == '\0')
            break;

        cleanpath[idx++] = '/';
    }
    cleanpath[idx] = '\0';

    return cleanpath;
}

/* Like strtol, but produce an "int" result, and check more carefully.
   Return 0 upon success;  return -1 to indicate failure.
   When END_PTR is NULL, the byte after the final valid digit must be NUL.
   Otherwise, it's like strtol and lets the caller check any suffix for
   validity.  This function is careful to return -1 when the string S
   represents a number that is not representable as an "int". */
int
virStrToLong_i(char const *s, char **end_ptr, int base, int *result)
{
    long int val;
    char *p;
    int err;

    errno = 0;
    val = strtol(s, &p, base);
    err = (errno || (!end_ptr && *p) || p == s || (int) val != val);
    if (end_ptr)
        *end_ptr = p;
    if (err)
        return -1;
    *result = val;
    return 0;
}

/* Just like virStrToLong_i, above, but produce an "unsigned int" value.  */
int
virStrToLong_ui(char const *s, char **end_ptr, int base, unsigned int *result)
{
    unsigned long int val;
    char *p;
    int err;

    errno = 0;
    val = strtoul(s, &p, base);
    err = (errno || (!end_ptr && *p) || p == s || (unsigned int) val != val);
    if (end_ptr)
        *end_ptr = p;
    if (err)
        return -1;
    *result = val;
    return 0;
}

/* Just like virStrToLong_i, above, but produce a "long" value.  */
int
virStrToLong_l(char const *s, char **end_ptr, int base, long *result)
{
    long int val;
    char *p;
    int err;

    errno = 0;
    val = strtol(s, &p, base);
    err = (errno || (!end_ptr && *p) || p == s);
    if (end_ptr)
        *end_ptr = p;
    if (err)
        return -1;
    *result = val;
    return 0;
}

/* Just like virStrToLong_i, above, but produce an "unsigned long" value.  */
int
virStrToLong_ul(char const *s, char **end_ptr, int base, unsigned long *result)
{
    unsigned long int val;
    char *p;
    int err;

    errno = 0;
    val = strtoul(s, &p, base);
    err = (errno || (!end_ptr && *p) || p == s);
    if (end_ptr)
        *end_ptr = p;
    if (err)
        return -1;
    *result = val;
    return 0;
}

/* Just like virStrToLong_i, above, but produce a "long long" value.  */
int
virStrToLong_ll(char const *s, char **end_ptr, int base, long long *result)
{
    long long val;
    char *p;
    int err;

    errno = 0;
    val = strtoll(s, &p, base);
    err = (errno || (!end_ptr && *p) || p == s || (long long) val != val);
    if (end_ptr)
        *end_ptr = p;
    if (err)
        return -1;
    *result = val;
    return 0;
}

/* Just like virStrToLong_i, above, but produce an "unsigned long long" value.  */
int
virStrToLong_ull(char const *s, char **end_ptr, int base, unsigned long long *result)
{
    unsigned long long val;
    char *p;
    int err;

    errno = 0;
    val = strtoull(s, &p, base);
    err = (errno || (!end_ptr && *p) || p == s || (unsigned long long) val != val);
    if (end_ptr)
        *end_ptr = p;
    if (err)
        return -1;
    *result = val;
    return 0;
}

int
virStrToDouble(char const *s,
               char **end_ptr,
               double *result)
{
    double val;
    char *p;
    int err;

    errno = 0;
    val = strtod(s, &p);
    err = (errno || (!end_ptr && *p) || p == s);
    if (end_ptr)
        *end_ptr = p;
    if (err)
        return -1;
    *result = val;
    return 0;
}

/* Convert C from hexadecimal character to integer.  */
int
virHexToBin(unsigned char c)
{
    switch (c) {
    default: return c - '0';
    case 'a': case 'A': return 10;
    case 'b': case 'B': return 11;
    case 'c': case 'C': return 12;
    case 'd': case 'D': return 13;
    case 'e': case 'E': return 14;
    case 'f': case 'F': return 15;
    }
}

/**
 * virSkipSpaces:
 * @str: pointer to the char pointer used
 *
 * Skip potential blanks, this includes space tabs, line feed,
 * carriage returns and also '\\' which can be erronously emitted
 * by xend
 */
void
virSkipSpaces(const char **str)
{
    const char *cur = *str;

    while ((*cur == ' ') || (*cur == '\t') || (*cur == '\n') ||
           (*cur == '\r') || (*cur == '\\'))
        cur++;
    *str = cur;
}

/**
 * virParseNumber:
 * @str: pointer to the char pointer used
 *
 * Parse an unsigned number
 *
 * Returns the unsigned number or -1 in case of error. @str will be
 *         updated to skip the number.
 */
int
virParseNumber(const char **str)
{
    int ret = 0;
    const char *cur = *str;

    if ((*cur < '0') || (*cur > '9'))
        return (-1);

    while (c_isdigit(*cur)) {
        unsigned int c = *cur - '0';

        if ((ret > INT_MAX / 10) ||
            ((ret == INT_MAX / 10) && (c > INT_MAX % 10)))
            return (-1);
        ret = ret * 10 + c;
        cur++;
    }
    *str = cur;
    return (ret);
}


/**
 * virParseVersionString:
 * @str: const char pointer to the version string
 * @version: unsigned long pointer to output the version number
 *
 * Parse an unsigned version number from a version string. Expecting
 * 'major.minor.micro' format, ignoring an optional suffix.
 *
 * The major, minor and micro numbers are encoded into a single version number:
 *
 *   1000000 * major + 1000 * minor + micro
 *
 * Returns the 0 for success, -1 for error.
 */
int
virParseVersionString(const char *str, unsigned long *version)
{
    unsigned int major, minor, micro;
    char *tmp;

    if (virStrToLong_ui(str, &tmp, 10, &major) < 0 || *tmp != '.')
        return -1;

    if (virStrToLong_ui(tmp + 1, &tmp, 10, &minor) < 0 || *tmp != '.')
        return -1;

    if (virStrToLong_ui(tmp + 1, &tmp, 10, &micro) < 0)
        return -1;

    *version = 1000000 * major + 1000 * minor + micro;

    return 0;
}

/**
 * virVasprintf
 *
 * like glibc's vasprintf but makes sure *strp == NULL on failure
 */
int
virVasprintf(char **strp, const char *fmt, va_list list)
{
    int ret;

    if ((ret = vasprintf(strp, fmt, list)) == -1)
        *strp = NULL;

    return ret;
}

/**
 * virAsprintf
 *
 * like glibc's_asprintf but makes sure *strp == NULL on failure
 */
int
virAsprintf(char **strp, const char *fmt, ...)
{
    va_list ap;
    int ret;

    va_start(ap, fmt);
    ret = virVasprintf(strp, fmt, ap);
    va_end(ap);
    return ret;
}

/**
 * virStrncpy
 *
 * A safe version of strncpy.  The last parameter is the number of bytes
 * available in the destination string, *not* the number of bytes you want
 * to copy.  If the destination is not large enough to hold all n of the
 * src string bytes plus a \0, NULL is returned and no data is copied.
 * If the destination is large enough to hold the n bytes plus \0, then the
 * string is copied and a pointer to the destination string is returned.
 */
char *
virStrncpy(char *dest, const char *src, size_t n, size_t destbytes)
{
    char *ret;

    if (n > (destbytes - 1))
        return NULL;

    ret = strncpy(dest, src, n);
    /* strncpy NULL terminates iff the last character is \0.  Therefore
     * force the last byte to be \0
     */
    dest[n] = '\0';

    return ret;
}

/**
 * virStrcpy
 *
 * A safe version of strcpy.  The last parameter is the number of bytes
 * available in the destination string, *not* the number of bytes you want
 * to copy.  If the destination is not large enough to hold all n of the
 * src string bytes plus a \0, NULL is returned and no data is copied.
 * If the destination is large enough to hold the source plus \0, then the
 * string is copied and a pointer to the destination string is returned.
 */
char *
virStrcpy(char *dest, const char *src, size_t destbytes)
{
    return virStrncpy(dest, src, strlen(src), destbytes);
}

/* Compare two MAC addresses, ignoring differences in case,
 * as well as leading zeros.
 */
int
virMacAddrCompare (const char *p, const char *q)
{
    unsigned char c, d;
    do {
        while (*p == '0' && c_isxdigit (p[1]))
            ++p;
        while (*q == '0' && c_isxdigit (q[1]))
            ++q;
        c = c_tolower (*p);
        d = c_tolower (*q);

        if (c == 0 || d == 0)
            break;

        ++p;
        ++q;
    } while (c == d);

    if (UCHAR_MAX <= INT_MAX)
        return c - d;

    /* On machines where 'char' and 'int' are types of the same size, the
       difference of two 'unsigned char' values - including the sign bit -
       doesn't fit in an 'int'.  */
    return (c > d ? 1 : c < d ? -1 : 0);
}

/**
 * virParseMacAddr:
 * @str: string representation of MAC address, e.g., "0:1E:FC:E:3a:CB"
 * @addr: 6-byte MAC address
 *
 * Parse a MAC address
 *
 * Return 0 upon success, or -1 in case of error.
 */
int
virParseMacAddr(const char* str, unsigned char *addr)
{
    int i;

    errno = 0;
    for (i = 0; i < VIR_MAC_BUFLEN; i++) {
        char *end_ptr;
        unsigned long result;

        /* This is solely to avoid accepting the leading
         * space or "+" that strtoul would otherwise accept.
         */
        if (!c_isxdigit(*str))
            break;

        result = strtoul(str, &end_ptr, 16);

        if ((end_ptr - str) < 1 || 2 < (end_ptr - str) ||
            (errno != 0) ||
            (0xFF < result))
            break;

        addr[i] = (unsigned char) result;

        if ((i == 5) && (*end_ptr == '\0'))
            return 0;
        if (*end_ptr != ':')
            break;

        str = end_ptr + 1;
    }

    return -1;
}

void virFormatMacAddr(const unsigned char *addr,
                      char *str)
{
    snprintf(str, VIR_MAC_STRING_BUFLEN,
             "%02X:%02X:%02X:%02X:%02X:%02X",
             addr[0], addr[1], addr[2],
             addr[3], addr[4], addr[5]);
    str[VIR_MAC_STRING_BUFLEN-1] = '\0';
}

void virGenerateMacAddr(const unsigned char *prefix,
                        unsigned char *addr)
{
    addr[0] = prefix[0];
    addr[1] = prefix[1];
    addr[2] = prefix[2];
    addr[3] = virRandom(256);
    addr[4] = virRandom(256);
    addr[5] = virRandom(256);
}


int virEnumFromString(const char *const*types,
                      unsigned int ntypes,
                      const char *type)
{
    unsigned int i;
    if (!type)
        return -1;

    for (i = 0 ; i < ntypes ; i++)
        if (STREQ(types[i], type))
            return i;

    return -1;
}

const char *virEnumToString(const char *const*types,
                            unsigned int ntypes,
                            int type)
{
    if (type < 0 || type >= ntypes)
        return NULL;

    return types[type];
}

/* Translates a device name of the form (regex) /^[fhv]d[a-z]+[0-9]*$/
 * into the corresponding index (e.g. sda => 0, hdz => 25, vdaa => 26)
 * Note that any trailing string of digits is simply ignored.
 * @param name The name of the device
 * @return name's index, or -1 on failure
 */
int virDiskNameToIndex(const char *name) {
    const char *ptr = NULL;
    int idx = 0;
    static char const* const drive_prefix[] = {"fd", "hd", "vd", "sd", "xvd", "ubd"};
    unsigned int i;

    for (i = 0; i < ARRAY_CARDINALITY(drive_prefix); i++) {
        if (STRPREFIX(name, drive_prefix[i])) {
            ptr = name + strlen(drive_prefix[i]);
            break;
        }
    }

    if (!ptr)
        return -1;

    for (i = 0; *ptr; i++) {
        idx = (idx + (i < 1 ? 0 : 1)) * 26;

        if (!c_islower(*ptr))
            break;

        idx += *ptr - 'a';
        ptr++;
    }

    /* Count the trailing digits.  */
    size_t n_digits = strspn(ptr, "0123456789");
    if (ptr[n_digits] != '\0')
        return -1;

    return idx;
}

char *virIndexToDiskName(int idx, const char *prefix)
{
    char *name = NULL;
    int i, k, offset;

    if (idx < 0) {
        virUtilError(VIR_ERR_INTERNAL_ERROR,
                     _("Disk index %d is negative"), idx);
        return NULL;
    }

    for (i = 0, k = idx; k >= 0; ++i, k = k / 26 - 1) { }

    offset = strlen(prefix);

    if (VIR_ALLOC_N(name, offset + i + 1)) {
        virReportOOMError();
        return NULL;
    }

    strcpy(name, prefix);
    name[offset + i] = '\0';

    for (i = i - 1, k = idx; k >= 0; --i, k = k / 26 - 1) {
        name[offset + i] = 'a' + (k % 26);
    }

    return name;
}

#ifndef AI_CANONIDN
# define AI_CANONIDN 0
#endif

/* Who knew getting a hostname could be so delicate.  In Linux (and Unices
 * in general), many things depend on "hostname" returning a value that will
 * resolve one way or another.  In the modern world where networks frequently
 * come and go this is often being hard-coded to resolve to "localhost".  If
 * it *doesn't* resolve to localhost, then we would prefer to have the FQDN.
 * That leads us to 3 possibilities:
 *
 * 1)  gethostname() returns an FQDN (not localhost) - we return the string
 *     as-is, it's all of the information we want
 * 2)  gethostname() returns "localhost" - we return localhost; doing further
 *     work to try to resolve it is pointless
 * 3)  gethostname() returns a shortened hostname - in this case, we want to
 *     try to resolve this to a fully-qualified name.  Therefore we pass it
 *     to getaddrinfo().  There are two possible responses:
 *     a)  getaddrinfo() resolves to a FQDN - return the FQDN
 *     b)  getaddrinfo() resolves to localhost - in this case, the data we got
 *         from gethostname() is actually more useful than what we got from
 *         getaddrinfo().  Return the value from gethostname() and hope for
 *         the best.
 */
char *virGetHostname(virConnectPtr conn ATTRIBUTE_UNUSED)
{
    int r;
    char hostname[HOST_NAME_MAX+1], *result;
    struct addrinfo hints, *info;

    r = gethostname (hostname, sizeof(hostname));
    if (r == -1) {
        virReportSystemError(errno,
                             "%s", _("failed to determine host name"));
        return NULL;
    }
    NUL_TERMINATE(hostname);

    if (STRPREFIX(hostname, "localhost") || strchr(hostname, '.')) {
        /* in this case, gethostname returned localhost (meaning we can't
         * do any further canonicalization), or it returned an FQDN (and
         * we don't need to do any further canonicalization).  Return the
         * string as-is; it's up to callers to check whether "localhost"
         * is allowed.
         */
        result = strdup(hostname);
        goto check_and_return;
    }

    /* otherwise, it's a shortened, non-localhost, hostname.  Attempt to
     * canonicalize the hostname by running it through getaddrinfo
     */

    memset(&hints, 0, sizeof(hints));
    hints.ai_flags = AI_CANONNAME|AI_CANONIDN;
    hints.ai_family = AF_UNSPEC;
    r = getaddrinfo(hostname, NULL, &hints, &info);
    if (r != 0) {
        virUtilError(VIR_ERR_INTERNAL_ERROR,
                     _("getaddrinfo failed for '%s': %s"),
                     hostname, gai_strerror(r));
        return NULL;
    }

    /* Tell static analyzers about getaddrinfo semantics.  */
    sa_assert (info);

    if (info->ai_canonname == NULL ||
        STRPREFIX(info->ai_canonname, "localhost"))
        /* in this case, we tried to canonicalize and we ended up back with
         * localhost.  Ignore the canonicalized name and just return the
         * original hostname
         */
        result = strdup(hostname);
    else
        /* Caller frees this string. */
        result = strdup (info->ai_canonname);

    freeaddrinfo(info);

check_and_return:
    if (result == NULL)
        virReportOOMError();
    return result;
}

/* send signal to a single process */
int virKillProcess(pid_t pid, int sig)
{
    if (pid <= 1) {
        errno = ESRCH;
        return -1;
    }

#ifdef WIN32
    /* Mingw / Windows don't have many signals (AFAIK) */
    switch (sig) {
    case SIGINT:
        /* This does a Ctrl+C equiv */
        if (!GenerateConsoleCtrlEvent(CTRL_C_EVENT, pid)) {
            errno = ESRCH;
            return -1;
        }
        break;

    case SIGTERM:
        /* Since TerminateProcess is closer to SIG_KILL, we do
         * a Ctrl+Break equiv which is more pleasant like the
         * good old unix SIGTERM/HUP
         */
        if (!GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT, pid)) {
            errno = ESRCH;
            return -1;
        }
        break;

    default:
    {
        HANDLE proc;
        proc = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
        if (!proc) {
            errno = ESRCH; /* Not entirely accurate, but close enough */
            return -1;
        }

        /*
         * TerminateProcess is more or less equiv to SIG_KILL, in that
         * a process can't trap / block it
         */
        if (!TerminateProcess(proc, sig)) {
            errno = ESRCH;
            return -1;
        }
        CloseHandle(proc);
    }
    }
    return 0;
#else
    return kill(pid, sig);
#endif
}


static char randomState[128];
static struct random_data randomData;
static virMutex randomLock;

int virRandomInitialize(unsigned int seed)
{
    if (virMutexInit(&randomLock) < 0)
        return -1;

    if (initstate_r(seed,
                    randomState,
                    sizeof(randomState),
                    &randomData) < 0)
        return -1;

    return 0;
}

int virRandom(int max)
{
    int32_t ret;

    virMutexLock(&randomLock);
    random_r(&randomData, &ret);
    virMutexUnlock(&randomLock);

    return (int) ((double)max * ((double)ret / (double)RAND_MAX));
}


#ifdef HAVE_GETPWUID_R
enum {
    VIR_USER_ENT_DIRECTORY,
    VIR_USER_ENT_NAME,
};

static char *virGetUserEnt(uid_t uid,
                           int field)
{
    char *strbuf;
    char *ret;
    struct passwd pwbuf;
    struct passwd *pw = NULL;
    long val = sysconf(_SC_GETPW_R_SIZE_MAX);
    size_t strbuflen = val;

    if (val < 0) {
        virReportSystemError(errno, "%s", _("sysconf failed"));
        return NULL;
    }

    if (VIR_ALLOC_N(strbuf, strbuflen) < 0) {
        virReportOOMError();
        return NULL;
    }

    /*
     * From the manpage (terrifying but true):
     *
     * ERRORS
     *  0 or ENOENT or ESRCH or EBADF or EPERM or ...
     *        The given name or uid was not found.
     */
    if (getpwuid_r(uid, &pwbuf, strbuf, strbuflen, &pw) != 0 || pw == NULL) {
        virReportSystemError(errno,
                             _("Failed to find user record for uid '%u'"),
                             (unsigned int) uid);
        VIR_FREE(strbuf);
        return NULL;
    }

    if (field == VIR_USER_ENT_DIRECTORY)
        ret = strdup(pw->pw_dir);
    else
        ret = strdup(pw->pw_name);

    VIR_FREE(strbuf);
    if (!ret)
        virReportOOMError();

    return ret;
}

char *virGetUserDirectory(uid_t uid)
{
    return virGetUserEnt(uid, VIR_USER_ENT_DIRECTORY);
}

char *virGetUserName(uid_t uid)
{
    return virGetUserEnt(uid, VIR_USER_ENT_NAME);
}


int virGetUserID(const char *name,
                 uid_t *uid)
{
    char *strbuf;
    struct passwd pwbuf;
    struct passwd *pw = NULL;
    long val = sysconf(_SC_GETPW_R_SIZE_MAX);
    size_t strbuflen = val;

    if (val < 0) {
        virReportSystemError(errno, "%s", _("sysconf failed"));
        return -1;
    }

    if (VIR_ALLOC_N(strbuf, strbuflen) < 0) {
        virReportOOMError();
        return -1;
    }

    /*
     * From the manpage (terrifying but true):
     *
     * ERRORS
     *  0 or ENOENT or ESRCH or EBADF or EPERM or ...
     *        The given name or uid was not found.
     */
    if (getpwnam_r(name, &pwbuf, strbuf, strbuflen, &pw) != 0 || pw == NULL) {
        virReportSystemError(errno,
                             _("Failed to find user record for name '%s'"),
                             name);
        VIR_FREE(strbuf);
        return -1;
    }

    *uid = pw->pw_uid;

    VIR_FREE(strbuf);

    return 0;
}


int virGetGroupID(const char *name,
                  gid_t *gid)
{
    char *strbuf;
    struct group grbuf;
    struct group *gr = NULL;
    long val = sysconf(_SC_GETGR_R_SIZE_MAX);
    size_t strbuflen = val;

    if (val < 0) {
        virReportSystemError(errno, "%s", _("sysconf failed"));
        return -1;
    }

    if (VIR_ALLOC_N(strbuf, strbuflen) < 0) {
        virReportOOMError();
        return -1;
    }

    /*
     * From the manpage (terrifying but true):
     *
     * ERRORS
     *  0 or ENOENT or ESRCH or EBADF or EPERM or ...
     *        The given name or uid was not found.
     */
    if (getgrnam_r(name, &grbuf, strbuf, strbuflen, &gr) != 0 || gr == NULL) {
        virReportSystemError(errno,
                             _("Failed to find group record for name '%s'"),
                             name);
        VIR_FREE(strbuf);
        return -1;
    }

    *gid = gr->gr_gid;

    VIR_FREE(strbuf);

    return 0;
}


/* Set the real and effective uid and gid to the given values, and call
 * initgroups so that the process has all the assumed group membership of
 * that uid. return 0 on success, -1 on failure.
 */
int
virSetUIDGID(uid_t uid, gid_t gid)
{
    if (gid > 0) {
        if (setregid(gid, gid) < 0) {
            virReportSystemError(errno,
                                 _("cannot change to '%d' group"),
                                 (unsigned int) gid);
            return -1;
        }
    }

    if (uid > 0) {
# ifdef HAVE_INITGROUPS
        struct passwd pwd, *pwd_result;
        char *buf = NULL;
        size_t bufsize;

        bufsize = sysconf(_SC_GETPW_R_SIZE_MAX);
        if (bufsize == -1)
            bufsize = 16384;

        if (VIR_ALLOC_N(buf, bufsize) < 0) {
            virReportOOMError();
            return -1;
        }
        getpwuid_r(uid, &pwd, buf, bufsize, &pwd_result);
        if (!pwd_result) {
            virReportSystemError(errno,
                                 _("cannot getpwuid_r(%d)"),
                                 (unsigned int) uid);
            VIR_FREE(buf);
            return -1;
        }
        if (initgroups(pwd.pw_name, pwd.pw_gid) < 0) {
            virReportSystemError(errno,
                                 _("cannot initgroups(\"%s\", %d)"),
                                 pwd.pw_name, (unsigned int) pwd.pw_gid);
            VIR_FREE(buf);
            return -1;
        }
        VIR_FREE(buf);
# endif
        if (setreuid(uid, uid) < 0) {
            virReportSystemError(errno,
                                 _("cannot change to uid to '%d'"),
                                 (unsigned int) uid);
            return -1;
        }
    }
    return 0;
}

#else /* HAVE_GETPWUID_R */

char *
virGetUserDirectory(uid_t uid ATTRIBUTE_UNUSED)
{
    virUtilError(VIR_ERR_INTERNAL_ERROR,
                 "%s", _("virGetUserDirectory is not available"));

    return NULL;
}

char *
virGetUserName(uid_t uid ATTRIBUTE_UNUSED)
{
    virUtilError(VIR_ERR_INTERNAL_ERROR,
                 "%s", _("virGetUserName is not available"));

    return NULL;
}

int virGetUserID(const char *name ATTRIBUTE_UNUSED,
                 uid_t *uid ATTRIBUTE_UNUSED)
{
    virUtilError(VIR_ERR_INTERNAL_ERROR,
                 "%s", _("virGetUserID is not available"));

    return 0;
}


int virGetGroupID(const char *name ATTRIBUTE_UNUSED,
                  gid_t *gid ATTRIBUTE_UNUSED)
{
    virUtilError(VIR_ERR_INTERNAL_ERROR,
                 "%s", _("virGetGroupID is not available"));

    return 0;
}

int
virSetUIDGID(uid_t uid ATTRIBUTE_UNUSED,
             gid_t gid ATTRIBUTE_UNUSED)
{
    virUtilError(VIR_ERR_INTERNAL_ERROR,
                 "%s", _("virSetUIDGID is not available"));
    return -1;
}
#endif /* HAVE_GETPWUID_R */


#if defined HAVE_MNTENT_H && defined HAVE_GETMNTENT_R
/* search /proc/mounts for mount point of *type; return pointer to
 * malloc'ed string of the path if found, otherwise return NULL
 * with errno set to an appropriate value.
 */
char *virFileFindMountPoint(const char *type)
{
    FILE *f;
    struct mntent mb;
    char mntbuf[1024];
    char *ret = NULL;

    f = setmntent("/proc/mounts", "r");
    if (!f)
        return NULL;

    while (getmntent_r(f, &mb, mntbuf, sizeof(mntbuf))) {
        if (STREQ(mb.mnt_type, type)) {
            ret = strdup(mb.mnt_dir);
            goto cleanup;
        }
    }

    if (!ret)
        errno = ENOENT;

cleanup:
    endmntent(f);

    return ret;
}

#else /* defined HAVE_MNTENT_H && defined HAVE_GETMNTENT_R */

char *
virFileFindMountPoint(const char *type ATTRIBUTE_UNUSED)
{
    errno = ENOSYS;

    return NULL;
}

#endif /* defined HAVE_MNTENT_H && defined HAVE_GETMNTENT_R */

#if defined(UDEVADM) || defined(UDEVSETTLE)
void virFileWaitForDevices(void)
{
# ifdef UDEVADM
    const char *const settleprog[] = { UDEVADM, "settle", NULL };
# else
    const char *const settleprog[] = { UDEVSETTLE, NULL };
# endif
    int exitstatus;

    if (access(settleprog[0], X_OK) != 0)
        return;

    /*
     * NOTE: we ignore errors here; this is just to make sure that any device
     * nodes that are being created finish before we try to scan them.
     * If this fails for any reason, we still have the backup of polling for
     * 5 seconds for device nodes.
     */
    if (virRun(settleprog, &exitstatus) < 0)
    {}
}
#else
void virFileWaitForDevices(void) {}
#endif

int virBuildPathInternal(char **path, ...)
{
    char *path_component = NULL;
    virBuffer buf = VIR_BUFFER_INITIALIZER;
    va_list ap;
    int ret = 0;

    va_start(ap, path);

    path_component = va_arg(ap, char *);
    virBufferAdd(&buf, path_component, -1);

    while ((path_component = va_arg(ap, char *)) != NULL)
    {
        virBufferAddChar(&buf, '/');
        virBufferAdd(&buf, path_component, -1);
    }

    va_end(ap);

    *path = virBufferContentAndReset(&buf);
    if (*path == NULL) {
        ret = -1;
    }

    return ret;
}

/**
 * virTimestamp:
 *
 * Return an allocated string containing the current date and time,
 * followed by ": ".  Return NULL on allocation failure.
 */
char *
virTimestamp(void)
{
    struct timeval cur_time;
    struct tm time_info;
    char timestr[100];
    char *timestamp;

    gettimeofday(&cur_time, NULL);
    localtime_r(&cur_time.tv_sec, &time_info);

    strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S", &time_info);

    if (virAsprintf(&timestamp, "%s.%03d",
                    timestr, (int) cur_time.tv_usec / 1000) < 0) {
        return NULL;
    }

    return timestamp;
}
