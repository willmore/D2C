/*
 * qemu_monitor_text.c: interaction with QEMU monitor console
 *
 * Copyright (C) 2006-2010 Red Hat, Inc.
 * Copyright (C) 2006 Daniel P. Berrange
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
 */

#include <config.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <poll.h>
#include <unistd.h>
#include <string.h>

#include "qemu_monitor_text.h"
#include "qemu_command.h"
#include "c-ctype.h"
#include "memory.h"
#include "logging.h"
#include "driver.h"
#include "datatypes.h"
#include "virterror_internal.h"
#include "buf.h"

#define VIR_FROM_THIS VIR_FROM_QEMU

#define QEMU_CMD_PROMPT "\n(qemu) "
#define QEMU_PASSWD_PROMPT "Password: "

#define DEBUG_IO 0

/* Return -1 for error, 0 for success */
typedef int qemuMonitorExtraPromptHandler(qemuMonitorPtr mon,
                                          const char *buf,
                                          const char *prompt,
                                          void *data);


/* When connecting to a monitor, QEMU will print a greeting like
 *
 * QEMU 0.11.0 monitor - type 'help' for more information
 *
 * Don't expect the version number bit to be stable :-)
 */
#define GREETING_PREFIX "QEMU "
#define GREETING_POSTFIX "type 'help' for more information\r\n(qemu) "
#define BASIC_PROMPT "(qemu) "
#define PASSWORD_PROMPT "Password:"
#define DISK_ENCRYPTION_PREFIX "("
#define DISK_ENCRYPTION_POSTFIX ") is encrypted."
#define LINE_ENDING "\r\n"

int qemuMonitorTextIOProcess(qemuMonitorPtr mon ATTRIBUTE_UNUSED,
                             const char *data,
                             size_t len ATTRIBUTE_UNUSED,
                             qemuMonitorMessagePtr msg)
{
    int used = 0;

    /* Check for & discard greeting */
    if (STRPREFIX(data, GREETING_PREFIX)) {
        const char *offset = strstr(data, GREETING_POSTFIX);

        /* We see the greeting prefix, but not postfix, so pretend we've
           not consumed anything. We'll restart when more data arrives. */
        if (!offset) {
#if DEBUG_IO
            VIR_DEBUG0("Partial greeting seen, getting out & waiting for more");
#endif
            return 0;
        }

        used = offset - data + strlen(GREETING_POSTFIX);

#if DEBUG_IO
        VIR_DEBUG0("Discarded monitor greeting");
#endif
    }

    /* Don't print raw data in debug because its full of control chars */
    /*VIR_DEBUG("Process data %d byts of data [%s]", len - used, data + used);*/
#if DEBUG_IO
    VIR_DEBUG("Process data %d byts of data", (int)(len - used));
#endif

    /* Look for a non-zero reply followed by prompt */
    if (msg && !msg->finished) {
        char *start = NULL;
        char *end = NULL;
        char *skip;

        /* If we're here, we've already sent the command. We now
         * strip the trailing '\r' because it makes the matching
         * code that follows a little easier ie we can just strstr()
         * for the original command
         */
        if (msg->txLength > 0) {
            char *tmp;
            if ((tmp = strchr(msg->txBuffer, '\r'))) {
                *tmp = '\0';
            }
        }

        /* QEMU echos the command back to us, full of control
         * character junk that we don't want. We have to skip
         * over this junk by looking for the first complete
         * repetition of our command. Then we can look for
         * the prompt that is supposed to follow
         *
         * NB, we can't optimize by immediately looking for
         * LINE_ENDING, because QEMU 0.10 has bad problems
         * when initially connecting where it might write a
         * prompt in the wrong place. So we must not look
         * for LINE_ENDING, or BASIC_PROMPT until we've
         * seen our original command echod.
         */
        skip = strstr(data + used, msg->txBuffer);

        /* After the junk we should have a line ending... */
        if (skip) {
            start = strstr(skip + strlen(msg->txBuffer), LINE_ENDING);
        }

        /* ... then our command reply data, following by a (qemu) prompt */
        if (start) {
            char *passwd;
            start += strlen(LINE_ENDING);

            /* We might get a prompt for a password before the (qemu) prompt */
            passwd = strstr(start, PASSWORD_PROMPT);
            if (passwd) {
#if DEBUG_IO
                VIR_DEBUG("Seen a passwowrd prompt [%s]", data + used);
#endif
                if (msg->passwordHandler) {
                    int i;
                    /* Try and handle the prompt */
                    if (msg->passwordHandler(mon, msg,
                                             start,
                                             passwd - start + strlen(PASSWORD_PROMPT),
                                             msg->passwordOpaque) < 0)
                        return -1;

                    /* Blank out the password prompt so we don't re-trigger
                     * if we have to go back to sleep for more I/O */
                    for (i = 0 ; i < strlen(PASSWORD_PROMPT) ; i++)
                        start[i] = ' ';

                    /* Handled, so skip forward over password prompt */
                    start = passwd;
                } else {
                    errno = EACCES;
                    return -1;
                }
            }

            end = strstr(start, BASIC_PROMPT);
        }

        if (start && end) {
            int want = end - start;
            /* Annoyingly some commands may not have any reply data
             * at all upon success, but since we've detected the
             * BASIC_PROMPT we can reasonably reliably cope */
            if (want) {
                if (VIR_REALLOC_N(msg->rxBuffer,
                                  msg->rxLength + want + 1) < 0)
                    return -1;
                memcpy(msg->rxBuffer + msg->rxLength, start, want);
                msg->rxLength += want;
                msg->rxBuffer[msg->rxLength] = '\0';
#if DEBUG_IO
                VIR_DEBUG("Finished %d byte reply [%s]", want, msg->rxBuffer);
            } else {
                VIR_DEBUG0("Finished 0 byte reply");
#endif
            }
            msg->finished = 1;
            used += end - (data + used);
            used += strlen(BASIC_PROMPT);
        }
    }

#if DEBUG_IO
    VIR_DEBUG("Total used %d", used);
#endif
    return used;
}

static int
qemuMonitorCommandWithHandler(qemuMonitorPtr mon,
                              const char *cmd,
                              qemuMonitorPasswordHandler passwordHandler,
                              void *passwordOpaque,
                              int scm_fd,
                              char **reply) {
    int ret;
    qemuMonitorMessage msg;

    *reply = NULL;

    memset(&msg, 0, sizeof msg);

    if (virAsprintf(&msg.txBuffer, "%s\r", cmd) < 0) {
        virReportOOMError();
        return -1;
    }
    msg.txLength = strlen(msg.txBuffer);
    msg.txFD = scm_fd;
    msg.passwordHandler = passwordHandler;
    msg.passwordOpaque = passwordOpaque;

    VIR_DEBUG("Send command '%s' for write with FD %d", cmd, scm_fd);

    ret = qemuMonitorSend(mon, &msg);

    VIR_DEBUG("Receive command reply ret=%d errno=%d %d bytes '%s'",
              ret, msg.lastErrno, msg.rxLength, msg.rxBuffer);

    /* Just in case buffer had some passwords in */
    memset(msg.txBuffer, 0, msg.txLength);
    VIR_FREE(msg.txBuffer);

    /* To make life safer for callers, already ensure there's at least an empty string */
    if (msg.rxBuffer) {
        *reply = msg.rxBuffer;
    } else {
        *reply = strdup("");
        if (!*reply) {
            virReportOOMError();
            return -1;
        }
    }

    if (ret < 0) {
        virReportSystemError(msg.lastErrno,
                             _("cannot send monitor command '%s'"), cmd);
        VIR_FREE(*reply);
    }

    return ret;
}

static int
qemuMonitorCommandWithFd(qemuMonitorPtr mon,
                          const char *cmd,
                          int scm_fd,
                          char **reply) {
    return qemuMonitorCommandWithHandler(mon, cmd, NULL, NULL, scm_fd, reply);
}

static int
qemuMonitorCommand(qemuMonitorPtr mon,
                    const char *cmd,
                    char **reply) {
    return qemuMonitorCommandWithFd(mon, cmd, -1, reply);
}


static int
qemuMonitorSendDiskPassphrase(qemuMonitorPtr mon,
                              qemuMonitorMessagePtr msg,
                              const char *data,
                              size_t len ATTRIBUTE_UNUSED,
                              void *opaque)
{
    virConnectPtr conn = opaque;
    char *path;
    char *passphrase = NULL;
    size_t passphrase_len = 0;
    int res;
    const char *pathStart;
    const char *pathEnd;

    /*
     * For disk passwords:
     *
     *    ide0-hd0 (/path/to/volume) is encrypted.
     *    Password:
     *
     */
    pathStart = strstr(data, DISK_ENCRYPTION_PREFIX);
    pathEnd = strstr(data, DISK_ENCRYPTION_POSTFIX);
    if (!pathStart || !pathEnd || pathStart >= pathEnd) {
        errno = -EINVAL;
        return -1;
    }

    /* Extra the path */
    pathStart += strlen(DISK_ENCRYPTION_PREFIX);
    path = strndup(pathStart, pathEnd - pathStart);
    if (!path) {
        errno = ENOMEM;
        return -1;
    }

    /* Fetch the disk password if possible */
    res = qemuMonitorGetDiskSecret(mon,
                                   conn,
                                   path,
                                   &passphrase,
                                   &passphrase_len);
    VIR_FREE(path);
    if (res < 0)
        return -1;

    /* Enlarge transmit buffer to allow for the extra data
     * to be sent back */
    if (VIR_REALLOC_N(msg->txBuffer,
                      msg->txLength + passphrase_len + 1 + 1) < 0) {
        memset(passphrase, 0, passphrase_len);
        VIR_FREE(passphrase);
        errno = ENOMEM;
        return -1;
    }

    /* Queue the password for sending */
    memcpy(msg->txBuffer + msg->txLength,
           passphrase, passphrase_len);
    msg->txLength += passphrase_len;
    msg->txBuffer[msg->txLength] = '\r';
    msg->txLength++;
    msg->txBuffer[msg->txLength] = '\0';

    memset(passphrase, 0, passphrase_len);
    VIR_FREE(passphrase);

    return 0;
}

