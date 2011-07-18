/*
 * xen_internal.h: internal API for direct access to Xen hypervisor level
 *
 * Copyright (C) 2005, 2010 Red Hat, Inc.
 *
 * See COPYING.LIB for the License of this software
 *
 * Daniel Veillard <veillard@redhat.com>
 */

#ifndef __VIR_XEN_INTERNAL_H__
# define __VIR_XEN_INTERNAL_H__

# include <libxml/uri.h>

# include "internal.h"
# include "capabilities.h"
# include "driver.h"

extern struct xenUnifiedDriver xenHypervisorDriver;
int    xenHypervisorInit                 (void);

virCapsPtr xenHypervisorMakeCapabilities (virConnectPtr conn);

int
        xenHypervisorHasDomain(virConnectPtr conn,
                               int id);
virDomainPtr
        xenHypervisorLookupDomainByID   (virConnectPtr conn,
                                         int id);
virDomainPtr
        xenHypervisorLookupDomainByUUID (virConnectPtr conn,
                                         const unsigned char *uuid);
char *
        xenHypervisorDomainGetOSType    (virDomainPtr dom);

virDrvOpenStatus
        xenHypervisorOpen               (virConnectPtr conn,
                                         virConnectAuthPtr auth,
                                         int flags);
int     xenHypervisorClose              (virConnectPtr conn);
int     xenHypervisorGetVersion         (virConnectPtr conn,
                                         unsigned long *hvVer);
virCapsPtr
        xenHypervisorMakeCapabilitiesInternal(virConnectPtr conn,
                                              const char *hostmachine,
                                              FILE *cpuinfo,
                                              FILE *capabilities);
char *
        xenHypervisorGetCapabilities    (virConnectPtr conn);
unsigned long
        xenHypervisorGetDomMaxMemory    (virConnectPtr conn,
                                         int id);
int     xenHypervisorNumOfDomains       (virConnectPtr conn);
int     xenHypervisorListDomains        (virConnectPtr conn,
                                         int *ids,
                                         int maxids);
int     xenHypervisorGetMaxVcpus        (virConnectPtr conn,
                                         const char *type);
int     xenHypervisorDestroyDomain      (virDomainPtr domain)
          ATTRIBUTE_NONNULL (1);
int     xenHypervisorResumeDomain       (virDomainPtr domain)
          ATTRIBUTE_NONNULL (1);
int     xenHypervisorPauseDomain        (virDomainPtr domain)
          ATTRIBUTE_NONNULL (1);
int     xenHypervisorGetDomainInfo        (virDomainPtr domain,
                                           virDomainInfoPtr info)
          ATTRIBUTE_NONNULL (1);
int     xenHypervisorGetDomInfo         (virConnectPtr conn,
                                         int id,
                                         virDomainInfoPtr info);
int     xenHypervisorSetMaxMemory       (virDomainPtr domain,
                                         unsigned long memory)
          ATTRIBUTE_NONNULL (1);
int     xenHypervisorCheckID            (virConnectPtr conn,
                                         int id);
int     xenHypervisorSetVcpus           (virDomainPtr domain,
                                         unsigned int nvcpus)
          ATTRIBUTE_NONNULL (1);
int     xenHypervisorPinVcpu            (virDomainPtr domain,
                                         unsigned int vcpu,
                                         unsigned char *cpumap,
                                         int maplen)
          ATTRIBUTE_NONNULL (1);
int     xenHypervisorGetVcpus           (virDomainPtr domain,
                                         virVcpuInfoPtr info,
                                         int maxinfo,
                                         unsigned char *cpumaps,
                                         int maplen)
          ATTRIBUTE_NONNULL (1);
int     xenHypervisorGetVcpuMax         (virDomainPtr domain)
          ATTRIBUTE_NONNULL (1);

char *  xenHypervisorGetSchedulerType   (virDomainPtr domain,
                                         int *nparams)
          ATTRIBUTE_NONNULL (1);

int     xenHypervisorGetSchedulerParameters(virDomainPtr domain,
                                         virSchedParameterPtr params,
                                         int *nparams)
          ATTRIBUTE_NONNULL (1);

int     xenHypervisorSetSchedulerParameters(virDomainPtr domain,
                                         virSchedParameterPtr params,
                                         int nparams)
          ATTRIBUTE_NONNULL (1);

int     xenHypervisorDomainBlockStats   (virDomainPtr domain,
                                         const char *path,
                                         struct _virDomainBlockStats *stats)
          ATTRIBUTE_NONNULL (1);
int     xenHypervisorDomainInterfaceStats (virDomainPtr domain,
                                         const char *path,
                                         struct _virDomainInterfaceStats *stats)
          ATTRIBUTE_NONNULL (1);

int     xenHypervisorNodeGetCellsFreeMemory(virConnectPtr conn,
                                          unsigned long long *freeMems,
                                          int startCell,
                                          int maxCells);

int	xenHavePrivilege(void);

#endif                          /* __VIR_XEN_INTERNAL_H__ */
