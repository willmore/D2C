/*
 * testutils.c: basic test utils
 *
 * Copyright (C) 2005-2011 Red Hat, Inc.
 *
 * See COPYING.LIB for the License of this software
 *
 * Karel Zak <kzak@redhat.com>
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifndef WIN32
# include <sys/wait.h>
#endif
#ifdef HAVE_REGEX_H
# include <regex.h>
#endif
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <limits.h>
#include "testutils.h"
#include "internal.h"
#include "memory.h"
#include "util.h"
#include "threads.h"
#include "virterror_internal.h"
#include "buf.h"
#include "logging.h"

#if TEST_OOM_TRACE
# include <execinfo.h>
#endif

#ifdef HAVE_PATHS_H
# include <paths.h>
#endif

#define GETTIMEOFDAY(T) gettimeofday(T, NULL)
#define DIFF_MSEC(T, U)                                 \
    ((((int) ((T)->tv_sec - (U)->tv_sec)) * 1000000.0 + \
      ((int) ((T)->tv_usec - (U)->tv_usec))) / 1000.0)

#include "files.h"

static unsigned int testDebug = -1;
static unsigned int testVerbose = -1;

static unsigned int testOOM = 0;
static unsigned int testCounter = 0;

double
virtTestCountAverage(double *items, int nitems)
{
    long double sum = 0;
    int i;

    for (i=1; i < nitems; i++)
        sum += items[i];

    return (double) (sum / nitems);
}


void virtTestResult(const char *name, int ret, const char *msg, ...)
{
    va_list vargs;
    va_start(vargs, msg);

    testCounter++;
    if (virTestGetVerbose()) {
        fprintf(stderr, "%3d) %-60s ", testCounter, name);
        if (ret == 0)
            fprintf(stderr, "OK\n");
        else {
            fprintf(stderr, "FAILED\n");
            if (msg) {
                vfprintf(stderr, msg, vargs);
            }
        }
    } else {
        if (testCounter != 1 &&
            !((testCounter-1) % 40)) {
            fprintf(stderr, " %-3d\n", (testCounter-1));
            fprintf(stderr, "      ");
        }
        if (ret == 0)
            fprintf(stderr, ".");
        else
            fprintf(stderr, "!");
    }

    va_end(vargs);
}

/*
 * Runs test and count average time (if the nloops is grater than 1)
 *
 * returns: -1 = error, 0 = success
 */
int
virtTestRun(const char *title, int nloops, int (*body)(const void *data), const void *data)
{
    int i, ret = 0;
    double *ts = NULL;

    testCounter++;

    if (testOOM < 2) {
        if (virTestGetVerbose())
            fprintf(stderr, "%2d) %-65s ... ", testCounter, title);
    }

    if (nloops > 1 && (ts = calloc(nloops,
                                   sizeof(double)))==NULL)
        return -1;

    for (i=0; i < nloops; i++) {
        struct timeval before, after;

        if (ts)
            GETTIMEOFDAY(&before);

        virResetLastError();
        ret = body(data);
        virErrorPtr err = virGetLastError();
        if (err) {
            if (virTestGetVerbose() || virTestGetDebug())
                virDispatchError(NULL);
        }

        if (ret != 0) {
            break;
        }

        if (ts) {
            GETTIMEOFDAY(&after);
            ts[i] = DIFF_MSEC(&after, &before);
        }
    }
    if (testOOM < 2) {
        if (virTestGetVerbose()) {
            if (ret == 0 && ts)
                fprintf(stderr, "OK     [%.5f ms]\n",
                        virtTestCountAverage(ts, nloops));
            else if (ret == 0)
                fprintf(stderr, "OK\n");
            else
                fprintf(stderr, "FAILED\n");
        } else {
            if (testCounter != 1 &&
                !((testCounter-1) % 40)) {
                fprintf(stderr, " %-3d\n", (testCounter-1));
                fprintf(stderr, "      ");
            }
            if (ret == 0)
                fprintf(stderr, ".");
            else
                fprintf(stderr, "!");
        }
    }

    free(ts);
    return ret;
}

/* Read FILE into buffer BUF of length BUFLEN.
   Upon any failure, or if FILE appears to contain more than BUFLEN bytes,
   diagnose it and return -1, but don't bother trying to preserve errno.
   Otherwise, return the number of bytes copied into BUF. */
