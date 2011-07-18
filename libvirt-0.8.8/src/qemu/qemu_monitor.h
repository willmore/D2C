/*
 * qemu_monitor.h: interaction with QEMU monitor console
 *
 * Copyright (C) 2006-2011 Red Hat, Inc.
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


#ifndef QEMU_MONITOR_H
# define QEMU_MONITOR_H

# include "internal.h"

# include "domain_conf.h"
# include "hash.h"

typedef struct _qemuMonitor qemuMonitor;
typedef qemuMonitor *qemuMonitorPtr;

typedef struct _qemuMonitorMessage qemuMonitorMessage;
typedef qemuMonitorMessage *qemuMonitorMessagePtr;

typedef int (*qemuMonitorPasswordHandler)(qemuMonitorPtr mon,
                                          qemuMonitorMessagePtr msg,
                                          const char *data,
                                          size_t len,
                                          void *opaque);

struct _qemuMonitorMessage {
    int txFD;

    char *txBuffer;
    int txOffset;
    int txLength;

    char *rxBuffer;
    int rxLength;

    int finished;

    int lastErrno;

    qemuMonitorPasswordHandler passwordHandler;
    void *passwordOpaque;
};

typedef struct _qemuMonitorCallbacks qemuMonitorCallbacks;
typedef qemuMonitorCallbacks *qemuMonitorCallbacksPtr;
struct _qemuMonitorCallbacks {
    void (*destroy)(qemuMonitorPtr mon,
                    virDomainObjPtr vm);

    void (*eofNotify)(qemuMonitorPtr mon,
                      virDomainObjPtr vm,
                      int withError);
    /* XXX we'd really like to avoid virCOnnectPtr here
     * It is required so the callback can find the active
     * secret driver. Need to change this to work like the
     * security drivers do, to avoid this
     */
    int (*diskSecretLookup)(qemuMonitorPtr mon,
                            virConnectPtr conn,
                            virDomainObjPtr vm,
                            const char *path,
                            char **secret,
                            size_t *secretLen);

    int (*domainShutdown)(qemuMonitorPtr mon,
                          virDomainObjPtr vm);
    int (*domainReset)(qemuMonitorPtr mon,
                       virDomainObjPtr vm);
    int (*domainPowerdown)(qemuMonitorPtr mon,
                           virDomainObjPtr vm);
    int (*domainStop)(qemuMonitorPtr mon,
                      virDomainObjPtr vm);
    int (*domainRTCChange)(qemuMonitorPtr mon,
                           virDomainObjPtr vm,
                           long long offset);
    int (*domainWatchdog)(qemuMonitorPtr mon,
                          virDomainObjPtr vm,
                          int action);
    int (*domainIOError)(qemuMonitorPtr mon,
                         virDomainObjPtr vm,
                         const char *diskAlias,
                         int action,
                         const char *reason);
    int (*domainGraphics)(qemuMonitorPtr mon,
                          virDomainObjPtr vm,
                          int phase,
                          int localFamily,
                          const char *localNode,
                          const char *localService,
                          int remoteFamily,
                          const char *remoteNode,
                          const char *remoteService,
                          const char *authScheme,
                          const char *x509dname,
                          const char *saslUsername);
};


char *qemuMonitorEscapeArg(const char *in);
char *qemuMonitorEscapeShell(const char *in);

qemuMonitorPtr qemuMonitorOpen(virDomainObjPtr vm,
                               virDomainChrSourceDefPtr config,
                               int json,
                               qemuMonitorCallbacksPtr cb);

void qemuMonitorClose(qemuMonitorPtr mon);

int qemuMonitorSetCapabilities(qemuMonitorPtr mon);

void qemuMonitorLock(qemuMonitorPtr mon);
void qemuMonitorUnlock(qemuMonitorPtr mon);

int qemuMonitorRef(qemuMonitorPtr mon);
int qemuMonitorUnref(qemuMonitorPtr mon);

/* This API is for use by the internal Text/JSON monitor impl code only */
int qemuMonitorSend(qemuMonitorPtr mon,
                    qemuMonitorMessagePtr msg);

/* XXX same comment about virConnectPtr as above */
int qemuMonitorGetDiskSecret(qemuMonitorPtr mon,
                             virConnectPtr conn,
                             const char *path,
                             char **secret,
                             size_t *secretLen);

int qemuMonitorEmitShutdown(qemuMonitorPtr mon);
int qemuMonitorEmitReset(qemuMonitorPtr mon);
int qemuMonitorEmitPowerdown(qemuMonitorPtr mon);
int qemuMonitorEmitStop(qemuMonitorPtr mon);
int qemuMonitorEmitRTCChange(qemuMonitorPtr mon, long long offset);
int qemuMonitorEmitWatchdog(qemuMonitorPtr mon, int action);
int qemuMonitorEmitIOError(qemuMonitorPtr mon,
                           const char *diskAlias,
                           int action,
                           const char *reason);