int
qemuMonitorTextStartCPUs(qemuMonitorPtr mon,
                         virConnectPtr conn) {
    char *reply;

    if (qemuMonitorCommandWithHandler(mon, "cont",
                                      qemuMonitorSendDiskPassphrase,
                                      conn,
                                      -1, &reply) < 0)
        return -1;

    VIR_FREE(reply);
    return 0;
}


int
qemuMonitorTextStopCPUs(qemuMonitorPtr mon) {
    char *info;

    if (qemuMonitorCommand(mon, "stop", &info) < 0) {
        qemuReportError(VIR_ERR_OPERATION_FAILED,
                        "%s", _("cannot stop CPU execution"));
        return -1;
    }
    VIR_FREE(info);
    return 0;
}


int qemuMonitorTextSystemPowerdown(qemuMonitorPtr mon) {
    char *info;

    if (qemuMonitorCommand(mon, "system_powerdown", &info) < 0) {
        qemuReportError(VIR_ERR_OPERATION_FAILED,
                        "%s", _("system shutdown operation failed"));
        return -1;
    }
    VIR_FREE(info);
    return 0;
}


int qemuMonitorTextGetCPUInfo(qemuMonitorPtr mon,
                              int **pids)
{
    char *qemucpus = NULL;
    char *line;
    int lastVcpu = -1;
    pid_t *cpupids = NULL;
    size_t ncpupids = 0;

    if (qemuMonitorCommand(mon, "info cpus", &qemucpus) < 0) {
        qemuReportError(VIR_ERR_INTERNAL_ERROR,
                        "%s", _("cannot run monitor command to fetch CPU thread info"));
        return -1;
    }

    /*
     * This is the gross format we're about to parse :-{
     *
     * (qemu) info cpus
     * * CPU #0: pc=0x00000000000f0c4a thread_id=30019
     *   CPU #1: pc=0x00000000fffffff0 thread_id=30020
     *   CPU #2: pc=0x00000000fffffff0 thread_id=30021
     *
     */
    line = qemucpus;
    do {
        char *offset = strchr(line, '#');
        char *end = NULL;
        int vcpu = 0, tid = 0;

        /* See if we're all done */
        if (offset == NULL)
            break;

        /* Extract VCPU number */
        if (virStrToLong_i(offset + 1, &end, 10, &vcpu) < 0)
            goto error;

        if (end == NULL || *end != ':')
            goto error;

        /* Extract host Thread ID */
        if ((offset = strstr(line, "thread_id=")) == NULL)
            goto error;

        if (virStrToLong_i(offset + strlen("thread_id="), &end, 10, &tid) < 0)
            goto error;
        if (end == NULL || !c_isspace(*end))
            goto error;

        if (vcpu != (lastVcpu + 1))
            goto error;

        if (VIR_REALLOC_N(cpupids, ncpupids+1) < 0)
            goto error;

        DEBUG("vcpu=%d pid=%d", vcpu, tid);
        cpupids[ncpupids++] = tid;
        lastVcpu = vcpu;

        /* Skip to next data line */
        line = strchr(offset, '\r');
        if (line == NULL)
            line = strchr(offset, '\n');
    } while (line != NULL);

    /* Validate we got data for all VCPUs we expected */
    VIR_FREE(qemucpus);
    *pids = cpupids;
    return ncpupids;

error:
    VIR_FREE(qemucpus);
    VIR_FREE(cpupids);

    /* Returning 0 to indicate non-fatal failure, since
     * older QEMU does not have VCPU<->PID mapping and
     * we don't want to fail on that
     */
    return 0;
}

static int parseMemoryStat(char **text, unsigned int tag,
                           const char *search, virDomainMemoryStatPtr stat)
{
    char *dummy;
    unsigned long long value;

    if (STRPREFIX (*text, search)) {
        *text += strlen(search);
        if (virStrToLong_ull (*text, &dummy, 10, &value)) {
            DEBUG ("error reading %s: %s", search, *text);
            return 0;
        }

        /* Convert bytes to kilobytes for libvirt */
        switch (tag) {
            case VIR_DOMAIN_MEMORY_STAT_SWAP_IN:
            case VIR_DOMAIN_MEMORY_STAT_SWAP_OUT:
            case VIR_DOMAIN_MEMORY_STAT_UNUSED:
            case VIR_DOMAIN_MEMORY_STAT_AVAILABLE:
                value = value >> 10;
        }
        stat->tag = tag;
        stat->val = value;
        return 1;
    }
    return 0;
}

/* The reply from the 'info balloon' command may contain additional memory
 * statistics in the form: '[,<tag>=<val>]*'
 */
static int qemuMonitorParseExtraBalloonInfo(char *text,
                                            virDomainMemoryStatPtr stats,
                                            unsigned int nr_stats)
{
    char *p = text;
    unsigned int nr_stats_found = 0;

    while (*p && nr_stats_found < nr_stats) {
        if (parseMemoryStat(&p, VIR_DOMAIN_MEMORY_STAT_SWAP_IN,
                            ",mem_swapped_in=", &stats[nr_stats_found]) ||
            parseMemoryStat(&p, VIR_DOMAIN_MEMORY_STAT_SWAP_OUT,
                            ",mem_swapped_out=", &stats[nr_stats_found]) ||
            parseMemoryStat(&p, VIR_DOMAIN_MEMORY_STAT_MAJOR_FAULT,
                            ",major_page_faults=", &stats[nr_stats_found]) ||
            parseMemoryStat(&p, VIR_DOMAIN_MEMORY_STAT_MINOR_FAULT,
                            ",minor_page_faults=", &stats[nr_stats_found]) ||
            parseMemoryStat(&p, VIR_DOMAIN_MEMORY_STAT_UNUSED,
                            ",free_mem=", &stats[nr_stats_found]) ||
            parseMemoryStat(&p, VIR_DOMAIN_MEMORY_STAT_AVAILABLE,
                            ",total_mem=", &stats[nr_stats_found]))
            nr_stats_found++;

        /* Skip to the next label.  When *p is ',' the last match attempt
         * failed so try to match the next ','.
         */
        if (*p == ',')
            p++;
        p = strchr (p, ',');
        if (!p) break;
    }
    return nr_stats_found;
}


/* The reply from QEMU contains 'ballon: actual=421' where value is in MB */
#define BALLOON_PREFIX "balloon: actual="

/*
 * Returns: 0 if balloon not supported, +1 if balloon query worked
 * or -1 on failure
 */
int qemuMonitorTextGetBalloonInfo(qemuMonitorPtr mon,
                                  unsigned long *currmem)
{
    char *reply = NULL;
    int ret = -1;
    char *offset;

    if (qemuMonitorCommand(mon, "info balloon", &reply) < 0) {
        qemuReportError(VIR_ERR_OPERATION_FAILED,
                        "%s", _("could not query memory balloon allocation"));
        return -1;
    }

    if ((offset = strstr(reply, BALLOON_PREFIX)) != NULL) {
        unsigned int memMB;
        char *end;
        offset += strlen(BALLOON_PREFIX);
        if (virStrToLong_ui(offset, &end, 10, &memMB) < 0) {
            qemuReportError(VIR_ERR_OPERATION_FAILED,
                            _("could not parse memory balloon allocation from '%s'"), reply);
            goto cleanup;
        }
        *currmem = memMB * 1024;
        ret = 1;
    } else {
        /* We don't raise an error here, since its to be expected that
         * many QEMU's don't support ballooning
         */
        ret = 0;
    }

cleanup:
    VIR_FREE(reply);
    return ret;
}

int qemuMonitorTextGetMemoryStats(qemuMonitorPtr mon,
                                  virDomainMemoryStatPtr stats,
                                  unsigned int nr_stats)
{
    char *reply = NULL;
    int ret = 0;
    char *offset;

    if (qemuMonitorCommand(mon, "info balloon", &reply) < 0) {
        qemuReportError(VIR_ERR_OPERATION_FAILED,
                        "%s", _("could not query memory balloon statistics"));
        return -1;
    }

    if ((offset = strstr(reply, BALLOON_PREFIX)) != NULL) {
        offset += strlen(BALLOON_PREFIX);
        if ((offset = strchr(offset, ',')) != NULL) {
            ret = qemuMonitorParseExtraBalloonInfo(offset, stats, nr_stats);
        }
    }

    VIR_FREE(reply);
    return ret;
}


int qemuMonitorTextGetBlockStatsInfo(qemuMonitorPtr mon,
                                     const char *devname,
                                     long long *rd_req,
                                     long long *rd_bytes,
                                     long long *wr_req,
                                     long long *wr_bytes,
                                     long long *errs)
{
    char *info = NULL;
    int ret = -1;
    char *dummy;
    const char *p, *eol;
    int devnamelen = strlen(devname);

    if (qemuMonitorCommand (mon, "info blockstats", &info) < 0) {
        qemuReportError(VIR_ERR_OPERATION_FAILED,
                        "%s", _("'info blockstats' command failed"));
        goto cleanup;
    }

    /* If the command isn't supported then qemu prints the supported
     * info commands, so the output starts "info ".  Since this is
     * unlikely to be the name of a block device, we can use this
     * to detect if qemu supports the command.
     */
    if (strstr(info, "\ninfo ")) {
        qemuReportError(VIR_ERR_NO_SUPPORT,
                        "%s",
                        _("'info blockstats' not supported by this qemu"));
        goto cleanup;
    }

    *rd_req = -1;
    *rd_bytes = -1;
    *wr_req = -1;
    *wr_bytes = -1;
    *errs = -1;

    /* The output format for both qemu & KVM is:
     *   blockdevice: rd_bytes=% wr_bytes=% rd_operations=% wr_operations=%
     *   (repeated for each block device)
     * where '%' is a 64 bit number.
     */
    p = info;

    while (*p) {
        /* New QEMU has separate names for host & guest side of the disk
         * and libvirt gives the host side a 'drive-' prefix. The passed
         * in devname is the guest side though
         */
        if (STRPREFIX(p, QEMU_DRIVE_HOST_PREFIX))
            p += strlen(QEMU_DRIVE_HOST_PREFIX);

        if (STREQLEN (p, devname, devnamelen)
            && p[devnamelen] == ':' && p[devnamelen+1] == ' ') {

            eol = strchr (p, '\n');
            if (!eol)
                eol = p + strlen (p);

            p += devnamelen+2;         /* Skip to first label. */

            while (*p) {
                if (STRPREFIX (p, "rd_bytes=")) {
                    p += 9;
                    if (virStrToLong_ll (p, &dummy, 10, rd_bytes) == -1)
                        DEBUG ("error reading rd_bytes: %s", p);
                } else if (STRPREFIX (p, "wr_bytes=")) {
                    p += 9;
                    if (virStrToLong_ll (p, &dummy, 10, wr_bytes) == -1)
                        DEBUG ("error reading wr_bytes: %s", p);
                } else if (STRPREFIX (p, "rd_operations=")) {
                    p += 14;
                    if (virStrToLong_ll (p, &dummy, 10, rd_req) == -1)
                        DEBUG ("error reading rd_req: %s", p);
                } else if (STRPREFIX (p, "wr_operations=")) {
                    p += 14;
                    if (virStrToLong_ll (p, &dummy, 10, wr_req) == -1)
                        DEBUG ("error reading wr_req: %s", p);
                } else
                    DEBUG ("unknown block stat near %s", p);

                /* Skip to next label. */
                p = strchr (p, ' ');
                if (!p || p >= eol) break;
                p++;
            }
            ret = 0;
            goto cleanup;
        }

        /* Skip to next line. */
        p = strchr (p, '\n');
        if (!p) break;
        p++;
    }

    /* If we reach here then the device was not found. */
    qemuReportError (VIR_ERR_INVALID_ARG,
                     _("no stats found for device %s"), devname);

 cleanup:
    VIR_FREE(info);
    return ret;
}


