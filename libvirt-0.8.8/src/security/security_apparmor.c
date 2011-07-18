/*
 * AppArmor security driver for libvirt
 * Copyright (C) 2009-2010 Canonical Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Author:
 *   Jamie Strandboge <jamie@canonical.com>
 *   Based on security_selinux.c by James Morris <jmorris@namei.org>
 *
 * AppArmor security driver.
 */

#include <config.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/apparmor.h>
#include <errno.h>
#include <unistd.h>
#include <wait.h>
#include <stdbool.h>

#include "internal.h"

#include "security_apparmor.h"
#include "util.h"
#include "memory.h"
#include "virterror_internal.h"
#include "datatypes.h"
#include "uuid.h"
#include "pci.h"
#include "hostusb.h"
#include "files.h"
#include "configmake.h"

#define VIR_FROM_THIS VIR_FROM_SECURITY
#define SECURITY_APPARMOR_VOID_DOI      "0"
#define SECURITY_APPARMOR_NAME          "apparmor"
#define VIRT_AA_HELPER LIBEXECDIR "/virt-aa-helper"

/* Data structure to pass to *FileIterate so we have everything we need */
struct SDPDOP {
    virSecurityManagerPtr mgr;
    virDomainObjPtr vm;
};

/*
 * profile_status returns '-1' on error, '0' if loaded
 *
 * If check_enforcing is set to '1', then returns '-1' on error, '0' if
 * loaded in complain mode, and '1' if loaded in enforcing mode.
 */
static int
profile_status(const char *str, const int check_enforcing)
{
    char *content = NULL;
    char *tmp = NULL;
    char *etmp = NULL;
    int rc = -1;

    /* create string that is '<str> \0' for accurate matching */
    if (virAsprintf(&tmp, "%s ", str) == -1) {
        virReportOOMError();
        return rc;
    }

    if (check_enforcing != 0) {
        /* create string that is '<str> (enforce)\0' for accurate matching */
        if (virAsprintf(&etmp, "%s (enforce)", str) == -1) {
            VIR_FREE(tmp);
            virReportOOMError();
            return rc;
        }
    }

    if (virFileReadAll(APPARMOR_PROFILES_PATH, MAX_FILE_LEN, &content) < 0) {
        virReportSystemError(errno,
                             _("Failed to read AppArmor profiles list "
                             "\'%s\'"), APPARMOR_PROFILES_PATH);
        goto clean;
    }

    if (strstr(content, tmp) != NULL)
        rc = 0;
    if (check_enforcing != 0) {
        if (rc == 0 && strstr(content, etmp) != NULL)
            rc = 1;                 /* return '1' if loaded and enforcing */
    }

    VIR_FREE(content);
  clean:
    VIR_FREE(tmp);
    VIR_FREE(etmp);

    return rc;
}

static int
profile_loaded(const char *str)
{
    return profile_status(str, 0);
}

/*
 * profile_status_file returns '-1' on error, '0' if file on disk is in
 * complain mode and '1' if file on disk is in enforcing mode
 */
static int
profile_status_file(const char *str)
{
    char *profile = NULL;
    char *content = NULL;
    char *tmp = NULL;
    int rc = -1;
    int len;

    if (virAsprintf(&profile, "%s/%s", APPARMOR_DIR "/libvirt", str) == -1) {
        virReportOOMError();
        return rc;
    }

    if (!virFileExists(profile))
        goto failed;

    if ((len = virFileReadAll(profile, MAX_FILE_LEN, &content)) < 0) {
        virReportSystemError(errno,
                             _("Failed to read \'%s\'"), profile);
        goto failed;
    }

    /* create string that is ' <str> flags=(complain)\0' */
    if (virAsprintf(&tmp, " %s flags=(complain)", str) == -1) {
        virReportOOMError();
        goto failed;
    }

    if (strstr(content, tmp) != NULL)
        rc = 0;
    else
        rc = 1;

  failed:
    VIR_FREE(tmp);
    VIR_FREE(profile);
    VIR_FREE(content);

    return rc;
}

/*
 * load (add) a profile. Will create one if necessary
 */