int virtTestLoadFile(const char *file,
                     char **buf,
                     int buflen) {
    FILE *fp = fopen(file, "r");
    struct stat st;
    char *tmp = *buf;
    int len, tmplen = buflen;

    if (!fp) {
        fprintf (stderr, "%s: failed to open: %s\n", file, strerror(errno));
        return -1;
    }

    if (fstat(fileno(fp), &st) < 0) {
        fprintf (stderr, "%s: failed to fstat: %s\n", file, strerror(errno));
        VIR_FORCE_FCLOSE(fp);
        return -1;
    }

    if (st.st_size > (buflen-1)) {
        fprintf (stderr, "%s: larger than buffer (> %d)\n", file, buflen-1);
        VIR_FORCE_FCLOSE(fp);
        return -1;
    }

    (*buf)[0] = '\0';
    if (st.st_size) {
        /* read the file line by line */
        while (fgets(tmp, tmplen, fp) != NULL) {
            len = strlen(tmp);
            /* remove trailing backslash-newline pair */
            if (len >= 2 && tmp[len-2] == '\\' && tmp[len-1] == '\n') {
                len -= 2;
                tmp[len] = '\0';
            }
            /* advance the temporary buffer pointer */
            tmp += len;
            tmplen -= len;
        }
        if (ferror(fp)) {
            fprintf (stderr, "%s: read failed: %s\n", file, strerror(errno));
            VIR_FORCE_FCLOSE(fp);
            return -1;
        }
    }

    VIR_FORCE_FCLOSE(fp);
    return strlen(*buf);
}

#ifndef WIN32
static
void virtTestCaptureProgramExecChild(const char *const argv[],
                                     int pipefd) {
    int i;
    int open_max;
    int stdinfd = -1;
    const char *const env[] = {
        "LANG=C",
# if WITH_DRIVER_MODULES
        "LIBVIRT_DRIVER_DIR=" TEST_DRIVER_DIR,
# endif
        NULL
    };

    if ((stdinfd = open("/dev/null", O_RDONLY)) < 0)
        goto cleanup;

    open_max = sysconf (_SC_OPEN_MAX);
    for (i = 0; i < open_max; i++) {
        if (i != stdinfd &&
            i != pipefd) {
            int tmpfd = i;
            VIR_FORCE_CLOSE(tmpfd);
        }
    }

    if (dup2(stdinfd, STDIN_FILENO) != STDIN_FILENO)
        goto cleanup;
    if (dup2(pipefd, STDOUT_FILENO) != STDOUT_FILENO)
        goto cleanup;
    if (dup2(pipefd, STDERR_FILENO) != STDERR_FILENO)
        goto cleanup;

    /* SUS is crazy here, hence the cast */
    execve(argv[0], (char *const*)argv, (char *const*)env);

 cleanup:
    VIR_FORCE_CLOSE(stdinfd);
}

int virtTestCaptureProgramOutput(const char *const argv[],
                                 char **buf,
                                 int buflen) {
    int pipefd[2];

    if (pipe(pipefd) < 0)
        return -1;

    int pid = fork();
    switch (pid) {
    case 0:
        VIR_FORCE_CLOSE(pipefd[0]);
        virtTestCaptureProgramExecChild(argv, pipefd[1]);

        VIR_FORCE_CLOSE(pipefd[1]);
        _exit(1);

    case -1:
        return -1;

    default:
        {
            int got = 0;
            int ret = -1;
            int want = buflen-1;

            VIR_FORCE_CLOSE(pipefd[1]);

            while (want) {
                if ((ret = read(pipefd[0], (*buf)+got, want)) <= 0)
                    break;
                got += ret;
                want -= ret;
            }
            VIR_FORCE_CLOSE(pipefd[0]);

            if (!ret)
                (*buf)[got] = '\0';

            waitpid(pid, NULL, 0);

            return ret;
        }
    }
}
#else /* !WIN32 */
int virtTestCaptureProgramOutput(const char *const argv[] ATTRIBUTE_UNUSED,
                                 char **buf ATTRIBUTE_UNUSED,
                                 int buflen ATTRIBUTE_UNUSED) {
    return -1;
}
#endif /* !WIN32 */


/**
 * @param stream: output stream write to differences to
 * @param expect: expected output text
 * @param actual: actual output text
 *
 * Display expected and actual output text, trimmed to
 * first and last characters at which differences occur
 */