int qemuMonitorTextGetBlockExtent(qemuMonitorPtr mon ATTRIBUTE_UNUSED,
                                  const char *devname ATTRIBUTE_UNUSED,
                                  unsigned long long *extent ATTRIBUTE_UNUSED)
{
    qemuReportError(VIR_ERR_INTERNAL_ERROR, "%s",
                    _("unable to query block extent with this QEMU"));
    return -1;
}


static int
qemuMonitorSendVNCPassphrase(qemuMonitorPtr mon ATTRIBUTE_UNUSED,
                             qemuMonitorMessagePtr msg,
                             const char *data ATTRIBUTE_UNUSED,
                             size_t len ATTRIBUTE_UNUSED,
                             void *opaque)
{
    char *passphrase = opaque;
    size_t passphrase_len = strlen(passphrase);

    /* Enlarge transmit buffer to allow for the extra data
     * to be sent back */
    if (VIR_REALLOC_N(msg->txBuffer,
                      msg->txLength + passphrase_len + 1 + 1) < 0) {
        errno = ENOMEM;
        return -1;
    }

    /* Queue the password for sending */
    memcpy(msg->txBuffer + msg->txLength,
           passphrase, passphrase_len);
    msg->txLength += passphrase_len;
    msg->txBuffer[msg->txLength] = '\r';
    msg->txLength++;
    msg->txBuffer[msg->txLength] = '\0';

    return 0;
}

int qemuMonitorTextSetVNCPassword(qemuMonitorPtr mon,
                                  const char *password)
{
    char *info = NULL;

    if (qemuMonitorCommandWithHandler(mon, "change vnc password",
                                      qemuMonitorSendVNCPassphrase,
                                      (char *)password,
                                      -1, &info) < 0) {
        qemuReportError(VIR_ERR_INTERNAL_ERROR,
                        "%s", _("setting VNC password failed"));
        return -1;
    }
    VIR_FREE(info);
    return 0;
}

/* Returns -1 on error, -2 if not supported */
int qemuMonitorTextSetPassword(qemuMonitorPtr mon,
                               const char *protocol,
                               const char *password,
                               const char *action_if_connected)
{
    char *cmd = NULL;
    char *reply = NULL;
    int ret = -1;

    if (virAsprintf(&cmd, "set_password %s \"%s\" %s",
                    protocol, password, action_if_connected) < 0) {
        virReportOOMError();
        goto cleanup;
    }

    if (qemuMonitorCommand(mon, cmd, &reply) < 0) {
        qemuReportError(VIR_ERR_OPERATION_FAILED,
                        "%s", _("setting password failed"));
        goto cleanup;
    }

    if (strstr(reply, "unknown command:")) {
        ret = -2;
        goto cleanup;
    }

    ret = 0;

cleanup:
    VIR_FREE(reply);
    VIR_FREE(cmd);
    return ret;
}

/* Returns -1 on error, -2 if not supported */
int qemuMonitorTextExpirePassword(qemuMonitorPtr mon,
                                  const char *protocol,
                                  const char *expire_time)
{
    char *cmd = NULL;
    char *reply = NULL;
    int ret = -1;

    if (virAsprintf(&cmd, "expire_password %s %s",
                    protocol, expire_time) < 0) {
        virReportOOMError();
        goto cleanup;
    }

    if (qemuMonitorCommand(mon, cmd, &reply) < 0) {
        qemuReportError(VIR_ERR_OPERATION_FAILED,
                        "%s", _("expiring password failed"));
        goto cleanup;
    }

    if (strstr(reply, "unknown command:")) {
        ret = -2;
        goto cleanup;
    }

    ret = 0;

cleanup:
    VIR_FREE(reply);
    VIR_FREE(cmd);
    return ret;
}

/*
 * Returns: 0 if balloon not supported, +1 if balloon adjust worked
 * or -1 on failure
 */
int qemuMonitorTextSetBalloon(qemuMonitorPtr mon,
                              unsigned long newmem)
{
    char *cmd;
    char *reply = NULL;
    int ret = -1;

    /*
     * 'newmem' is in KB, QEMU monitor works in MB, and we all wish
     * we just worked in bytes with unsigned long long everywhere.
     */
    if (virAsprintf(&cmd, "balloon %lu", VIR_DIV_UP(newmem, 1024)) < 0) {
        virReportOOMError();
        return -1;
    }

    if (qemuMonitorCommand(mon, cmd, &reply) < 0) {
        qemuReportError(VIR_ERR_OPERATION_FAILED,
                        "%s", _("could not balloon memory allocation"));
        VIR_FREE(cmd);
        return -1;
    }
    VIR_FREE(cmd);

    /* If the command failed qemu prints: 'unknown command'
     * No message is printed on success it seems */
    if (strstr(reply, "unknown command:")) {
        /* Don't set error - it is expected memory balloon fails on many qemu */
        ret = 0;
    } else {
        ret = 1;
    }

    VIR_FREE(reply);
    return ret;
}


/*
 * Returns: 0 if CPU hotplug not supported, +1 if CPU hotplug worked
 * or -1 on failure
 */
int qemuMonitorTextSetCPU(qemuMonitorPtr mon, int cpu, int online)
{
    char *cmd;
    char *reply = NULL;
    int ret = -1;

    if (virAsprintf(&cmd, "cpu_set %d %s", cpu, online ? "online" : "offline") < 0) {
        virReportOOMError();
        return -1;
    }

    if (qemuMonitorCommand(mon, cmd, &reply) < 0) {
        qemuReportError(VIR_ERR_OPERATION_FAILED,
                        "%s", _("could not change CPU online status"));
        VIR_FREE(cmd);
        return -1;
    }
    VIR_FREE(cmd);

    /* If the command failed qemu prints: 'unknown command'
     * No message is printed on success it seems */
    if (strstr(reply, "unknown command:")) {
        /* Don't set error - it is expected CPU onlining fails on many qemu - caller will handle */
        ret = 0;
    } else {
        ret = 1;
    }

    VIR_FREE(reply);
    return ret;
}


int qemuMonitorTextEjectMedia(qemuMonitorPtr mon,
                              const char *devname,
                              bool force)
{
    char *cmd = NULL;
    char *reply = NULL;
    int ret = -1;

    if (virAsprintf(&cmd, "eject %s%s", force ? "-f " : "", devname) < 0) {
        virReportOOMError();
        goto cleanup;
    }

    if (qemuMonitorCommand(mon, cmd, &reply) < 0) {
        qemuReportError(VIR_ERR_OPERATION_FAILED,
                        _("could not eject media on %s"), devname);
        goto cleanup;
    }

    /* If the command failed qemu prints:
     * device not found, device is locked ...
     * No message is printed on success it seems */
    if (strstr(reply, "device ")) {
        qemuReportError(VIR_ERR_OPERATION_FAILED,
                        _("could not eject media on %s: %s"), devname, reply);
        goto cleanup;
    }

    ret = 0;

cleanup:
    VIR_FREE(reply);
    VIR_FREE(cmd);
    return ret;
}


int qemuMonitorTextChangeMedia(qemuMonitorPtr mon,
                               const char *devname,
                               const char *newmedia,
                               const char *format ATTRIBUTE_UNUSED)
{
    char *cmd = NULL;
    char *reply = NULL;
    char *safepath = NULL;
    int ret = -1;

    if (!(safepath = qemuMonitorEscapeArg(newmedia))) {
        virReportOOMError();
        goto cleanup;
    }

    if (virAsprintf(&cmd, "change %s \"%s\"", devname, safepath) < 0) {
        virReportOOMError();
        goto cleanup;
    }

    if (qemuMonitorCommand(mon, cmd, &reply) < 0) {
        qemuReportError(VIR_ERR_OPERATION_FAILED,
                        _("could not change media on %s"), devname);
        goto cleanup;
    }

    /* If the command failed qemu prints:
     * device not found, device is locked ...
     * No message is printed on success it seems */
    if (strstr(reply, "device ")) {
        qemuReportError(VIR_ERR_OPERATION_FAILED,
                        _("could not change media on %s: %s"), devname, reply);
        goto cleanup;
    }

    /* Could not open message indicates bad filename */
    if (strstr(reply, "Could not open ")) {
        qemuReportError(VIR_ERR_OPERATION_FAILED,
                        _("could not change media on %s: %s"), devname, reply);
        goto cleanup;
    }

    ret = 0;

cleanup:
    VIR_FREE(reply);
    VIR_FREE(cmd);
    VIR_FREE(safepath);
    return ret;
}