static int
load_profile(virSecurityManagerPtr mgr,
             const char *profile,
             virDomainObjPtr vm,
             const char *fn,
             bool append)
{
    int rc = -1, status, ret;
    bool create = true;
    char *xml = NULL;
    int pipefd[2];
    pid_t child;
    const char *probe = virSecurityManagerGetAllowDiskFormatProbing(mgr)
        ? "1" : "0";

    if (pipe(pipefd) < -1) {
        virReportSystemError(errno, "%s", _("unable to create pipe"));
        return rc;
    }

    xml = virDomainDefFormat(vm->def, VIR_DOMAIN_XML_SECURE);
    if (!xml)
        goto clean;

    if (profile_status_file(profile) >= 0)
        create = false;

    if (create) {
        const char *const argv[] = {
            VIRT_AA_HELPER, "-p", probe, "-c", "-u", profile, NULL
        };
        ret = virExec(argv, NULL, NULL, &child,
                      pipefd[0], NULL, NULL, VIR_EXEC_NONE);
    } else if (fn && append) {
        const char *const argv[] = {
            VIRT_AA_HELPER, "-p", probe, "-r", "-u", profile, "-F", fn, NULL
        };
        ret = virExec(argv, NULL, NULL, &child,
                      pipefd[0], NULL, NULL, VIR_EXEC_NONE);
    } else if (fn) {
        const char *const argv[] = {
            VIRT_AA_HELPER, "-p", probe, "-r", "-u", profile, "-f", fn, NULL
        };
        ret = virExec(argv, NULL, NULL, &child,
                      pipefd[0], NULL, NULL, VIR_EXEC_NONE);
    } else {
        const char *const argv[] = {
            VIRT_AA_HELPER, "-p", probe, "-r", "-u", profile, NULL
        };
        ret = virExec(argv, NULL, NULL, &child,
                      pipefd[0], NULL, NULL, VIR_EXEC_NONE);
    }
    if (ret < 0)
        goto clean;

    /* parent continues here */
    if (safewrite(pipefd[1], xml, strlen(xml)) < 0) {
        virReportSystemError(errno, "%s", _("unable to write to pipe"));
        goto clean;
    }
    VIR_FORCE_CLOSE(pipefd[1]);
    rc = 0;

  rewait:
    if (waitpid(child, &status, 0) != child) {
        if (errno == EINTR)
            goto rewait;

        virSecurityReportError(VIR_ERR_INTERNAL_ERROR,
                               _("Unexpected exit status from virt-aa-helper "
                               "%d pid %lu"),
                               WEXITSTATUS(status), (unsigned long)child);
        rc = -1;
    }

  clean:
    VIR_FREE(xml);

    VIR_FORCE_CLOSE(pipefd[0]);
    VIR_FORCE_CLOSE(pipefd[1]);

    return rc;
}

static int
remove_profile(const char *profile)
{
    int rc = -1;
    const char * const argv[] = {
        VIRT_AA_HELPER, "-R", "-u", profile, NULL
    };

    if (virRun(argv, NULL) == 0)
        rc = 0;

    return rc;
}

static char *
get_profile_name(virDomainObjPtr vm)
{
    char uuidstr[VIR_UUID_STRING_BUFLEN];
    char *name = NULL;

    virUUIDFormat(vm->def->uuid, uuidstr);
    if (virAsprintf(&name, "%s%s", AA_PREFIX, uuidstr) < 0) {
        virReportOOMError();
        return NULL;
    }

    return name;
}

/* returns -1 on error or profile for libvirtd is unconfined, 0 if complain
 * mode and 1 if enforcing. This is required because at present you cannot
 * aa_change_profile() from a process that is unconfined.
 */
static int
use_apparmor(void)
{
    int rc = -1;
    char *libvirt_daemon = NULL;

    if (virFileResolveLink("/proc/self/exe", &libvirt_daemon) < 0) {
        virSecurityReportError(VIR_ERR_INTERNAL_ERROR,
                               "%s", _("could not find libvirtd"));
        return rc;
    }

    if (access(APPARMOR_PROFILES_PATH, R_OK) != 0)
        goto cleanup;

    rc = profile_status(libvirt_daemon, 1);

cleanup:
    VIR_FREE(libvirt_daemon);
    return rc;
}

/* reload the profile, adding read/write file specified by fn if it is not
 * NULL.
 */