int virtTestDifference(FILE *stream,
                       const char *expect,
                       const char *actual)
{
    const char *expectStart = expect;
    const char *expectEnd = expect + (strlen(expect)-1);
    const char *actualStart = actual;
    const char *actualEnd = actual + (strlen(actual)-1);

    if (!virTestGetDebug())
        return 0;

    if (virTestGetDebug() < 2) {
        /* Skip to first character where they differ */
        while (*expectStart && *actualStart &&
               *actualStart == *expectStart) {
            actualStart++;
            expectStart++;
        }

        /* Work backwards to last character where they differ */
        while (actualEnd > actualStart &&
               expectEnd > expectStart &&
               *actualEnd == *expectEnd) {
            actualEnd--;
            expectEnd--;
        }
    }

    /* Show the trimmed differences */
    fprintf(stream, "\nExpect [");
    if ((expectEnd - expectStart + 1) &&
        fwrite(expectStart, (expectEnd-expectStart+1), 1, stream) != 1)
        return -1;
    fprintf(stream, "]\n");
    fprintf(stream, "Actual [");
    if ((actualEnd - actualStart + 1) &&
        fwrite(actualStart, (actualEnd-actualStart+1), 1, stream) != 1)
        return -1;
    fprintf(stream, "]\n");

    /* Pad to line up with test name ... in virTestRun */
    fprintf(stream, "                                                                      ... ");

    return 0;
}

#if TEST_OOM
static void
virtTestErrorFuncQuiet(void *data ATTRIBUTE_UNUSED,
                       virErrorPtr err ATTRIBUTE_UNUSED)
{ }
#endif

struct virtTestLogData {
    virBuffer buf;
};

static struct virtTestLogData testLog = { VIR_BUFFER_INITIALIZER };

static int
virtTestLogOutput(const char *category ATTRIBUTE_UNUSED,
                  int priority ATTRIBUTE_UNUSED,
                  const char *funcname ATTRIBUTE_UNUSED,
                  long long lineno ATTRIBUTE_UNUSED,
                  const char *str, int len, void *data)
{
    struct virtTestLogData *log = data;
    virBufferAdd(&log->buf, str, len);
    return len;
}

static void
virtTestLogClose(void *data)
{
    struct virtTestLogData *log = data;

    virBufferFreeAndReset(&log->buf);
}

/* Return a malloc'd string (possibly with strlen of 0) of all data
 * logged since the last call to this function, or NULL on failure.  */
char *
virtTestLogContentAndReset(void)
{
    char *ret;

    if (virBufferError(&testLog.buf))
        return NULL;
    ret = virBufferContentAndReset(&testLog.buf);
    return ret ? ret : strdup("");
}

#if TEST_OOM_TRACE
static void
virtTestErrorHook(int n, void *data ATTRIBUTE_UNUSED)
{
    void *trace[30];
    int ntrace = ARRAY_CARDINALITY(trace);
    int i;
    char **symbols = NULL;

    ntrace = backtrace(trace, ntrace);
    symbols = backtrace_symbols(trace, ntrace);
    if (symbols) {
        fprintf(stderr, "Failing allocation %d at:\n", n);
        for (i = 0 ; i < ntrace ; i++) {
            if (symbols[i])
                fprintf(stderr, "  TRACE:  %s\n", symbols[i]);
        }
        free(symbols);
    }
}
#endif

static unsigned int
virTestGetFlag(const char *name) {
    char *flagStr;
    unsigned int flag;

    if ((flagStr = getenv(name)) == NULL)
        return 0;

    if (virStrToLong_ui(flagStr, NULL, 10, &flag) < 0)
        return 0;

    return flag;
}

unsigned int
virTestGetDebug() {
    if (testDebug == -1)
        testDebug = virTestGetFlag("VIR_TEST_DEBUG");
    return testDebug;
}

unsigned int
virTestGetVerbose() {
    if (testVerbose == -1)
        testVerbose = virTestGetFlag("VIR_TEST_VERBOSE");
    return testVerbose || virTestGetDebug();
}