static int qemuMonitorTextSaveMemory(qemuMonitorPtr mon,
                                     const char *cmdtype,
                                     unsigned long long offset,
                                     size_t length,
                                     const char *path)
{
    char *cmd = NULL;
    char *reply = NULL;
    char *safepath = NULL;
    int ret = -1;

    if (!(safepath = qemuMonitorEscapeArg(path))) {
        virReportOOMError();
        goto cleanup;
    }

    if (virAsprintf(&cmd, "%s %llu %zi \"%s\"", cmdtype, offset, length, safepath) < 0) {
        virReportOOMError();
        goto cleanup;
    }

    if (qemuMonitorCommand(mon, cmd, &reply) < 0) {
        qemuReportError(VIR_ERR_OPERATION_FAILED,
                        _("could not save memory region to '%s'"), path);
        goto cleanup;
    }

    /* XXX what is printed on failure ? */

    ret = 0;

cleanup:
    VIR_FREE(cmd);
    VIR_FREE(reply);
    VIR_FREE(safepath);
    return ret;
}


int qemuMonitorTextSaveVirtualMemory(qemuMonitorPtr mon,
                                     unsigned long long offset,
                                     size_t length,
                                     const char *path)
{
    return qemuMonitorTextSaveMemory(mon, "memsave", offset, length, path);
}

int qemuMonitorTextSavePhysicalMemory(qemuMonitorPtr mon,
                                      unsigned long long offset,
                                      size_t length,
                                      const char *path)
{
    return qemuMonitorTextSaveMemory(mon, "pmemsave", offset, length, path);
}


int qemuMonitorTextSetMigrationSpeed(qemuMonitorPtr mon,
                                     unsigned long bandwidth)
{
    char *cmd = NULL;
    char *info = NULL;
    int ret = -1;

    if (virAsprintf(&cmd, "migrate_set_speed %lum", bandwidth) < 0) {
        virReportOOMError();
        goto cleanup;
    }

    if (qemuMonitorCommand(mon, cmd, &info) < 0) {
        qemuReportError(VIR_ERR_OPERATION_FAILED,
                        "%s", _("could not restrict migration speed"));
        goto cleanup;
    }

    ret = 0;

cleanup:
    VIR_FREE(info);
    VIR_FREE(cmd);
    return ret;
}


int qemuMonitorTextSetMigrationDowntime(qemuMonitorPtr mon,
                                        unsigned long long downtime)
{
    char *cmd = NULL;
    char *info = NULL;
    int ret = -1;

    if (virAsprintf(&cmd, "migrate_set_downtime %llums", downtime) < 0) {
        virReportOOMError();
        goto cleanup;
    }

    if (qemuMonitorCommand(mon, cmd, &info) < 0) {
        qemuReportError(VIR_ERR_OPERATION_FAILED,
                        "%s", _("could not set maximum migration downtime"));
        goto cleanup;
    }

    ret = 0;

cleanup:
    VIR_FREE(info);
    VIR_FREE(cmd);
    return ret;
}


#define MIGRATION_PREFIX "Migration status: "
#define MIGRATION_TRANSFER_PREFIX "transferred ram: "
#define MIGRATION_REMAINING_PREFIX "remaining ram: "
#define MIGRATION_TOTAL_PREFIX "total ram: "

int qemuMonitorTextGetMigrationStatus(qemuMonitorPtr mon,
                                      int *status,
                                      unsigned long long *transferred,
                                      unsigned long long *remaining,
                                      unsigned long long *total) {
    char *reply;
    char *tmp;
    char *end;
    int ret = -1;

    *status = QEMU_MONITOR_MIGRATION_STATUS_INACTIVE;
    *transferred = 0;
    *remaining = 0;
    *total = 0;

    if (qemuMonitorCommand(mon, "info migrate", &reply) < 0) {
        qemuReportError(VIR_ERR_OPERATION_FAILED,
                        "%s", _("cannot query migration status"));
        return -1;
    }

    if ((tmp = strstr(reply, MIGRATION_PREFIX)) != NULL) {
        tmp += strlen(MIGRATION_PREFIX);
        end = strchr(tmp, '\r');
        if (end == NULL) {
            qemuReportError(VIR_ERR_INTERNAL_ERROR,
                            _("unexpected migration status in %s"), reply);
            goto cleanup;
        }
        *end = '\0';

        if ((*status = qemuMonitorMigrationStatusTypeFromString(tmp)) < 0) {
            qemuReportError(VIR_ERR_INTERNAL_ERROR,
                            _("unexpected migration status in %s"), reply);
            goto cleanup;
        }

        if (*status == QEMU_MONITOR_MIGRATION_STATUS_ACTIVE) {
            tmp = end + 1;

            if (!(tmp = strstr(tmp, MIGRATION_TRANSFER_PREFIX)))
                goto done;
            tmp += strlen(MIGRATION_TRANSFER_PREFIX);

            if (virStrToLong_ull(tmp, &end, 10, transferred) < 0) {
                qemuReportError(VIR_ERR_INTERNAL_ERROR,
                                _("cannot parse migration data transferred statistic %s"), tmp);
                goto cleanup;
            }
            *transferred *= 1024;
            tmp = end;

            if (!(tmp = strstr(tmp, MIGRATION_REMAINING_PREFIX)))
                goto done;
            tmp += strlen(MIGRATION_REMAINING_PREFIX);

            if (virStrToLong_ull(tmp, &end, 10, remaining) < 0) {
                qemuReportError(VIR_ERR_INTERNAL_ERROR,
                                _("cannot parse migration data remaining statistic %s"), tmp);
                goto cleanup;
            }
            *remaining *= 1024;

            if (!(tmp = strstr(tmp, MIGRATION_TOTAL_PREFIX)))
                goto done;
            tmp += strlen(MIGRATION_TOTAL_PREFIX);

            if (virStrToLong_ull(tmp, &end, 10, total) < 0) {
                qemuReportError(VIR_ERR_INTERNAL_ERROR,
                                _("cannot parse migration data total statistic %s"), tmp);
                goto cleanup;
            }
            *total *= 1024;

        }
    } else if (strstr(reply, "info migration") != NULL) {
        /* 'info migrate' returned help for info commands and the help page
         * advertises 'info migration' command. Therefore we have an old
         * command implementation installed and this must be kvm 72 on debian */
        qemuReportError(VIR_ERR_INTERNAL_ERROR,
                        _("command 'info migrate' is not implemented in kvm,"
                          " please update to qemu-kvm"));
        goto cleanup;
   }

done:
    ret = 0;

cleanup:
    VIR_FREE(reply);
    return ret;
}


static int qemuMonitorTextMigrate(qemuMonitorPtr mon,
                                  unsigned int flags,
                                  const char *dest)
{
    char *cmd = NULL;
    char *info = NULL;
    int ret = -1;
    char *safedest = qemuMonitorEscapeArg(dest);
    virBuffer extra = VIR_BUFFER_INITIALIZER;
    char *extrastr = NULL;

    if (!safedest) {
        virReportOOMError();
        return -1;
    }

    if (flags & QEMU_MONITOR_MIGRATE_BACKGROUND)
        virBufferAddLit(&extra, " -d");
    if (flags & QEMU_MONITOR_MIGRATE_NON_SHARED_DISK)
        virBufferAddLit(&extra, " -b");
    if (flags & QEMU_MONITOR_MIGRATE_NON_SHARED_INC)
        virBufferAddLit(&extra, " -i");
    if (virBufferError(&extra)) {
        virBufferFreeAndReset(&extra);
        virReportOOMError();
        goto cleanup;
    }

    extrastr = virBufferContentAndReset(&extra);
    if (virAsprintf(&cmd, "migrate %s\"%s\"", extrastr ? extrastr : "",
                    safedest) < 0) {
        virReportOOMError();
        goto cleanup;
    }

    if (qemuMonitorCommand(mon, cmd, &info) < 0) {
        qemuReportError(VIR_ERR_INTERNAL_ERROR,
                        _("unable to start migration to %s"), dest);
        goto cleanup;
    }

    /* Now check for "fail" in the output string */
    if (strstr(info, "fail") != NULL) {
        qemuReportError(VIR_ERR_OPERATION_FAILED,
                        _("migration to '%s' failed: %s"), dest, info);
        goto cleanup;
    }
    /* If the command isn't supported then qemu prints:
     * unknown command: migrate" */
    if (strstr(info, "unknown command:")) {
        qemuReportError(VIR_ERR_NO_SUPPORT,
                        _("migration to '%s' not supported by this qemu: %s"), dest, info);
        goto cleanup;
    }


    ret = 0;

cleanup:
    VIR_FREE(extrastr);
    VIR_FREE(safedest);
    VIR_FREE(info);
    VIR_FREE(cmd);
    return ret;
}

int qemuMonitorTextMigrateToHost(qemuMonitorPtr mon,
                                 unsigned int flags,
                                 const char *hostname,
                                 int port)
{
    char *uri = NULL;
    int ret;

    if (virAsprintf(&uri, "tcp:%s:%d", hostname, port) < 0) {
        virReportOOMError();
        return -1;
    }

    ret = qemuMonitorTextMigrate(mon, flags, uri);

    VIR_FREE(uri);

    return ret;
}


int qemuMonitorTextMigrateToCommand(qemuMonitorPtr mon,
                                    unsigned int flags,
                                    const char * const *argv)
{
    char *argstr;
    char *dest = NULL;
    int ret = -1;

    argstr = virArgvToString(argv);
    if (!argstr) {
        virReportOOMError();
        goto cleanup;
    }

    if (virAsprintf(&dest, "exec:%s", argstr) < 0) {
        virReportOOMError();
        goto cleanup;
    }

    ret = qemuMonitorTextMigrate(mon, flags, dest);

cleanup:
    VIR_FREE(argstr);
    VIR_FREE(dest);
    return ret;
}