static int
reload_profile(virSecurityManagerPtr mgr,
               virDomainObjPtr vm,
               const char *fn,
               bool append)
{
    const virSecurityLabelDefPtr secdef = &vm->def->seclabel;
    int rc = -1;
    char *profile_name = NULL;

    if (secdef->type == VIR_DOMAIN_SECLABEL_STATIC)
        return 0;

    if ((profile_name = get_profile_name(vm)) == NULL)
        return rc;

    /* Update the profile only if it is loaded */
    if (profile_loaded(secdef->imagelabel) >= 0) {
        if (load_profile(mgr, secdef->imagelabel, vm, fn, append) < 0) {
            virSecurityReportError(VIR_ERR_INTERNAL_ERROR,
                                   _("cannot update AppArmor profile "
                                     "\'%s\'"),
                                   secdef->imagelabel);
            goto clean;
        }
    }

    rc = 0;
  clean:
    VIR_FREE(profile_name);

    return rc;
}

static int
AppArmorSetSecurityUSBLabel(usbDevice *dev ATTRIBUTE_UNUSED,
                           const char *file, void *opaque)
{
    struct SDPDOP *ptr = opaque;
    virDomainObjPtr vm = ptr->vm;

    if (reload_profile(ptr->mgr, vm, file, true) < 0) {
        const virSecurityLabelDefPtr secdef = &vm->def->seclabel;
        virSecurityReportError(VIR_ERR_INTERNAL_ERROR,
                               _("cannot update AppArmor profile "
                                 "\'%s\'"),
                               secdef->imagelabel);
        return -1;
    }
    return 0;
}

static int
AppArmorSetSecurityPCILabel(pciDevice *dev ATTRIBUTE_UNUSED,
                           const char *file, void *opaque)
{
    struct SDPDOP *ptr = opaque;
    virDomainObjPtr vm = ptr->vm;

    if (reload_profile(ptr->mgr, vm, file, true) < 0) {
        const virSecurityLabelDefPtr secdef = &vm->def->seclabel;
        virSecurityReportError(VIR_ERR_INTERNAL_ERROR,
                               _("cannot update AppArmor profile "
                                 "\'%s\'"),
                               secdef->imagelabel);
        return -1;
    }
    return 0;
}

/* Called on libvirtd startup to see if AppArmor is available */
static int
AppArmorSecurityManagerProbe(void)
{
    char *template = NULL;
    int rc = SECURITY_DRIVER_DISABLE;

    if (use_apparmor() < 0)
        return rc;

    /* see if template file exists */
    if (virAsprintf(&template, "%s/TEMPLATE",
                               APPARMOR_DIR "/libvirt") == -1) {
        virReportOOMError();
        return rc;
    }

    if (!virFileExists(template)) {
        virSecurityReportError(VIR_ERR_INTERNAL_ERROR,
                               _("template \'%s\' does not exist"), template);
        goto clean;
    }
    rc = SECURITY_DRIVER_ENABLE;

  clean:
    VIR_FREE(template);

    return rc;
}

/* Security driver initialization. DOI is for 'Domain of Interpretation' and is
 * currently not used.
 */
static int
AppArmorSecurityManagerOpen(virSecurityManagerPtr mgr ATTRIBUTE_UNUSED)
{
    return 0;
}

static int
AppArmorSecurityManagerClose(virSecurityManagerPtr mgr ATTRIBUTE_UNUSED)
{
    return 0;
}

static const char *
AppArmorSecurityManagerGetModel(virSecurityManagerPtr mgr ATTRIBUTE_UNUSED)
{
    return SECURITY_APPARMOR_NAME;
}

static const char *
AppArmorSecurityManagerGetDOI(virSecurityManagerPtr mgr ATTRIBUTE_UNUSED)
{
    return SECURITY_APPARMOR_VOID_DOI;
}