int virtTestMain(int argc,
                 char **argv,
                 int (*func)(int, char **))
{
    int ret;
#if TEST_OOM
    int approxAlloc = 0;
    int n;
    char *oomStr = NULL;
    int oomCount;
    int mp = 0;
    pid_t *workers;
    int worker = 0;
#endif

    fprintf(stderr, "TEST: %s\n", STRPREFIX(argv[0], "./") ? argv[0] + 2 : argv[0]);
    if (!virTestGetVerbose())
        fprintf(stderr, "      ");

    if (virThreadInitialize() < 0 ||
        virErrorInitialize() < 0 ||
        virRandomInitialize(time(NULL) ^ getpid()))
        return 1;

    virLogSetFromEnv();
    if (virLogDefineOutput(virtTestLogOutput, virtTestLogClose, &testLog,
                           0, 0, NULL, 0) < 0)
        return 1;

#if TEST_OOM
    if ((oomStr = getenv("VIR_TEST_OOM")) != NULL) {
        if (virStrToLong_i(oomStr, NULL, 10, &oomCount) < 0)
            oomCount = 0;

        if (oomCount < 0)
            oomCount = 0;
        if (oomCount)
            testOOM = 1;
    }

    if (getenv("VIR_TEST_MP") != NULL) {
        mp = sysconf(_SC_NPROCESSORS_ONLN);
        fprintf(stderr, "Using %d worker processes\n", mp);
        if (VIR_ALLOC_N(workers, mp) < 0) {
            ret = EXIT_FAILURE;
            goto cleanup;
        }
    }

    /* Run once to prime any static allocations & ensure it passes */
    ret = (func)(argc, argv);
    if (ret != EXIT_SUCCESS)
        goto cleanup;

# if TEST_OOM_TRACE
    if (virTestGetDebug())
        virAllocTestHook(virtTestErrorHook, NULL);
# endif

    if (testOOM) {
        /* Makes next test runs quiet... */
        testOOM++;
        virSetErrorFunc(NULL, virtTestErrorFuncQuiet);

        virAllocTestInit();

        /* Run again to count allocs, and ensure it passes :-) */
        ret = (func)(argc, argv);
        if (ret != EXIT_SUCCESS)
            goto cleanup;

        approxAlloc = virAllocTestCount();
        testCounter++;
        if (virTestGetDebug())
            fprintf(stderr, "%d) OOM...\n", testCounter);
        else
            fprintf(stderr, "%d) OOM of %d allocs ", testCounter, approxAlloc);

        if (mp) {
            int i;
            for (i = 0 ; i < mp ; i++) {
                workers[i] = fork();
                if (workers[i] == 0) {
                    worker = i + 1;
                    break;
                }
            }
        }

        /* Run once for each alloc, failing a different one
           and validating that the test case failed */
        for (n = 0; n < approxAlloc && (!mp || worker) ; n++) {
            if (mp &&
                (n % mp) != (worker - 1))
                continue;
            if (!virTestGetDebug()) {
                if (mp)
                    fprintf(stderr, "%d", worker);
                else
                    fprintf(stderr, ".");
                fflush(stderr);
            }
            virAllocTestOOM(n+1, oomCount);

            if (((func)(argc, argv)) != EXIT_FAILURE) {
                ret = EXIT_FAILURE;
                break;
            }
        }

        if (mp) {
            if (worker) {
                _exit(ret);
            } else {
                int i, status;
                for (i = 0 ; i < mp ; i++) {
                    waitpid(workers[i], &status, 0);
                    if (WEXITSTATUS(status) != EXIT_SUCCESS)
                        ret = EXIT_FAILURE;
                }
                VIR_FREE(workers);
            }
        }

        if (virTestGetDebug())
            fprintf(stderr, " ... OOM of %d allocs", approxAlloc);

        if (ret == EXIT_SUCCESS)
            fprintf(stderr, " OK\n");
        else
            fprintf(stderr, " FAILED\n");
    }
cleanup:
#else
    ret = (func)(argc, argv);
#endif

    virResetLastError();
    if (!virTestGetVerbose()) {
        int i;
        for (i = (testCounter % 40) ; i > 0 && i < 40 ; i++)
            fprintf(stderr, " ");
        fprintf(stderr, " %-3d %s\n", testCounter, ret == 0 ? "OK" : "FAIL");
    }
    return ret;
}


#ifdef HAVE_REGEX_H
int virtTestClearLineRegex(const char *pattern,
                           char *str)
{
    regex_t reg;
    char *lineStart = str;
    char *lineEnd = strchr(str, '\n');

    if (regcomp(&reg, pattern, REG_EXTENDED | REG_NOSUB) != 0)
        return -1;

    while (lineStart) {
        int ret;
        if (lineEnd)
            *lineEnd = '\0';


        ret = regexec(&reg, lineStart, 0, NULL, 0);
        //fprintf(stderr, "Match %d '%s' '%s'\n", ret, lineStart, pattern);
        if (ret == 0) {
            if (lineEnd) {
                memmove(lineStart, lineEnd + 1, strlen(lineEnd+1) + 1);
                /* Don't update lineStart - just iterate again on this
                   location */
                lineEnd = strchr(lineStart, '\n');
            } else {
                *lineStart = '\0';
                lineStart = NULL;
            }
        } else {
            if (lineEnd) {
                *lineEnd = '\n';
                lineStart = lineEnd + 1;
                lineEnd = strchr(lineStart, '\n');
            } else {
                lineStart = NULL;
            }
        }
    }

    regfree(&reg);

    return 0;
}
#else
int virtTestClearLineRegex(const char *pattern ATTRIBUTE_UNUSED,
                           char *str ATTRIBUTE_UNUSED)
{
    return 0;
}
#endif