int qemuMonitorTextMigrateToFile(qemuMonitorPtr mon,
                                 unsigned int flags,
                                 const char * const *argv,
                                 const char *target,
                                 unsigned long long offset)
{
    char *argstr;
    char *dest = NULL;
    int ret = -1;
    char *safe_target = NULL;

    argstr = virArgvToString(argv);
    if (!argstr) {
        virReportOOMError();
        goto cleanup;
    }

    /* Migrate to file */
    safe_target = qemuMonitorEscapeShell(target);
    if (!safe_target) {
        virReportOOMError();
        goto cleanup;
    }

    /* Two dd processes, sharing the same stdout, are necessary to
     * allow starting at an alignment of 512, but without wasting
     * padding to get to the larger alignment useful for speed.  Use
     * <> redirection to avoid truncating a regular file.  */
    if (virAsprintf(&dest, "exec:" VIR_WRAPPER_SHELL_PREFIX "%s | "
                    "{ dd bs=%llu seek=%llu if=/dev/null && "
                    "dd bs=%llu; } 1<>%s" VIR_WRAPPER_SHELL_SUFFIX,
                    argstr, QEMU_MONITOR_MIGRATE_TO_FILE_BS,
                    offset / QEMU_MONITOR_MIGRATE_TO_FILE_BS,
                    QEMU_MONITOR_MIGRATE_TO_FILE_TRANSFER_SIZE,
                    safe_target) < 0) {
        virReportOOMError();
        goto cleanup;
    }

    ret = qemuMonitorTextMigrate(mon, flags, dest);

cleanup:
    VIR_FREE(safe_target);
    VIR_FREE(argstr);
    VIR_FREE(dest);
    return ret;
}

int qemuMonitorTextMigrateToUnix(qemuMonitorPtr mon,
                                 unsigned int flags,
                                 const char *unixfile)
{
    char *dest = NULL;
    int ret = -1;

    if (virAsprintf(&dest, "unix:%s", unixfile) < 0) {
        virReportOOMError();
        return -1;
    }

    ret = qemuMonitorTextMigrate(mon, flags, dest);

    VIR_FREE(dest);

    return ret;
}

int qemuMonitorTextMigrateCancel(qemuMonitorPtr mon)
{
    char *info = NULL;

    if (qemuMonitorCommand(mon, "migrate_cancel", &info) < 0) {
        qemuReportError(VIR_ERR_INTERNAL_ERROR,
                        "%s", _("cannot run monitor command to cancel migration"));
        return -1;
    }
    VIR_FREE(info);

    return 0;
}

int qemuMonitorTextAddUSBDisk(qemuMonitorPtr mon,
                              const char *path)
{
    char *cmd = NULL;
    char *safepath;
    int ret = -1;
    char *info = NULL;

    safepath = qemuMonitorEscapeArg(path);
    if (!safepath) {
        virReportOOMError();
        return -1;
    }

    if (virAsprintf(&cmd, "usb_add disk:%s", safepath) < 0) {
        virReportOOMError();
        goto cleanup;
    }

    if (qemuMonitorCommand(mon, cmd, &info) < 0) {
        qemuReportError(VIR_ERR_INTERNAL_ERROR,
                        "%s", _("cannot run monitor command to add usb disk"));
        goto cleanup;
    }

    /* If the command failed qemu prints:
     * Could not add ... */
    if (strstr(info, "Could not add ")) {
        qemuReportError(VIR_ERR_OPERATION_FAILED,
                        _("unable to add USB disk %s: %s"), path, info);
        goto cleanup;
    }

    ret = 0;

cleanup:
    VIR_FREE(cmd);
    VIR_FREE(safepath);
    VIR_FREE(info);
    return ret;
}


static int qemuMonitorTextAddUSBDevice(qemuMonitorPtr mon,
                                       const char *addr)
{
    char *cmd;
    char *reply = NULL;
    int ret = -1;

    if (virAsprintf(&cmd, "usb_add %s", addr) < 0) {
        virReportOOMError();
        return -1;
    }

    if (qemuMonitorCommand(mon, cmd, &reply) < 0) {
        qemuReportError(VIR_ERR_OPERATION_FAILED,
                        "%s", _("cannot attach usb device"));
        goto cleanup;
    }

    /* If the command failed qemu prints:
     * Could not add ... */
    if (strstr(reply, "Could not add ")) {
        qemuReportError(VIR_ERR_OPERATION_FAILED,
                        "%s", _("adding usb device failed"));
        goto cleanup;
    }

    ret = 0;

cleanup:
    VIR_FREE(cmd);
    VIR_FREE(reply);
    return ret;
}


int qemuMonitorTextAddUSBDeviceExact(qemuMonitorPtr mon,
                                     int bus,
                                     int dev)
{
    int ret;
    char *addr;

    if (virAsprintf(&addr, "host:%.3d.%.3d", bus, dev) < 0) {
        virReportOOMError();
        return -1;
    }

    ret = qemuMonitorTextAddUSBDevice(mon, addr);

    VIR_FREE(addr);
    return ret;
}

int qemuMonitorTextAddUSBDeviceMatch(qemuMonitorPtr mon,
                                     int vendor,
                                     int product)
{
    int ret;
    char *addr;

    if (virAsprintf(&addr, "host:%.4x:%.4x", vendor, product) < 0) {
        virReportOOMError();
        return -1;
    }

    ret = qemuMonitorTextAddUSBDevice(mon, addr);

    VIR_FREE(addr);
    return ret;
}


static int
qemuMonitorTextParsePciAddReply(qemuMonitorPtr mon ATTRIBUTE_UNUSED,
                                const char *reply,
                                virDomainDevicePCIAddress *addr)
{
    char *s, *e;

    /* If the command succeeds qemu prints:
     * OK bus 0, slot XXX...
     * or
     * OK domain 0, bus 0, slot XXX
     */
    if (!(s = strstr(reply, "OK ")))
        return -1;

    s += 3;

    if (STRPREFIX(s, "domain ")) {
        s += strlen("domain ");

        if (virStrToLong_ui(s, &e, 10, &addr->domain) == -1) {
            VIR_WARN("Unable to parse domain number '%s'", s);
            return -1;
        }

        if (!STRPREFIX(e, ", ")) {
            VIR_WARN("Expected ', ' parsing pci_add reply '%s'", s);
            return -1;
        }
        s = e + 2;
    }

    if (!STRPREFIX(s, "bus ")) {
        VIR_WARN("Expected 'bus ' parsing pci_add reply '%s'", s);
        return -1;
    }
    s += strlen("bus ");

    if (virStrToLong_ui(s, &e, 10, &addr->bus) == -1) {
        VIR_WARN("Unable to parse bus number '%s'", s);
        return -1;
    }

    if (!STRPREFIX(e, ", ")) {
        VIR_WARN("Expected ', ' parsing pci_add reply '%s'", s);
        return -1;
    }
    s = e + 2;

    if (!STRPREFIX(s, "slot ")) {
        VIR_WARN("Expected 'slot ' parsing pci_add reply '%s'", s);
        return -1;
    }
    s += strlen("slot ");

    if (virStrToLong_ui(s, &e, 10, &addr->slot) == -1) {
        VIR_WARN("Unable to parse slot number '%s'", s);
        return -1;
    }

    return 0;
}


int qemuMonitorTextAddPCIHostDevice(qemuMonitorPtr mon,
                                    virDomainDevicePCIAddress *hostAddr,
                                    virDomainDevicePCIAddress *guestAddr)
{
    char *cmd;
    char *reply = NULL;
    int ret = -1;

    memset(guestAddr, 0, sizeof(*guestAddr));

    /* XXX hostAddr->domain */
    if (virAsprintf(&cmd, "pci_add pci_addr=auto host host=%.2x:%.2x.%.1x",
                    hostAddr->bus, hostAddr->slot, hostAddr->function) < 0) {
        virReportOOMError();
        goto cleanup;
    }

    if (qemuMonitorCommand(mon, cmd, &reply) < 0) {
        qemuReportError(VIR_ERR_OPERATION_FAILED,
                        "%s", _("cannot attach host pci device"));
        goto cleanup;
    }

    if (strstr(reply, "invalid type: host")) {
        qemuReportError(VIR_ERR_NO_SUPPORT, "%s",
                        _("PCI device assignment is not supported by this version of qemu"));
        goto cleanup;
    }

    if (qemuMonitorTextParsePciAddReply(mon, reply, guestAddr) < 0) {
        qemuReportError(VIR_ERR_OPERATION_FAILED,
                        _("parsing pci_add reply failed: %s"), reply);
        goto cleanup;
    }

    ret = 0;

cleanup:
    VIR_FREE(cmd);
    VIR_FREE(reply);
    return ret;
}


int qemuMonitorTextAddPCIDisk(qemuMonitorPtr mon,
                              const char *path,
                              const char *bus,
                              virDomainDevicePCIAddress *guestAddr)
{
    char *cmd = NULL;
    char *reply = NULL;
    char *safe_path = NULL;
    int tryOldSyntax = 0;
    int ret = -1;

    safe_path = qemuMonitorEscapeArg(path);
    if (!safe_path) {
        virReportOOMError();
        return -1;
    }

try_command:
    if (virAsprintf(&cmd, "pci_add %s storage file=%s,if=%s",
                    (tryOldSyntax ? "0": "pci_addr=auto"), safe_path, bus) < 0) {
        virReportOOMError();
        goto cleanup;
    }

    if (qemuMonitorCommand(mon, cmd, &reply) < 0) {
        qemuReportError(VIR_ERR_OPERATION_FAILED,
                        _("cannot attach %s disk %s"), bus, path);
        goto cleanup;
    }

    if (qemuMonitorTextParsePciAddReply(mon, reply, guestAddr) < 0) {
        if (!tryOldSyntax && strstr(reply, "invalid char in expression")) {
            VIR_FREE(reply);
            VIR_FREE(cmd);
            tryOldSyntax = 1;
            goto try_command;
        }

        qemuReportError(VIR_ERR_OPERATION_FAILED,
                        _("adding %s disk failed %s: %s"), bus, path, reply);
        goto cleanup;
    }

    ret = 0;

cleanup:
    VIR_FREE(safe_path);
    VIR_FREE(cmd);
    VIR_FREE(reply);
    return ret;
}