int qemuMonitorEmitGraphics(qemuMonitorPtr mon,
                            int phase,
                            int localFamily,
                            const char *localNode,
                            const char *localService,
                            int remoteFamily,
                            const char *remoteNode,
                            const char *remoteService,
                            const char *authScheme,
                            const char *x509dname,
                            const char *saslUsername);


int qemuMonitorStartCPUs(qemuMonitorPtr mon,
                         virConnectPtr conn);
int qemuMonitorStopCPUs(qemuMonitorPtr mon);

int qemuMonitorSystemPowerdown(qemuMonitorPtr mon);

int qemuMonitorGetCPUInfo(qemuMonitorPtr mon,
                          int **pids);
int qemuMonitorGetBalloonInfo(qemuMonitorPtr mon,
                              unsigned long *currmem);
int qemuMonitorGetMemoryStats(qemuMonitorPtr mon,
                              virDomainMemoryStatPtr stats,
                              unsigned int nr_stats);
int qemuMonitorGetBlockStatsInfo(qemuMonitorPtr mon,
                                 const char *devname,
                                 long long *rd_req,
                                 long long *rd_bytes,
                                 long long *wr_req,
                                 long long *wr_bytes,
                                 long long *errs);

int qemuMonitorGetBlockExtent(qemuMonitorPtr mon,
                              const char *devname,
                              unsigned long long *extent);


int qemuMonitorSetVNCPassword(qemuMonitorPtr mon,
                              const char *password);
int qemuMonitorSetPassword(qemuMonitorPtr mon,
                           int type,
                           const char *password,
                           const char *action_if_connected);
int qemuMonitorExpirePassword(qemuMonitorPtr mon,
                              int type,
                              const char *expire_time);
int qemuMonitorSetBalloon(qemuMonitorPtr mon,
                          unsigned long newmem);
int qemuMonitorSetCPU(qemuMonitorPtr mon, int cpu, int online);


/* XXX should we pass the virDomainDiskDefPtr instead
 * and hide devname details inside monitor. Reconsider
 * this when doing the QMP implementation
 */
int qemuMonitorEjectMedia(qemuMonitorPtr mon,
                          const char *devname,
                          bool force);
int qemuMonitorChangeMedia(qemuMonitorPtr mon,
                           const char *devname,
                           const char *newmedia,
                           const char *format);


int qemuMonitorSaveVirtualMemory(qemuMonitorPtr mon,
                                 unsigned long long offset,
                                 size_t length,
                                 const char *path);
int qemuMonitorSavePhysicalMemory(qemuMonitorPtr mon,
                                  unsigned long long offset,
                                  size_t length,
                                  const char *path);

int qemuMonitorSetMigrationSpeed(qemuMonitorPtr mon,
                                 unsigned long bandwidth);

int qemuMonitorSetMigrationDowntime(qemuMonitorPtr mon,
                                    unsigned long long downtime);

enum {
    QEMU_MONITOR_MIGRATION_STATUS_INACTIVE,
    QEMU_MONITOR_MIGRATION_STATUS_ACTIVE,
    QEMU_MONITOR_MIGRATION_STATUS_COMPLETED,
    QEMU_MONITOR_MIGRATION_STATUS_ERROR,
    QEMU_MONITOR_MIGRATION_STATUS_CANCELLED,

    QEMU_MONITOR_MIGRATION_STATUS_LAST
};

VIR_ENUM_DECL(qemuMonitorMigrationStatus)

int qemuMonitorGetMigrationStatus(qemuMonitorPtr mon,
                                  int *status,
                                  unsigned long long *transferred,
                                  unsigned long long *remaining,
                                  unsigned long long *total);

typedef enum {
  QEMU_MONITOR_MIGRATE_BACKGROUND	= 1 << 0,
  QEMU_MONITOR_MIGRATE_NON_SHARED_DISK  = 1 << 1, /* migration with non-shared storage with full disk copy */
  QEMU_MONITOR_MIGRATE_NON_SHARED_INC   = 1 << 2, /* migration with non-shared storage with incremental copy */
  QEMU_MONITOR_MIGRATION_FLAGS_LAST
} QEMU_MONITOR_MIGRATE;

int qemuMonitorMigrateToHost(qemuMonitorPtr mon,
                             unsigned int flags,
                             const char *hostname,
                             int port);

int qemuMonitorMigrateToCommand(qemuMonitorPtr mon,
                                unsigned int flags,
                                const char * const *argv);

/* In general, BS is the smallest fundamental block size we can use to
 * access a block device; everything must be aligned to a multiple of
 * this.  Linux generally supports a BS as small as 512, but with
 * newer disks with 4k sectors, performance is better if we guarantee
 * alignment to the sector size.  However, operating on BS-sized
 * blocks is painfully slow, so we also have a transfer size that is
 * larger but only aligned to the smaller block size.
 */
# define QEMU_MONITOR_MIGRATE_TO_FILE_BS (1024llu * 4)
# define QEMU_MONITOR_MIGRATE_TO_FILE_TRANSFER_SIZE (1024llu * 1024)