/* Currently called in qemudStartVMDaemon to setup a 'label'. We look for and
 * use a profile based on the UUID, otherwise create one based on a template.
 * Keep in mind that this is called on 'start' with RestoreSecurityLabel being
 * called on shutdown.
*/
static int
AppArmorGenSecurityLabel(virSecurityManagerPtr mgr ATTRIBUTE_UNUSED,
                         virDomainObjPtr vm)
{
    int rc = -1;
    char *profile_name = NULL;

    if (vm->def->seclabel.type == VIR_DOMAIN_SECLABEL_STATIC)
        return 0;

    if ((vm->def->seclabel.label) ||
        (vm->def->seclabel.model) || (vm->def->seclabel.imagelabel)) {
        virSecurityReportError(VIR_ERR_INTERNAL_ERROR,
                               "%s",
                               _("security label already defined for VM"));
        return rc;
    }

    if ((profile_name = get_profile_name(vm)) == NULL)
        return rc;

    vm->def->seclabel.label = strndup(profile_name, strlen(profile_name));
    if (!vm->def->seclabel.label) {
        virReportOOMError();
        goto clean;
    }

    /* set imagelabel the same as label (but we won't use it) */
    vm->def->seclabel.imagelabel = strndup(profile_name,
                                           strlen(profile_name));
    if (!vm->def->seclabel.imagelabel) {
        virReportOOMError();
        goto err;
    }

    vm->def->seclabel.model = strdup(SECURITY_APPARMOR_NAME);
    if (!vm->def->seclabel.model) {
        virReportOOMError();
        goto err;
    }

    rc = 0;
    goto clean;

  err:
    VIR_FREE(vm->def->seclabel.label);
    VIR_FREE(vm->def->seclabel.imagelabel);
    VIR_FREE(vm->def->seclabel.model);

  clean:
    VIR_FREE(profile_name);

    return rc;
}

static int
AppArmorSetSecurityAllLabel(virSecurityManagerPtr mgr,
                            virDomainObjPtr vm, const char *stdin_path)
{
    if (vm->def->seclabel.type == VIR_DOMAIN_SECLABEL_STATIC)
        return 0;

    /* if the profile is not already loaded, then load one */
    if (profile_loaded(vm->def->seclabel.label) < 0) {
        if (load_profile(mgr, vm->def->seclabel.label, vm, stdin_path,
                         false) < 0) {
            virSecurityReportError(VIR_ERR_INTERNAL_ERROR,
                                   _("cannot generate AppArmor profile "
                                   "\'%s\'"), vm->def->seclabel.label);
            return -1;
        }
    }

    return 0;
}

/* Seen with 'virsh dominfo <vm>'. This function only called if the VM is
 * running.
 */
static int
AppArmorGetSecurityProcessLabel(virSecurityManagerPtr mgr ATTRIBUTE_UNUSED,
                                virDomainObjPtr vm,
                                virSecurityLabelPtr sec)
{
    int rc = -1;
    char *profile_name = NULL;

    if ((profile_name = get_profile_name(vm)) == NULL)
        return rc;

    if (virStrcpy(sec->label, profile_name,
        VIR_SECURITY_LABEL_BUFLEN) == NULL) {
        virSecurityReportError(VIR_ERR_INTERNAL_ERROR,
                               "%s", _("error copying profile name"));
        goto clean;
    }

    if ((sec->enforcing = profile_status(profile_name, 1)) < 0) {
        virSecurityReportError(VIR_ERR_INTERNAL_ERROR,
                               "%s", _("error calling profile_status()"));
        goto clean;
    }
    rc = 0;

  clean:
    VIR_FREE(profile_name);

    return rc;
}

/* Called on VM shutdown and destroy. See AppArmorGenSecurityLabel (above) for
 * more details. Currently called via qemudShutdownVMDaemon.
 */
static int
AppArmorReleaseSecurityLabel(virSecurityManagerPtr mgr ATTRIBUTE_UNUSED,
                             virDomainObjPtr vm)
{
    const virSecurityLabelDefPtr secdef = &vm->def->seclabel;

    VIR_FREE(secdef->model);
    VIR_FREE(secdef->label);
    VIR_FREE(secdef->imagelabel);

    return 0;
}


static int
AppArmorRestoreSecurityAllLabel(virSecurityManagerPtr mgr ATTRIBUTE_UNUSED,
                                virDomainObjPtr vm,
                                int migrated ATTRIBUTE_UNUSED)
{
    const virSecurityLabelDefPtr secdef = &vm->def->seclabel;
    int rc = 0;

    if (secdef->type == VIR_DOMAIN_SECLABEL_DYNAMIC) {
        if ((rc = remove_profile(secdef->label)) != 0) {
            virSecurityReportError(VIR_ERR_INTERNAL_ERROR,
                                   _("could not remove profile for \'%s\'"),
                                   secdef->label);
        }
    }
    return rc;
}