int qemuMonitorTextAddPCINetwork(qemuMonitorPtr mon,
                                 const char *nicstr,
                                 virDomainDevicePCIAddress *guestAddr)
{
    char *cmd;
    char *reply = NULL;
    int ret = -1;

    if (virAsprintf(&cmd, "pci_add pci_addr=auto nic %s", nicstr) < 0) {
        virReportOOMError();
        return -1;
    }

    if (qemuMonitorCommand(mon, cmd, &reply) < 0) {
        qemuReportError(VIR_ERR_OPERATION_FAILED,
                        _("failed to add NIC with '%s'"), cmd);
        goto cleanup;
    }

    if (qemuMonitorTextParsePciAddReply(mon, reply, guestAddr) < 0) {
        qemuReportError(VIR_ERR_OPERATION_FAILED,
                        _("parsing pci_add reply failed: %s"), reply);
        goto cleanup;
    }

    ret = 0;

cleanup:
    VIR_FREE(reply);
    VIR_FREE(cmd);
    return ret;
}


int qemuMonitorTextRemovePCIDevice(qemuMonitorPtr mon,
                                   virDomainDevicePCIAddress *guestAddr)
{
    char *cmd = NULL;
    char *reply = NULL;
    int tryOldSyntax = 0;
    int ret = -1;

try_command:
    if (tryOldSyntax) {
        if (virAsprintf(&cmd, "pci_del 0 %.2x", guestAddr->slot) < 0) {
            virReportOOMError();
            goto cleanup;
        }
    } else {
        /* XXX function ? */
        if (virAsprintf(&cmd, "pci_del pci_addr=%.4x:%.2x:%.2x",
                        guestAddr->domain, guestAddr->bus, guestAddr->slot) < 0) {
            virReportOOMError();
            goto cleanup;
        }
    }

    if (qemuMonitorCommand(mon, cmd, &reply) < 0) {
        qemuReportError(VIR_ERR_OPERATION_FAILED,
                        "%s", _("failed to remove PCI device"));
        goto cleanup;
    }

    /* Syntax changed when KVM merged PCI hotplug upstream to QEMU,
     * so check for an error message from old KVM indicating the
     * need to try the old syntax */
    if (!tryOldSyntax &&
        strstr(reply, "extraneous characters")) {
        tryOldSyntax = 1;
        VIR_FREE(reply);
        VIR_FREE(cmd);
        goto try_command;
    }
    /* If the command fails due to a wrong slot qemu prints: invalid slot,
     * nothing is printed on success */
    if (strstr(reply, "invalid slot") ||
        strstr(reply, "Invalid pci address")) {
        qemuReportError(VIR_ERR_OPERATION_FAILED,
                        _("failed to detach PCI device, invalid address %.4x:%.2x:%.2x: %s"),
                        guestAddr->domain, guestAddr->bus, guestAddr->slot, reply);
        goto cleanup;
    }

    ret = 0;

cleanup:
    VIR_FREE(cmd);
    VIR_FREE(reply);
    return ret;
}


int qemuMonitorTextSendFileHandle(qemuMonitorPtr mon,
                                  const char *fdname,
                                  int fd)
{
    char *cmd;
    char *reply = NULL;
    int ret = -1;

    if (virAsprintf(&cmd, "getfd %s", fdname) < 0) {
        virReportOOMError();
        return -1;
    }

    if (qemuMonitorCommandWithFd(mon, cmd, fd, &reply) < 0) {
        qemuReportError(VIR_ERR_OPERATION_FAILED,
                        _("failed to pass fd to qemu with '%s'"), cmd);
        goto cleanup;
    }

    /* If the command isn't supported then qemu prints:
     * unknown command: getfd" */
    if (strstr(reply, "unknown command:")) {
        qemuReportError(VIR_ERR_NO_SUPPORT,
                        _("qemu does not support sending of file handles: %s"),
                        reply);
        goto cleanup;
    }

    if (STRNEQ(reply, "")) {
        qemuReportError(VIR_ERR_INTERNAL_ERROR,
                        _("unable to send TAP file handle: %s"),
                        reply);
        goto cleanup;
    }

    ret = 0;

cleanup:
    VIR_FREE(cmd);
    VIR_FREE(reply);
    return ret;
}


int qemuMonitorTextCloseFileHandle(qemuMonitorPtr mon,
                                   const char *fdname)
{
    char *cmd;
    char *reply = NULL;
    int ret = -1;

    if (virAsprintf(&cmd, "closefd %s", fdname) < 0) {
        virReportOOMError();
        return -1;
    }

    if (qemuMonitorCommand(mon, cmd, &reply) < 0) {
        qemuReportError(VIR_ERR_OPERATION_FAILED,
                        _("failed to close fd in qemu with '%s'"), cmd);
        goto cleanup;
    }

    /* If the command isn't supported then qemu prints:
     * unknown command: getfd" */
    if (strstr(reply, "unknown command:")) {
        qemuReportError(VIR_ERR_NO_SUPPORT,
                        _("qemu does not support closing of file handles: %s"),
                        reply);
        goto cleanup;
    }

    ret = 0;

cleanup:
    VIR_FREE(cmd);
    VIR_FREE(reply);
    return ret;
}


int qemuMonitorTextAddHostNetwork(qemuMonitorPtr mon,
                                  const char *netstr)
{
    char *cmd;
    char *reply = NULL;
    int ret = -1;

    if (virAsprintf(&cmd, "host_net_add %s", netstr) < 0) {
        virReportOOMError();
        return -1;
    }

    if (qemuMonitorCommand(mon, cmd, &reply) < 0) {
        qemuReportError(VIR_ERR_OPERATION_FAILED,
                        _("failed to add host net with '%s'"), cmd);
        goto cleanup;
    }

    if (STRNEQ(reply, "")) {
        qemuReportError(VIR_ERR_INTERNAL_ERROR,
                        _("unable to add host net: %s"),
                        reply);
        goto cleanup;
    }

    ret = 0;

cleanup:
    VIR_FREE(cmd);
    VIR_FREE(reply);
    return ret;
}


int qemuMonitorTextRemoveHostNetwork(qemuMonitorPtr mon,
                                     int vlan,
                                     const char *netname)
{
    char *cmd;
    char *reply = NULL;
    int ret = -1;

    if (virAsprintf(&cmd, "host_net_remove %d %s", vlan, netname) < 0) {
        virReportOOMError();
        return -1;
    }

    if (qemuMonitorCommand(mon, cmd, &reply) < 0) {
        qemuReportError(VIR_ERR_OPERATION_FAILED,
                        _("failed to remove host network in qemu with '%s'"), cmd);
        goto cleanup;
    }

    /* XXX error messages here ? */

    ret = 0;

cleanup:
    VIR_FREE(cmd);
    VIR_FREE(reply);
    return ret;
}


int qemuMonitorTextAddNetdev(qemuMonitorPtr mon,
                             const char *netdevstr)
{
    char *cmd;
    char *reply = NULL;
    int ret = -1;

    if (virAsprintf(&cmd, "netdev_add %s", netdevstr) < 0) {
        virReportOOMError();
        return -1;
    }

    if (qemuMonitorCommand(mon, cmd, &reply) < 0) {
        qemuReportError(VIR_ERR_OPERATION_FAILED,
                        _("failed to add netdev with '%s'"), cmd);
        goto cleanup;
    }

    /* XXX error messages here ? */

    ret = 0;

cleanup:
    VIR_FREE(cmd);
    VIR_FREE(reply);
    return ret;
}


int qemuMonitorTextRemoveNetdev(qemuMonitorPtr mon,
                                const char *alias)
{
    char *cmd;
    char *reply = NULL;
    int ret = -1;

    if (virAsprintf(&cmd, "netdev_del %s", alias) < 0) {
        virReportOOMError();
        return -1;
    }

    if (qemuMonitorCommand(mon, cmd, &reply) < 0) {
        qemuReportError(VIR_ERR_OPERATION_FAILED,
                        _("failed to remove netdev in qemu with '%s'"), cmd);
        goto cleanup;
    }

    /* XXX error messages here ? */

    ret = 0;

cleanup:
    VIR_FREE(cmd);
    VIR_FREE(reply);
    return ret;
}


/* Parse the output of "info chardev" and return a hash of pty paths.
 *
 * Output is:
 * foo: filename=pty:/dev/pts/7
 * monitor: filename=stdio
 * serial0: filename=vc
 * parallel0: filename=vc
 *
 * Non-pty lines are ignored. In the above example, key is 'foo', value is
 * '/dev/pty/7'. The hash will contain only a single value.
 */

int qemuMonitorTextGetPtyPaths(qemuMonitorPtr mon,
                               virHashTablePtr paths)
{
    char *reply = NULL;
    int ret = -1;

    if (qemuMonitorCommand(mon, "info chardev", &reply) < 0) {
        qemuReportError(VIR_ERR_OPERATION_FAILED, "%s",
                        _("failed to retrieve chardev info in qemu with 'info chardev'"));
        goto cleanup;
    }

    char *pos;                          /* The current start of searching */
    char *next = reply;                 /* The start of the next line */
    char *eol;                   /* The character which ends the current line */
    char *end = reply + strlen(reply);  /* The end of the reply string */

    while (next) {
        pos = next;

        /* Split the output into lines */
        eol = memchr(pos, '\n', end - pos);
        if (eol == NULL) {
            eol = end;
            next = NULL;
        } else {
            next = eol + 1;
        }

        /* Ignore all whitespace immediately before eol */
        while (eol > pos && c_isspace(*(eol-1)))
            eol -= 1;

        /* Look for 'filename=pty:' */
#define NEEDLE "filename=pty:"
        char *needle = memmem(pos, eol - pos, NEEDLE, strlen(NEEDLE));

        /* If it's not there we can ignore this line */
        if (!needle)
            continue;

        /* id is everthing from the beginning of the line to the ':'
         * find ':' and turn it into a terminator */
        char *colon = memchr(pos, ':', needle - pos);
        if (colon == NULL)
            continue;
        *colon = '\0';
        char *id = pos;

        /* Path is everything after needle to the end of the line */
        *eol = '\0';
        char *path = strdup(needle + strlen(NEEDLE));
        if (path == NULL) {
            virReportOOMError();
            goto cleanup;
        }

        if (virHashAddEntry(paths, id, path) < 0) {
            qemuReportError(VIR_ERR_OPERATION_FAILED,
                            _("failed to save chardev path '%s'"),
                            path);
            VIR_FREE(path);
            goto cleanup;
        }
#undef NEEDLE
    }

    ret = 0;

cleanup:
    VIR_FREE(reply);
    return ret;
}