int qemuMonitorMigrateToFile(qemuMonitorPtr mon,
                             unsigned int flags,
                             const char * const *argv,
                             const char *target,
                             unsigned long long offset);

int qemuMonitorMigrateToUnix(qemuMonitorPtr mon,
                             unsigned int flags,
                             const char *unixfile);

int qemuMonitorMigrateCancel(qemuMonitorPtr mon);


/* XXX disk driver type eg,  qcow/etc.
 * XXX cache mode
 */
int qemuMonitorAddUSBDisk(qemuMonitorPtr mon,
                          const char *path);

int qemuMonitorAddUSBDeviceExact(qemuMonitorPtr mon,
                                 int bus,
                                 int dev);
int qemuMonitorAddUSBDeviceMatch(qemuMonitorPtr mon,
                                 int vendor,
                                 int product);


int qemuMonitorAddPCIHostDevice(qemuMonitorPtr mon,
                                virDomainDevicePCIAddress *hostAddr,
                                virDomainDevicePCIAddress *guestAddr);

/* XXX disk driver type eg,  qcow/etc.
 * XXX cache mode
 */
int qemuMonitorAddPCIDisk(qemuMonitorPtr mon,
                          const char *path,
                          const char *bus,
                          virDomainDevicePCIAddress *guestAddr);

/* XXX do we really want to hardcode 'nicstr' as the
 * sendable item here
 */
int qemuMonitorAddPCINetwork(qemuMonitorPtr mon,
                             const char *nicstr,
                             virDomainDevicePCIAddress *guestAddr);

int qemuMonitorRemovePCIDevice(qemuMonitorPtr mon,
                               virDomainDevicePCIAddress *guestAddr);


int qemuMonitorSendFileHandle(qemuMonitorPtr mon,
                              const char *fdname,
                              int fd);

int qemuMonitorCloseFileHandle(qemuMonitorPtr mon,
                               const char *fdname);


/* XXX do we really want to hardcode 'netstr' as the
 * sendable item here
 */
int qemuMonitorAddHostNetwork(qemuMonitorPtr mon,
                              const char *netstr);

int qemuMonitorRemoveHostNetwork(qemuMonitorPtr mon,
                                 int vlan,
                                 const char *netname);

int qemuMonitorAddNetdev(qemuMonitorPtr mon,
                         const char *netdevstr);

int qemuMonitorRemoveNetdev(qemuMonitorPtr mon,
                            const char *alias);

int qemuMonitorGetPtyPaths(qemuMonitorPtr mon,
                           virHashTablePtr paths);

int qemuMonitorAttachPCIDiskController(qemuMonitorPtr mon,
                                       const char *bus,
                                       virDomainDevicePCIAddress *guestAddr);

int qemuMonitorAttachDrive(qemuMonitorPtr mon,
                           const char *drivestr,
                           virDomainDevicePCIAddress *controllerAddr,
                           virDomainDeviceDriveAddress *driveAddr);


typedef struct _qemuMonitorPCIAddress qemuMonitorPCIAddress;
struct _qemuMonitorPCIAddress {
    unsigned int vendor;
    unsigned int product;
    virDomainDevicePCIAddress addr;
};

int qemuMonitorGetAllPCIAddresses(qemuMonitorPtr mon,
                                  qemuMonitorPCIAddress **addrs);

int qemuMonitorAddDevice(qemuMonitorPtr mon,
                         const char *devicestr);

int qemuMonitorDelDevice(qemuMonitorPtr mon,
                         const char *devalias);

int qemuMonitorAddDrive(qemuMonitorPtr mon,
                        const char *drivestr);

int qemuMonitorDriveDel(qemuMonitorPtr mon,
                        const char *drivestr);

int qemuMonitorSetDrivePassphrase(qemuMonitorPtr mon,
                                  const char *alias,
                                  const char *passphrase);

int qemuMonitorCreateSnapshot(qemuMonitorPtr mon, const char *name);
int qemuMonitorLoadSnapshot(qemuMonitorPtr mon, const char *name);
int qemuMonitorDeleteSnapshot(qemuMonitorPtr mon, const char *name);

int qemuMonitorArbitraryCommand(qemuMonitorPtr mon,
                                const char *cmd,
                                char **reply,
                                bool hmp);

/**
 * When running two dd process and using <> redirection, we need a
 * shell that will not truncate files.  These two strings serve that
 * purpose.
 */
# ifdef VIR_WRAPPER_SHELL
#  define VIR_WRAPPER_SHELL_PREFIX VIR_WRAPPER_SHELL " -c '"
#  define VIR_WRAPPER_SHELL_SUFFIX "'"
# else
#  define VIR_WRAPPER_SHELL_PREFIX /* nothing */
#  define VIR_WRAPPER_SHELL_SUFFIX /* nothing */
# endif

#endif /* QEMU_MONITOR_H */
