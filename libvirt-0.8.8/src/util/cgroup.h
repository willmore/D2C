/*
 * cgroup.h: Interface to tools for managing cgroups
 *
 * Copyright (C) 2011 Red Hat, Inc.
 * Copyright IBM Corp. 2008
 *
 * See COPYING.LIB for the License of this software
 *
 * Authors:
 *  Dan Smith <danms@us.ibm.com>
 */

#ifndef CGROUP_H
# define CGROUP_H

struct virCgroup;
typedef struct virCgroup *virCgroupPtr;

enum {
    VIR_CGROUP_CONTROLLER_CPU,
    VIR_CGROUP_CONTROLLER_CPUACCT,
    VIR_CGROUP_CONTROLLER_CPUSET,
    VIR_CGROUP_CONTROLLER_MEMORY,
    VIR_CGROUP_CONTROLLER_DEVICES,
    VIR_CGROUP_CONTROLLER_FREEZER,
    VIR_CGROUP_CONTROLLER_BLKIO,

    VIR_CGROUP_CONTROLLER_LAST
};

VIR_ENUM_DECL(virCgroupController);

int virCgroupForDriver(const char *name,
                       virCgroupPtr *group,
                       int privileged,
                       int create);

int virCgroupForDomain(virCgroupPtr driver,
                       const char *name,
                       virCgroupPtr *group,
                       int create);

int virCgroupAddTask(virCgroupPtr group, pid_t pid);

int virCgroupSetBlkioWeight(virCgroupPtr group, unsigned int weight);
int virCgroupGetBlkioWeight(virCgroupPtr group, unsigned int *weight);

int virCgroupSetMemory(virCgroupPtr group, unsigned long long kb);
int virCgroupGetMemoryUsage(virCgroupPtr group, unsigned long *kb);

int virCgroupSetMemoryHardLimit(virCgroupPtr group, unsigned long long kb);
int virCgroupGetMemoryHardLimit(virCgroupPtr group, unsigned long long *kb);
int virCgroupSetMemorySoftLimit(virCgroupPtr group, unsigned long long kb);
int virCgroupGetMemorySoftLimit(virCgroupPtr group, unsigned long long *kb);
int virCgroupSetSwapHardLimit(virCgroupPtr group, unsigned long long kb);
int virCgroupGetSwapHardLimit(virCgroupPtr group, unsigned long long *kb);

int virCgroupDenyAllDevices(virCgroupPtr group);

int virCgroupAllowDevice(virCgroupPtr group,
                         char type,
                         int major,
                         int minor);
int virCgroupAllowDeviceMajor(virCgroupPtr group,
                              char type,
                              int major);
int virCgroupAllowDevicePath(virCgroupPtr group,
                             const char *path);

int virCgroupDenyDevice(virCgroupPtr group,
                        char type,
                        int major,
                        int minor);
int virCgroupDenyDeviceMajor(virCgroupPtr group,
                             char type,
                             int major);
int virCgroupDenyDevicePath(virCgroupPtr group,
                            const char *path);

int virCgroupSetCpuShares(virCgroupPtr group, unsigned long long shares);
int virCgroupGetCpuShares(virCgroupPtr group, unsigned long long *shares);

int virCgroupGetCpuacctUsage(virCgroupPtr group, unsigned long long *usage);

int virCgroupSetFreezerState(virCgroupPtr group, const char *state);
int virCgroupGetFreezerState(virCgroupPtr group, char **state);

int virCgroupRemove(virCgroupPtr group);

void virCgroupFree(virCgroupPtr *group);
bool virCgroupMounted(virCgroupPtr cgroup, int controller);

#endif /* CGROUP_H */