int qemuMonitorTextAttachPCIDiskController(qemuMonitorPtr mon,
                                           const char *bus,
                                           virDomainDevicePCIAddress *guestAddr)
{
    char *cmd = NULL;
    char *reply = NULL;
    int tryOldSyntax = 0;
    int ret = -1;

try_command:
    if (virAsprintf(&cmd, "pci_add %s storage if=%s",
                    (tryOldSyntax ? "0": "pci_addr=auto"), bus) < 0) {
        virReportOOMError();
        goto cleanup;
    }

    if (qemuMonitorCommand(mon, cmd, &reply) < 0) {
        qemuReportError(VIR_ERR_OPERATION_FAILED,
                        _("cannot attach %s disk controller"), bus);
        goto cleanup;
    }

    if (qemuMonitorTextParsePciAddReply(mon, reply, guestAddr) < 0) {
        if (!tryOldSyntax && strstr(reply, "invalid char in expression")) {
            VIR_FREE(reply);
            VIR_FREE(cmd);
            tryOldSyntax = 1;
            goto try_command;
        }

        qemuReportError(VIR_ERR_OPERATION_FAILED,
                        _("adding %s disk controller failed: %s"), bus, reply);
        goto cleanup;
    }

    ret = 0;

cleanup:
    VIR_FREE(cmd);
    VIR_FREE(reply);
    return ret;
}


static int
qemudParseDriveAddReply(const char *reply,
                        virDomainDeviceDriveAddressPtr addr)
{
    char *s, *e;

    /* If the command succeeds qemu prints:
     * OK bus X, unit Y
     */

    if (!(s = strstr(reply, "OK ")))
        return -1;

    s += 3;

    if (STRPREFIX(s, "bus ")) {
        s += strlen("bus ");

        if (virStrToLong_ui(s, &e, 10, &addr->bus) == -1) {
            VIR_WARN("Unable to parse bus '%s'", s);
            return -1;
        }

        if (!STRPREFIX(e, ", ")) {
            VIR_WARN("Expected ', ' parsing drive_add reply '%s'", s);
            return -1;
        }
        s = e + 2;
    }

    if (!STRPREFIX(s, "unit ")) {
        VIR_WARN("Expected 'unit ' parsing drive_add reply '%s'", s);
        return -1;
    }
    s += strlen("bus ");

    if (virStrToLong_ui(s, &e, 10, &addr->unit) == -1) {
        VIR_WARN("Unable to parse unit number '%s'", s);
        return -1;
    }

    return 0;
}


int qemuMonitorTextAttachDrive(qemuMonitorPtr mon,
                               const char *drivestr,
                               virDomainDevicePCIAddress *controllerAddr,
                               virDomainDeviceDriveAddress *driveAddr)
{
    char *cmd = NULL;
    char *reply = NULL;
    int ret = -1;
    char *safe_str;
    int tryOldSyntax = 0;

    safe_str = qemuMonitorEscapeArg(drivestr);
    if (!safe_str) {
        virReportOOMError();
        return -1;
    }

try_command:
    if (virAsprintf(&cmd, "drive_add %s%.2x:%.2x:%.2x %s",
                    (tryOldSyntax ? "" : "pci_addr="),
                    controllerAddr->domain, controllerAddr->bus,
                    controllerAddr->slot, safe_str) < 0) {
        virReportOOMError();
        goto cleanup;
    }

    if (qemuMonitorCommand(mon, cmd, &reply) < 0) {
        qemuReportError(VIR_ERR_OPERATION_FAILED,
                        _("failed to close fd in qemu with '%s'"), cmd);
        goto cleanup;
    }

    if (strstr(reply, "unknown command:")) {
        qemuReportError(VIR_ERR_OPERATION_FAILED, "%s",
                        _("drive hotplug is not supported"));
        goto cleanup;
    }

    if (qemudParseDriveAddReply(reply, driveAddr) < 0) {
        if (!tryOldSyntax && strstr(reply, "invalid char in expression")) {
            VIR_FREE(reply);
            VIR_FREE(cmd);
            tryOldSyntax = 1;
            goto try_command;
        }
        qemuReportError(VIR_ERR_OPERATION_FAILED,
                        _("adding %s disk failed: %s"), drivestr, reply);
        goto cleanup;
    }

    ret = 0;

cleanup:
    VIR_FREE(cmd);
    VIR_FREE(reply);
    VIR_FREE(safe_str);
    return ret;
}


/*
 * The format we're after looks like this
 *
 *   (qemu) info pci
 *   Bus  0, device   0, function 0:
 *     Host bridge: PCI device 8086:1237
 *       id ""
 *   Bus  0, device   1, function 0:
 *     ISA bridge: PCI device 8086:7000
 *       id ""
 *   Bus  0, device   1, function 1:
 *     IDE controller: PCI device 8086:7010
 *       BAR4: I/O at 0xc000 [0xc00f].
 *       id ""
 *   Bus  0, device   1, function 3:
 *     Bridge: PCI device 8086:7113
 *       IRQ 9.
 *       id ""
 *   Bus  0, device   2, function 0:
 *     VGA controller: PCI device 1013:00b8
 *       BAR0: 32 bit prefetchable memory at 0xf0000000 [0xf1ffffff].
 *       BAR1: 32 bit memory at 0xf2000000 [0xf2000fff].
 *       id ""
 *   Bus  0, device   3, function 0:
 *     Ethernet controller: PCI device 8086:100e
 *      IRQ 11.
 *      BAR0: 32 bit memory at 0xf2020000 [0xf203ffff].
 *      BAR1: I/O at 0xc040 [0xc07f].
 *       id ""
 *
 * Of this, we're interesting in the vendor/product ID
 * and the bus/device/function data.
 */
#define CHECK_END(p) if (!(p)) break;
#define SKIP_TO(p, lbl)                                            \
    (p) = strstr((p), (lbl));                                      \
    if (p)                                                         \
        (p) += strlen(lbl);