/* Called via virExecWithHook. Output goes to
 * LOCALSTATEDIR/log/libvirt/qemu/<vm name>.log
 */
static int
AppArmorSetSecurityProcessLabel(virSecurityManagerPtr mgr, virDomainObjPtr vm)
{
    const virSecurityLabelDefPtr secdef = &vm->def->seclabel;
    int rc = -1;
    char *profile_name = NULL;

    if ((profile_name = get_profile_name(vm)) == NULL)
        return rc;

    if (STRNEQ(virSecurityManagerGetModel(mgr), secdef->model)) {
        virSecurityReportError(VIR_ERR_INTERNAL_ERROR,
                               _("security label driver mismatch: "
                               "\'%s\' model configured for domain, but "
                               "hypervisor driver is \'%s\'."),
                               secdef->model, virSecurityManagerGetModel(mgr));
        if (use_apparmor() > 0)
            goto clean;
    }

    if (aa_change_profile(profile_name) < 0) {
        virSecurityReportError(VIR_ERR_INTERNAL_ERROR, "%s",
                               _("error calling aa_change_profile()"));
        goto clean;
    }
    rc = 0;

  clean:
    VIR_FREE(profile_name);

    return rc;
}

static int
AppArmorSetSecuritySocketLabel(virSecurityManagerPtr mgr ATTRIBUTE_UNUSED,
                               virDomainObjPtr vm ATTRIBUTE_UNUSED)
{
    return 0;
}

static int
AppArmorClearSecuritySocketLabel(virSecurityManagerPtr mgr ATTRIBUTE_UNUSED,
                                 virDomainObjPtr vm ATTRIBUTE_UNUSED)
{
    return 0;
}


/* Called when hotplugging */
static int
AppArmorRestoreSecurityImageLabel(virSecurityManagerPtr mgr,
                                  virDomainObjPtr vm,
                                  virDomainDiskDefPtr disk ATTRIBUTE_UNUSED)
{
    return reload_profile(mgr, vm, NULL, false);
}

/* Called when hotplugging */
static int
AppArmorSetSecurityImageLabel(virSecurityManagerPtr mgr,
                              virDomainObjPtr vm, virDomainDiskDefPtr disk)
{
    const virSecurityLabelDefPtr secdef = &vm->def->seclabel;
    int rc = -1;
    char *profile_name;

    if (secdef->type == VIR_DOMAIN_SECLABEL_STATIC)
        return 0;

    if (!disk->src || disk->type == VIR_DOMAIN_DISK_TYPE_NETWORK)
        return 0;

    if (secdef->imagelabel) {
        /* if the device doesn't exist, error out */
        if (!virFileExists(disk->src)) {
            virSecurityReportError(VIR_ERR_INTERNAL_ERROR,
                                   _("\'%s\' does not exist"), disk->src);
            return rc;
        }

        if ((profile_name = get_profile_name(vm)) == NULL)
            return rc;

        /* update the profile only if it is loaded */
        if (profile_loaded(secdef->imagelabel) >= 0) {
            if (load_profile(mgr, secdef->imagelabel, vm, disk->src,
                             false) < 0) {
                virSecurityReportError(VIR_ERR_INTERNAL_ERROR,
                                     _("cannot update AppArmor profile "
                                     "\'%s\'"),
                                     secdef->imagelabel);
                goto clean;
            }
        }
    }
    rc = 0;

  clean:
    VIR_FREE(profile_name);

    return rc;
}

static int
AppArmorSecurityVerify(virSecurityManagerPtr mgr ATTRIBUTE_UNUSED,
                       virDomainDefPtr def)
{
    const virSecurityLabelDefPtr secdef = &def->seclabel;

    if (secdef->type == VIR_DOMAIN_SECLABEL_STATIC) {
        if (use_apparmor() < 0 || profile_status(secdef->label, 0) < 0) {
            virSecurityReportError(VIR_ERR_XML_ERROR,
                                   _("Invalid security label \'%s\'"),
                                   secdef->label);
            return -1;
        }
    }
    return 0;
}

static int
AppArmorReserveSecurityLabel(virSecurityManagerPtr mgr ATTRIBUTE_UNUSED,
                             virDomainObjPtr vm ATTRIBUTE_UNUSED)
{
    /* NOOP. Nothing to reserve with AppArmor */
    return 0;
}

