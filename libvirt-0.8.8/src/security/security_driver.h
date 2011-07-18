/*
 * Copyright (C) 2008, 2010 Red Hat, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Authors:
 *     James Morris <jmorris@namei.org>
 *
 */
#ifndef __VIR_SECURITY_H__
# define __VIR_SECURITY_H__

# include "internal.h"
# include "domain_conf.h"

# include "security_manager.h"

/*
 * Return values for security driver probing: the driver will determine
 * whether it should be enabled or disabled.
 */
typedef enum {
    SECURITY_DRIVER_ENABLE      = 0,
    SECURITY_DRIVER_ERROR       = -1,
    SECURITY_DRIVER_DISABLE     = -2,
} virSecurityDriverStatus;

typedef struct _virSecurityDriver virSecurityDriver;
typedef virSecurityDriver *virSecurityDriverPtr;

typedef virSecurityDriverStatus (*virSecurityDriverProbe) (void);
typedef int (*virSecurityDriverOpen) (virSecurityManagerPtr mgr);
typedef int (*virSecurityDriverClose) (virSecurityManagerPtr mgr);

typedef const char *(*virSecurityDriverGetModel) (virSecurityManagerPtr mgr);
typedef const char *(*virSecurityDriverGetDOI) (virSecurityManagerPtr mgr);

typedef int (*virSecurityDomainRestoreImageLabel) (virSecurityManagerPtr mgr,
                                                   virDomainObjPtr vm,
                                                   virDomainDiskDefPtr disk);
typedef int (*virSecurityDomainSetSocketLabel) (virSecurityManagerPtr mgr,
                                                virDomainObjPtr vm);
typedef int (*virSecurityDomainClearSocketLabel)(virSecurityManagerPtr mgr,
                                                virDomainObjPtr vm);
typedef int (*virSecurityDomainSetImageLabel) (virSecurityManagerPtr mgr,
                                               virDomainObjPtr vm,
                                               virDomainDiskDefPtr disk);
typedef int (*virSecurityDomainRestoreHostdevLabel) (virSecurityManagerPtr mgr,
                                                     virDomainObjPtr vm,
                                                     virDomainHostdevDefPtr dev);
typedef int (*virSecurityDomainSetHostdevLabel) (virSecurityManagerPtr mgr,
                                                 virDomainObjPtr vm,
                                                 virDomainHostdevDefPtr dev);
typedef int (*virSecurityDomainSetSavedStateLabel) (virSecurityManagerPtr mgr,
                                                    virDomainObjPtr vm,
                                                    const char *savefile);
typedef int (*virSecurityDomainRestoreSavedStateLabel) (virSecurityManagerPtr mgr,
                                                        virDomainObjPtr vm,
                                                        const char *savefile);
typedef int (*virSecurityDomainGenLabel) (virSecurityManagerPtr mgr,
                                          virDomainObjPtr sec);
typedef int (*virSecurityDomainReserveLabel) (virSecurityManagerPtr mgr,
                                              virDomainObjPtr sec);
typedef int (*virSecurityDomainReleaseLabel) (virSecurityManagerPtr mgr,
                                              virDomainObjPtr sec);
typedef int (*virSecurityDomainSetAllLabel) (virSecurityManagerPtr mgr,
                                             virDomainObjPtr sec,
                                             const char *stdin_path);
typedef int (*virSecurityDomainRestoreAllLabel) (virSecurityManagerPtr mgr,
                                                 virDomainObjPtr vm,
                                                 int migrated);
typedef int (*virSecurityDomainGetProcessLabel) (virSecurityManagerPtr mgr,
                                                 virDomainObjPtr vm,
                                                 virSecurityLabelPtr sec);
typedef int (*virSecurityDomainSetProcessLabel) (virSecurityManagerPtr mgr,
                                                 virDomainObjPtr vm);
typedef int (*virSecurityDomainSecurityVerify) (virSecurityManagerPtr mgr,
                                                virDomainDefPtr def);
typedef int (*virSecurityDomainSetFDLabel) (virSecurityManagerPtr mgr,
                                            virDomainObjPtr vm,
                                            int fd);

struct _virSecurityDriver {
    size_t privateDataLen;
    const char *name;
    virSecurityDriverProbe probe;
    virSecurityDriverOpen open;
    virSecurityDriverClose close;

    virSecurityDriverGetModel getModel;
    virSecurityDriverGetDOI getDOI;

    virSecurityDomainSecurityVerify domainSecurityVerify;

    virSecurityDomainSetImageLabel domainSetSecurityImageLabel;
    virSecurityDomainRestoreImageLabel domainRestoreSecurityImageLabel;

    virSecurityDomainSetSocketLabel domainSetSecuritySocketLabel;
    virSecurityDomainClearSocketLabel domainClearSecuritySocketLabel;

    virSecurityDomainGenLabel domainGenSecurityLabel;
    virSecurityDomainReserveLabel domainReserveSecurityLabel;
    virSecurityDomainReleaseLabel domainReleaseSecurityLabel;

    virSecurityDomainGetProcessLabel domainGetSecurityProcessLabel;
    virSecurityDomainSetProcessLabel domainSetSecurityProcessLabel;

    virSecurityDomainSetAllLabel domainSetSecurityAllLabel;
    virSecurityDomainRestoreAllLabel domainRestoreSecurityAllLabel;

    virSecurityDomainSetHostdevLabel domainSetSecurityHostdevLabel;
    virSecurityDomainRestoreHostdevLabel domainRestoreSecurityHostdevLabel;

    virSecurityDomainSetSavedStateLabel domainSetSavedStateLabel;
    virSecurityDomainRestoreSavedStateLabel domainRestoreSavedStateLabel;

    virSecurityDomainSetFDLabel domainSetSecurityFDLabel;
};

virSecurityDriverPtr virSecurityDriverLookup(const char *name);

#endif /* __VIR_SECURITY_H__ */
