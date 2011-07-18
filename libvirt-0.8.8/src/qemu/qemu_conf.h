/*
 * qemu_conf.h: QEMU configuration management
 *
 * Copyright (C) 2006-2007, 2009-2010 Red Hat, Inc.
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

#ifndef __QEMUD_CONF_H
# define __QEMUD_CONF_H

# include <config.h>
# include <stdbool.h>

# include "ebtables.h"
# include "internal.h"
# include "bridge.h"
# include "capabilities.h"
# include "network_conf.h"
# include "domain_conf.h"
# include "domain_event.h"
# include "threads.h"
# include "security/security_manager.h"
# include "cgroup.h"
# include "pci.h"
# include "cpu_conf.h"
# include "driver.h"
# include "bitmap.h"
# include "macvtap.h"
# include "command.h"
# include "threadpool.h"

# define QEMUD_CPUMASK_LEN CPU_SETSIZE


/* Main driver state */
struct qemud_driver {
    virMutex lock;

    virThreadPoolPtr workerPool;

    int privileged;

    uid_t user;
    gid_t group;
    int dynamicOwnership;

    unsigned int qemuVersion;
    int nextvmid;

    virCgroupPtr cgroup;
    int cgroupControllers;
    char **cgroupDeviceACL;

    virDomainObjList domains;

    brControl *brctl;
    /* These four directories are ones libvirtd uses (so must be root:root
     * to avoid security risk from QEMU processes */
    char *configDir;
    char *autostartDir;
    char *logDir;
    char *stateDir;
    /* These two directories are ones QEMU processes use (so must match
     * the QEMU user/group */
    char *libDir;
    char *cacheDir;
    char *saveDir;
    char *snapshotDir;
    unsigned int vncAutoUnixSocket : 1;
    unsigned int vncTLS : 1;
    unsigned int vncTLSx509verify : 1;
    unsigned int vncSASL : 1;
    char *vncTLSx509certdir;
    char *vncListen;
    char *vncPassword;
    char *vncSASLdir;
    unsigned int spiceTLS : 1;
    char *spiceTLSx509certdir;
    char *spiceListen;
    char *spicePassword;
    char *hugetlbfs_mount;
    char *hugepage_path;

    unsigned int macFilter : 1;
    ebtablesContext *ebtables;

    unsigned int relaxedACS : 1;
    unsigned int vncAllowHostAudio : 1;
    unsigned int clearEmulatorCapabilities : 1;
    unsigned int allowDiskFormatProbing : 1;
    unsigned int setProcessName : 1;

    virCapsPtr caps;

    /* An array of callbacks */
    virDomainEventCallbackListPtr domainEventCallbacks;
    virDomainEventQueuePtr domainEventQueue;
    int domainEventTimer;
    int domainEventDispatching;

    char *securityDriverName;
    virSecurityManagerPtr securityManager;

    char *saveImageFormat;
    char *dumpImageFormat;

    char *autoDumpPath;

    pciDeviceList *activePciHostdevs;

    virBitmapPtr reservedVNCPorts;

    virSysinfoDefPtr hostsysinfo;
};

typedef struct _qemuDomainCmdlineDef qemuDomainCmdlineDef;
typedef qemuDomainCmdlineDef *qemuDomainCmdlineDefPtr;
struct _qemuDomainCmdlineDef {
    unsigned int num_args;
    char **args;

    unsigned int num_env;
    char **env_name;
    char **env_value;
};

/* Port numbers used for KVM migration. */
# define QEMUD_MIGRATION_FIRST_PORT 49152
# define QEMUD_MIGRATION_NUM_PORTS 64

# define qemuReportError(code, ...)                                      \
    virReportErrorHelper(NULL, VIR_FROM_QEMU, code, __FILE__,           \
                         __FUNCTION__, __LINE__, __VA_ARGS__)


void qemuDriverLock(struct qemud_driver *driver);
void qemuDriverUnlock(struct qemud_driver *driver);
int qemudLoadDriverConfig(struct qemud_driver *driver,
                          const char *filename);

#endif /* __QEMUD_CONF_H */