static int
AppArmorSetSecurityHostdevLabel(virSecurityManagerPtr mgr,
                                virDomainObjPtr vm,
                                virDomainHostdevDefPtr dev)

{
    const virSecurityLabelDefPtr secdef = &vm->def->seclabel;
    struct SDPDOP *ptr;
    int ret = -1;

    if (secdef->type == VIR_DOMAIN_SECLABEL_STATIC)
        return 0;

    if (dev->mode != VIR_DOMAIN_HOSTDEV_MODE_SUBSYS)
        return 0;

    if (profile_loaded(secdef->imagelabel) < 0)
        return 0;

    if (VIR_ALLOC(ptr) < 0)
        return -1;
    ptr->mgr = mgr;
    ptr->vm = vm;

    switch (dev->source.subsys.type) {
    case VIR_DOMAIN_HOSTDEV_SUBSYS_TYPE_USB: {
        usbDevice *usb = usbGetDevice(dev->source.subsys.u.usb.bus,
                                      dev->source.subsys.u.usb.device);

        if (!usb)
            goto done;

        ret = usbDeviceFileIterate(usb, AppArmorSetSecurityUSBLabel, ptr);
        usbFreeDevice(usb);
        break;
    }

    case VIR_DOMAIN_HOSTDEV_SUBSYS_TYPE_PCI: {
        pciDevice *pci = pciGetDevice(dev->source.subsys.u.pci.domain,
                                      dev->source.subsys.u.pci.bus,
                                      dev->source.subsys.u.pci.slot,
                                      dev->source.subsys.u.pci.function);

        if (!pci)
            goto done;

        ret = pciDeviceFileIterate(pci, AppArmorSetSecurityPCILabel, ptr);
        pciFreeDevice(pci);
        break;
    }

    default:
        ret = 0;
        break;
    }

done:
    VIR_FREE(ptr);
    return ret;
}


static int
AppArmorRestoreSecurityHostdevLabel(virSecurityManagerPtr mgr,
                                    virDomainObjPtr vm,
                                    virDomainHostdevDefPtr dev ATTRIBUTE_UNUSED)

{
    const virSecurityLabelDefPtr secdef = &vm->def->seclabel;
    if (secdef->type == VIR_DOMAIN_SECLABEL_STATIC)
        return 0;

    return reload_profile(mgr, vm, NULL, false);
}

static int
AppArmorSetSavedStateLabel(virSecurityManagerPtr mgr,
                           virDomainObjPtr vm,
                           const char *savefile)
{
    return reload_profile(mgr, vm, savefile, true);
}


static int
AppArmorRestoreSavedStateLabel(virSecurityManagerPtr mgr,
                               virDomainObjPtr vm,
                               const char *savefile ATTRIBUTE_UNUSED)
{
    return reload_profile(mgr, vm, NULL, false);
}

static int
AppArmorSetFDLabel(virSecurityManagerPtr mgr ATTRIBUTE_UNUSED,
                   virDomainObjPtr vm ATTRIBUTE_UNUSED,
                   int fd ATTRIBUTE_UNUSED)
{
    return 0;
}

virSecurityDriver virAppArmorSecurityDriver = {
    0,
    SECURITY_APPARMOR_NAME,
    AppArmorSecurityManagerProbe,
    AppArmorSecurityManagerOpen,
    AppArmorSecurityManagerClose,

    AppArmorSecurityManagerGetModel,
    AppArmorSecurityManagerGetDOI,

    AppArmorSecurityVerify,

    AppArmorSetSecurityImageLabel,
    AppArmorRestoreSecurityImageLabel,

    AppArmorSetSecuritySocketLabel,
    AppArmorClearSecuritySocketLabel,

    AppArmorGenSecurityLabel,
    AppArmorReserveSecurityLabel,
    AppArmorReleaseSecurityLabel,

    AppArmorGetSecurityProcessLabel,
    AppArmorSetSecurityProcessLabel,

    AppArmorSetSecurityAllLabel,
    AppArmorRestoreSecurityAllLabel,

    AppArmorSetSecurityHostdevLabel,
    AppArmorRestoreSecurityHostdevLabel,

    AppArmorSetSavedStateLabel,
    AppArmorRestoreSavedStateLabel,

    AppArmorSetFDLabel,
};