#define GET_INT(p, base, val)                                           \
    if (virStrToLong_ui((p), &(p), (base), &(val)) < 0) {               \
        qemuReportError(VIR_ERR_OPERATION_FAILED,                       \
                        _("cannot parse value for %s"), #val);          \
        break;                                                          \
    }
#define SKIP_SPACE(p)                           \
    while (*(p) == ' ') (p)++;

int qemuMonitorTextGetAllPCIAddresses(qemuMonitorPtr mon,
                                      qemuMonitorPCIAddress **retaddrs)
{
    char *reply;
    qemuMonitorPCIAddress *addrs = NULL;
    int naddrs = 0;
    char *p;

    *retaddrs = NULL;

    if (qemuMonitorCommand(mon, "info pci", &reply) < 0) {
        qemuReportError(VIR_ERR_OPERATION_FAILED,
                         "%s", _("cannot query PCI addresses"));
        return -1;
    }

    p = reply;


    while (p) {
        unsigned int bus, slot, func, vendor, product;

        SKIP_TO(p, "  Bus");
        CHECK_END(p);
        SKIP_SPACE(p);
        GET_INT(p, 10, bus);
        CHECK_END(p);

        SKIP_TO(p, ", device");
        CHECK_END(p);
        SKIP_SPACE(p);
        GET_INT(p, 10, slot);
        CHECK_END(p);

        SKIP_TO(p, ", function");
        CHECK_END(p);
        SKIP_SPACE(p);
        GET_INT(p, 10, func);
        CHECK_END(p);

        SKIP_TO(p, "PCI device");
        CHECK_END(p);
        SKIP_SPACE(p);
        GET_INT(p, 16, vendor);
        CHECK_END(p);

        if (*p != ':')
            break;
        p++;
        GET_INT(p, 16, product);

        if (VIR_REALLOC_N(addrs, naddrs+1) < 0) {
            virReportOOMError();
            goto error;
        }

        addrs[naddrs].addr.domain = 0;
        addrs[naddrs].addr.bus = bus;
        addrs[naddrs].addr.slot = slot;
        addrs[naddrs].addr.function = func;
        addrs[naddrs].vendor = vendor;
        addrs[naddrs].product = product;
        naddrs++;

        VIR_DEBUG("Got dev %d:%d:%d   %x:%x", bus, slot, func, vendor, product);
    }

    VIR_FREE(reply);

    *retaddrs = addrs;

    return naddrs;

error:
    VIR_FREE(addrs);
    VIR_FREE(reply);
    return -1;
}
#undef GET_INT
#undef SKIP_SPACE
#undef CHECK_END
#undef SKIP_TO


int qemuMonitorTextDelDevice(qemuMonitorPtr mon,
                             const char *devalias)
{
    char *cmd = NULL;
    char *reply = NULL;
    char *safedev;
    int ret = -1;

    if (!(safedev = qemuMonitorEscapeArg(devalias))) {
        virReportOOMError();
        goto cleanup;
    }

    if (virAsprintf(&cmd, "device_del %s", safedev) < 0) {
        virReportOOMError();
        goto cleanup;
    }

    DEBUG("TextDelDevice devalias=%s", devalias);
    if (qemuMonitorCommand(mon, cmd, &reply) < 0) {
        qemuReportError(VIR_ERR_OPERATION_FAILED,
                        _("cannot detach %s device"), devalias);
        goto cleanup;
    }

    if (STRNEQ(reply, "")) {
        qemuReportError(VIR_ERR_OPERATION_FAILED,
                        _("detaching %s device failed: %s"), devalias, reply);
        goto cleanup;
    }

    ret = 0;

cleanup:
    VIR_FREE(cmd);
    VIR_FREE(reply);
    VIR_FREE(safedev);
    return ret;
}


int qemuMonitorTextAddDevice(qemuMonitorPtr mon,
                             const char *devicestr)
{
    char *cmd = NULL;
    char *reply = NULL;
    char *safedev;
    int ret = -1;

    if (!(safedev = qemuMonitorEscapeArg(devicestr))) {
        virReportOOMError();
        goto cleanup;
    }

    if (virAsprintf(&cmd, "device_add %s", safedev) < 0) {
        virReportOOMError();
        goto cleanup;
    }

    if (qemuMonitorCommand(mon, cmd, &reply) < 0) {
        qemuReportError(VIR_ERR_OPERATION_FAILED,
                        _("cannot attach %s device"), devicestr);
        goto cleanup;
    }

    /* If the command succeeds, no output is sent. So
     * any non-empty string shows an error */
    if (STRNEQ(reply, "")) {
        qemuReportError(VIR_ERR_OPERATION_FAILED,
                        _("adding %s device failed: %s"), devicestr, reply);
        goto cleanup;
    }

    ret = 0;

cleanup:
    VIR_FREE(cmd);
    VIR_FREE(reply);
    VIR_FREE(safedev);
    return ret;
}


int qemuMonitorTextAddDrive(qemuMonitorPtr mon,
                            const char *drivestr)
{
    char *cmd = NULL;
    char *reply = NULL;
    int ret = -1;
    char *safe_str;

    safe_str = qemuMonitorEscapeArg(drivestr);
    if (!safe_str) {
        virReportOOMError();
        return -1;
    }

    /* 'dummy' here is just a placeholder since there is no PCI
     * address required when attaching drives to a controller */
    if (virAsprintf(&cmd, "drive_add dummy %s", safe_str) < 0) {
        virReportOOMError();
        goto cleanup;
    }

    if (qemuMonitorCommand(mon, cmd, &reply) < 0) {
        qemuReportError(VIR_ERR_OPERATION_FAILED,
                        _("failed to close fd in qemu with '%s'"), cmd);
        goto cleanup;
    }

    if (strstr(reply, "unknown command:")) {
        qemuReportError(VIR_ERR_OPERATION_FAILED, "%s",
                        _("drive hotplug is not supported"));
        goto cleanup;
    }

    ret = 0;

cleanup:
    VIR_FREE(cmd);
    VIR_FREE(reply);
    VIR_FREE(safe_str);
    return ret;
}

/* Attempts to remove a host drive.
 * Returns 1 if unsupported, 0 if ok, and -1 on other failure */
int qemuMonitorTextDriveDel(qemuMonitorPtr mon,
                            const char *drivestr)
{
    char *cmd = NULL;
    char *reply = NULL;
    char *safedev;
    int ret = -1;
    DEBUG("TextDriveDel drivestr=%s", drivestr);

    if (!(safedev = qemuMonitorEscapeArg(drivestr))) {
        virReportOOMError();
        goto cleanup;
    }

    if (virAsprintf(&cmd, "drive_del %s", safedev) < 0) {
        virReportOOMError();
        goto cleanup;
    }

    if (qemuMonitorCommand(mon, cmd, &reply) < 0) {
        qemuReportError(VIR_ERR_OPERATION_FAILED,
                        _("cannot delete %s drive"), drivestr);
        goto cleanup;
    }

    if (strstr(reply, "unknown command:")) {
        VIR_ERROR0(_("deleting drive is not supported.  "
                    "This may leak data if disk is reassigned"));
        ret = 1;
        goto cleanup;

    /* (qemu) drive_del wark
     * Device 'wark' not found */
    } else if (STRPREFIX(reply, "Device '") && (strstr(reply, "not found"))) {
        /* NB: device not found errors mean the drive was auto-deleted and we
         * ignore the error */
        ret = 0;
    } else if (STRNEQ(reply, "")) {
        qemuReportError(VIR_ERR_OPERATION_FAILED,
                        _("deleting %s drive failed: %s"), drivestr, reply);
        goto cleanup;
    }

    ret = 0;

cleanup:
    VIR_FREE(cmd);
    VIR_FREE(reply);
    VIR_FREE(safedev);
    return ret;
}

int qemuMonitorTextSetDrivePassphrase(qemuMonitorPtr mon,
                                      const char *alias,
                                      const char *passphrase)
{
    char *cmd = NULL;
    char *reply = NULL;
    int ret = -1;
    char *safe_str;

    safe_str = qemuMonitorEscapeArg(passphrase);
    if (!safe_str) {
        virReportOOMError();
        return -1;
    }

    if (virAsprintf(&cmd, "block_passwd %s%s \"%s\"",
                    QEMU_DRIVE_HOST_PREFIX, alias, safe_str) < 0) {
        virReportOOMError();
        goto cleanup;
    }

    if (qemuMonitorCommand(mon, cmd, &reply) < 0) {
        qemuReportError(VIR_ERR_OPERATION_FAILED,
                        _("failed to close fd in qemu with '%s'"), cmd);
        goto cleanup;
    }

    if (strstr(reply, "unknown command:")) {
        qemuReportError(VIR_ERR_OPERATION_FAILED, "%s",
                        _("setting disk password is not supported"));
        goto cleanup;
    } else if (strstr(reply, "The entered password is invalid")) {
        qemuReportError(VIR_ERR_OPERATION_FAILED, "%s",
                        _("the disk password is incorrect"));
        goto cleanup;
    }

    ret = 0;

cleanup:
    VIR_FREE(cmd);
    VIR_FREE(reply);
    VIR_FREE(safe_str);
    return ret;
}

int qemuMonitorTextCreateSnapshot(qemuMonitorPtr mon, const char *name)
{
    char *cmd;
    char *reply = NULL;
    int ret = -1;

    if (virAsprintf(&cmd, "savevm \"%s\"", name) < 0) {
        virReportOOMError();
        return -1;
    }

    if (qemuMonitorCommand(mon, cmd, &reply)) {
        qemuReportError(VIR_ERR_OPERATION_FAILED,
                        _("failed to take snapshot using command '%s'"), cmd);
        goto cleanup;
    }

    if (strstr(reply, "Error while creating snapshot") != NULL) {
        qemuReportError(VIR_ERR_OPERATION_FAILED,
                        _("Failed to take snapshot: %s"), reply);
        goto cleanup;
    }
    else if (strstr(reply, "No block device can accept snapshots") != NULL) {
        qemuReportError(VIR_ERR_OPERATION_INVALID, "%s",
                        _("this domain does not have a device to take snapshots"));
        goto cleanup;
    }
    else if (strstr(reply, "Could not open VM state file") != NULL) {
        qemuReportError(VIR_ERR_OPERATION_FAILED, "%s", reply);
        goto cleanup;
    }
    else if (strstr(reply, "Error") != NULL
             && strstr(reply, "while writing VM") != NULL) {
        qemuReportError(VIR_ERR_OPERATION_FAILED, "%s", reply);
        goto cleanup;
    }

    ret = 0;

cleanup:
    VIR_FREE(cmd);
    VIR_FREE(reply);
    return ret;
}

int qemuMonitorTextLoadSnapshot(qemuMonitorPtr mon, const char *name)
{
    char *cmd;
    char *reply = NULL;
    int ret = -1;

    if (virAsprintf(&cmd, "loadvm \"%s\"", name) < 0) {
        virReportOOMError();
        return -1;
    }

    if (qemuMonitorCommand(mon, cmd, &reply)) {
        qemuReportError(VIR_ERR_OPERATION_FAILED,
                         _("failed to restore snapshot using command '%s'"),
                         cmd);
        goto cleanup;
    }

    if (strstr(reply, "No block device supports snapshots") != NULL) {
        qemuReportError(VIR_ERR_OPERATION_INVALID, "%s",
                        _("this domain does not have a device to load snapshots"));
        goto cleanup;
    }
    else if (strstr(reply, "Could not find snapshot") != NULL) {
        qemuReportError(VIR_ERR_OPERATION_INVALID,
                         _("the snapshot '%s' does not exist, and was not loaded"),
                         name);
        goto cleanup;
    }
    else if (strstr(reply, "Snapshots not supported on device") != NULL) {
        qemuReportError(VIR_ERR_OPERATION_INVALID, "%s", reply);
        goto cleanup;
    }
    else if (strstr(reply, "Could not open VM state file") != NULL) {
        qemuReportError(VIR_ERR_OPERATION_FAILED, "%s", reply);
        goto cleanup;
    }
    else if (strstr(reply, "Error") != NULL
             && strstr(reply, "while loading VM state") != NULL) {
        qemuReportError(VIR_ERR_OPERATION_FAILED, "%s", reply);
        goto cleanup;
    }
    else if (strstr(reply, "Error") != NULL
             && strstr(reply, "while activating snapshot on") != NULL) {
        qemuReportError(VIR_ERR_OPERATION_FAILED, "%s", reply);
        goto cleanup;
    }

    ret = 0;

cleanup:
    VIR_FREE(cmd);
    VIR_FREE(reply);
    return ret;
}

int qemuMonitorTextDeleteSnapshot(qemuMonitorPtr mon, const char *name)
{
    char *cmd;
    char *reply = NULL;
    int ret = -1;

    if (virAsprintf(&cmd, "delvm \"%s\"", name) < 0) {
        virReportOOMError();
        return -1;
    }
    if (qemuMonitorCommand(mon, cmd, &reply)) {
        qemuReportError(VIR_ERR_OPERATION_FAILED,
                         _("failed to delete snapshot using command '%s'"),
                         cmd);
        goto cleanup;
    }

    if (strstr(reply, "No block device supports snapshots") != NULL) {
        qemuReportError(VIR_ERR_OPERATION_INVALID, "%s",
                        _("this domain does not have a device to delete snapshots"));
        goto cleanup;
    }
    else if (strstr(reply, "Snapshots not supported on device") != NULL) {
        qemuReportError(VIR_ERR_OPERATION_INVALID, "%s", reply);
        goto cleanup;
    }
    else if (strstr(reply, "Error") != NULL
             && strstr(reply, "while deleting snapshot") != NULL) {
        qemuReportError(VIR_ERR_OPERATION_FAILED, "%s", reply);
        goto cleanup;
    }

    ret = 0;

cleanup:
    VIR_FREE(cmd);
    VIR_FREE(reply);
    return ret;
}

int qemuMonitorTextArbitraryCommand(qemuMonitorPtr mon, const char *cmd,
                                    char **reply)
{
    char *safecmd = NULL;
    int ret;

    if (!(safecmd = qemuMonitorEscapeArg(cmd))) {
        virReportOOMError();
        return -1;
    }

    ret = qemuMonitorCommand(mon, safecmd, reply);
    if (ret != 0)
        qemuReportError(VIR_ERR_OPERATION_FAILED,
                        _("failed to run cmd '%s'"), safecmd);

    VIR_FREE(safecmd);

    return ret;
}
