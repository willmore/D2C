/*
 * storage_backend_logvol.c: storage backend for logical volume handling
 *
 * Copyright (C) 2007-2009 Red Hat, Inc.
 * Copyright (C) 2007-2008 Daniel P. Berrange
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

#include <sys/wait.h>
#include <sys/stat.h>
#include <stdio.h>
#include <regex.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "virterror_internal.h"
#include "storage_backend_logical.h"
#include "storage_conf.h"
#include "util.h"
#include "memory.h"
#include "logging.h"
#include "files.h"

#define VIR_FROM_THIS VIR_FROM_STORAGE

#define PV_BLANK_SECTOR_SIZE 512


static int
virStorageBackendLogicalSetActive(virStoragePoolObjPtr pool,
                                  int on)
{
    const char *cmdargv[4];

    cmdargv[0] = VGCHANGE;
    cmdargv[1] = on ? "-ay" : "-an";
    cmdargv[2] = pool->def->source.name;
    cmdargv[3] = NULL;

    if (virRun(cmdargv, NULL) < 0)
        return -1;

    return 0;
}


static int
virStorageBackendLogicalMakeVol(virStoragePoolObjPtr pool,
                                char **const groups,
                                void *data)
{
    virStorageVolDefPtr vol = NULL;
    unsigned long long offset, size, length;

    /* See if we're only looking for a specific volume */
    if (data != NULL) {
        vol = data;
        if (STRNEQ(vol->name, groups[0]))
            return 0;
    }

    /* Or filling in more data on an existing volume */
    if (vol == NULL)
        vol = virStorageVolDefFindByName(pool, groups[0]);

    /* Or a completely new volume */
    if (vol == NULL) {
        if (VIR_ALLOC(vol) < 0) {
            virReportOOMError();
            return -1;
        }

        vol->type = VIR_STORAGE_VOL_BLOCK;

        if ((vol->name = strdup(groups[0])) == NULL) {
            virReportOOMError();
            virStorageVolDefFree(vol);
            return -1;
        }

        if (VIR_REALLOC_N(pool->volumes.objs,
                          pool->volumes.count + 1)) {
            virReportOOMError();
            virStorageVolDefFree(vol);
            return -1;
        }
        pool->volumes.objs[pool->volumes.count++] = vol;
    }

    if (vol->target.path == NULL) {
        if (virAsprintf(&vol->target.path, "%s/%s",
                        pool->def->target.path, vol->name) < 0) {
            virReportOOMError();
            virStorageVolDefFree(vol);
            return -1;
        }
    }

    if (groups[1] && !STREQ(groups[1], "")) {
        if (virAsprintf(&vol->backingStore.path, "%s/%s",
                        pool->def->target.path, groups[1]) < 0) {
            virReportOOMError();
            virStorageVolDefFree(vol);
            return -1;
        }

        vol->backingStore.format = VIR_STORAGE_POOL_LOGICAL_LVM2;
    }

    if (vol->key == NULL &&
        (vol->key = strdup(groups[2])) == NULL) {
        virReportOOMError();
        return -1;
    }

    if (virStorageBackendUpdateVolInfo(vol, 1) < 0)
        return -1;


    /* Finally fill in extents information */
    if (VIR_REALLOC_N(vol->source.extents,
                      vol->source.nextent + 1) < 0) {
        virReportOOMError();
        return -1;
    }

    if ((vol->source.extents[vol->source.nextent].path =
         strdup(groups[3])) == NULL) {
        virReportOOMError();
        return -1;
    }

    if (virStrToLong_ull(groups[4], NULL, 10, &offset) < 0) {
        virStorageReportError(VIR_ERR_INTERNAL_ERROR,
                              "%s", _("malformed volume extent offset value"));
        return -1;
    }
    if (virStrToLong_ull(groups[5], NULL, 10, &length) < 0) {
        virStorageReportError(VIR_ERR_INTERNAL_ERROR,
                              "%s", _("malformed volume extent length value"));
        return -1;
    }
    if (virStrToLong_ull(groups[6], NULL, 10, &size) < 0) {
        virStorageReportError(VIR_ERR_INTERNAL_ERROR,
                              "%s", _("malformed volume extent size value"));
        return -1;
    }

    vol->source.extents[vol->source.nextent].start = offset * size;
    vol->source.extents[vol->source.nextent].end = (offset * size) + length;
    vol->source.nextent++;

    return 0;
}

static int
virStorageBackendLogicalFindLVs(virStoragePoolObjPtr pool,
                                virStorageVolDefPtr vol)
{
    /*
     *  # lvs --separator , --noheadings --units b --unbuffered --nosuffix --options "lv_name,origin,uuid,devices,seg_size,vg_extent_size" VGNAME
     *  RootLV,,06UgP5-2rhb-w3Bo-3mdR-WeoL-pytO-SAa2ky,/dev/hda2(0),5234491392,33554432
     *  SwapLV,,oHviCK-8Ik0-paqS-V20c-nkhY-Bm1e-zgzU0M,/dev/hda2(156),1040187392,33554432
     *  Test2,,3pg3he-mQsA-5Sui-h0i6-HNmc-Cz7W-QSndcR,/dev/hda2(219),1073741824,33554432
     *  Test3,,UB5hFw-kmlm-LSoX-EI1t-ioVd-h7GL-M0W8Ht,/dev/hda2(251),2181038080,33554432
     *  Test3,Test2,UB5hFw-kmlm-LSoX-EI1t-ioVd-h7GL-M0W8Ht,/dev/hda2(187),1040187392,33554432
     *
     * Pull out name, origin, & uuid, device, device extent start #, segment size, extent size.
     *
     * NB can be multiple rows per volume if they have many extents
     *
     * NB lvs from some distros (e.g. SLES10 SP2) outputs trailing "," on each line
     *
     * NB Encrypted logical volumes can print ':' in their name, so it is
     *    not a suitable separator (rhbz 470693).
     */
    const char *regexes[] = {
        "^\\s*(\\S+),(\\S*),(\\S+),(\\S+)\\((\\S+)\\),(\\S+),([0-9]+),?\\s*$"
    };
    int vars[] = {
        7
    };
    const char *prog[] = {
        LVS, "--separator", ",", "--noheadings", "--units", "b",
        "--unbuffered", "--nosuffix", "--options",
        "lv_name,origin,uuid,devices,seg_size,vg_extent_size",
        pool->def->source.name, NULL
    };

    int exitstatus;

    if (virStorageBackendRunProgRegex(pool,
                                      prog,
                                      1,
                                      regexes,
                                      vars,
                                      virStorageBackendLogicalMakeVol,
                                      vol,
                                      &exitstatus) < 0) {
        virStorageReportError(VIR_ERR_INTERNAL_ERROR,
                              "%s", _("lvs command failed"));
                              return -1;
    }

    if (exitstatus != 0) {
        virStorageReportError(VIR_ERR_INTERNAL_ERROR,
                              _("lvs command failed with exitstatus %d"),
                              exitstatus);
        return -1;
    }

    return 0;
}

static int
virStorageBackendLogicalRefreshPoolFunc(virStoragePoolObjPtr pool ATTRIBUTE_UNUSED,
                                        char **const groups,
                                        void *data ATTRIBUTE_UNUSED)
{
    if (virStrToLong_ull(groups[0], NULL, 10, &pool->def->capacity) < 0)
        return -1;
    if (virStrToLong_ull(groups[1], NULL, 10, &pool->def->available) < 0)
        return -1;
    pool->def->allocation = pool->def->capacity - pool->def->available;

    return 0;
}


static int
virStorageBackendLogicalFindPoolSourcesFunc(virStoragePoolObjPtr pool ATTRIBUTE_UNUSED,
                                            char **const groups,
                                            void *data)
{
    virStoragePoolSourceListPtr sourceList = data;
    char *pvname = NULL;
    char *vgname = NULL;
    int i;
    virStoragePoolSourceDevicePtr dev;
    virStoragePoolSource *thisSource;

    pvname = strdup(groups[0]);
    vgname = strdup(groups[1]);

    if (pvname == NULL || vgname == NULL) {
        virReportOOMError();
        goto err_no_memory;
    }

    thisSource = NULL;
    for (i = 0 ; i < sourceList->nsources; i++) {
        if (STREQ(sourceList->sources[i].name, vgname)) {
            thisSource = &sourceList->sources[i];
            break;
        }
    }

    if (thisSource == NULL) {
        if (!(thisSource = virStoragePoolSourceListNewSource(sourceList)))
            goto err_no_memory;

        thisSource->name = vgname;
    }
    else
        VIR_FREE(vgname);

    if (VIR_REALLOC_N(thisSource->devices, thisSource->ndevice + 1) != 0) {
        virReportOOMError();
        goto err_no_memory;
    }

    dev = &thisSource->devices[thisSource->ndevice];
    thisSource->ndevice++;
    thisSource->format = VIR_STORAGE_POOL_LOGICAL_LVM2;

    memset(dev, 0, sizeof(*dev));
    dev->path = pvname;

    return 0;

 err_no_memory:
    VIR_FREE(pvname);
    VIR_FREE(vgname);

    return -1;
}

static char *
virStorageBackendLogicalFindPoolSources(virConnectPtr conn ATTRIBUTE_UNUSED,
                                        const char *srcSpec ATTRIBUTE_UNUSED,
                                        unsigned int flags ATTRIBUTE_UNUSED)
{
    /*
     * # pvs --noheadings -o pv_name,vg_name
     *   /dev/sdb
     *   /dev/sdc VolGroup00
     */
    const char *regexes[] = {
        "^\\s*(\\S+)\\s+(\\S+)\\s*$"
    };
    int vars[] = {
        2
    };
    const char *const prog[] = { PVS, "--noheadings", "-o", "pv_name,vg_name", NULL };
    const char *const scanprog[] = { VGSCAN, NULL };
    int exitstatus;
    char *retval = NULL;
    virStoragePoolSourceList sourceList;
    int i;

    /*
     * NOTE: ignoring errors here; this is just to "touch" any logical volumes
     * that might be hanging around, so if this fails for some reason, the
     * worst that happens is that scanning doesn't pick everything up
     */
    if (virRun(scanprog, &exitstatus) < 0) {
        VIR_WARN0("Failure when running vgscan to refresh physical volumes");
    }

    memset(&sourceList, 0, sizeof(sourceList));
    sourceList.type = VIR_STORAGE_POOL_LOGICAL;

    if (virStorageBackendRunProgRegex(NULL, prog, 1, regexes, vars,
                                      virStorageBackendLogicalFindPoolSourcesFunc,
                                      &sourceList, &exitstatus) < 0)
        return NULL;

    retval = virStoragePoolSourceListFormat(&sourceList);
    if (retval == NULL) {
        virStorageReportError(VIR_ERR_INTERNAL_ERROR, "%s",
                              _("failed to get source from sourceList"));
        goto cleanup;
    }

 cleanup:
    for (i = 0; i < sourceList.nsources; i++)
        virStoragePoolSourceFree(&sourceList.sources[i]);

    VIR_FREE(sourceList.sources);

    return retval;
}


static int
virStorageBackendLogicalCheckPool(virConnectPtr conn ATTRIBUTE_UNUSED,
                                  virStoragePoolObjPtr pool,
                                  bool *isActive)
{
    char *path;

    *isActive = false;
    if (virAsprintf(&path, "/dev/%s", pool->def->source.name) < 0) {
        virReportOOMError();
        return -1;
    }

    if (access(path, F_OK) == 0)
        *isActive = true;

    VIR_FREE(path);

    return 0;
}

static int
virStorageBackendLogicalStartPool(virConnectPtr conn ATTRIBUTE_UNUSED,
                                  virStoragePoolObjPtr pool)
{
    if (virStorageBackendLogicalSetActive(pool, 1) < 0)
        return -1;

    return 0;
}


static int
virStorageBackendLogicalBuildPool(virConnectPtr conn ATTRIBUTE_UNUSED,
                                  virStoragePoolObjPtr pool,
                                  unsigned int flags ATTRIBUTE_UNUSED)
{
    const char **vgargv;
    const char *pvargv[3];
    int n = 0, i, fd;
    char zeros[PV_BLANK_SECTOR_SIZE];

    memset(zeros, 0, sizeof(zeros));

    if (VIR_ALLOC_N(vgargv, 3 + pool->def->source.ndevice) < 0) {
        virReportOOMError();
        return -1;
    }

    vgargv[n++] = VGCREATE;
    vgargv[n++] = pool->def->source.name;

    pvargv[0] = PVCREATE;
    pvargv[2] = NULL;
    for (i = 0 ; i < pool->def->source.ndevice ; i++) {
        /*
         * LVM requires that the first sector is blanked if using
         * a whole disk as a PV. So we just blank them out regardless
         * rather than trying to figure out if we're a disk or partition
         */
        if ((fd = open(pool->def->source.devices[i].path, O_WRONLY)) < 0) {
            virReportSystemError(errno,
                                 _("cannot open device '%s'"),
                                 pool->def->source.devices[i].path);
            goto cleanup;
        }
        if (safewrite(fd, zeros, sizeof(zeros)) < 0) {
            virReportSystemError(errno,
                                 _("cannot clear device header of '%s'"),
                                 pool->def->source.devices[i].path);
            VIR_FORCE_CLOSE(fd);
            goto cleanup;
        }
        if (VIR_CLOSE(fd) < 0) {
            virReportSystemError(errno,
                                 _("cannot close device '%s'"),
                                 pool->def->source.devices[i].path);
            goto cleanup;
        }

        /*
         * Initialize the physical volume because vgcreate is not
         * clever enough todo this for us :-(
         */
        vgargv[n++] = pool->def->source.devices[i].path;
        pvargv[1] = pool->def->source.devices[i].path;
        if (virRun(pvargv, NULL) < 0)
            goto cleanup;
    }

    vgargv[n] = NULL;

    /* Now create the volume group itself */
    if (virRun(vgargv, NULL) < 0)
        goto cleanup;

    VIR_FREE(vgargv);

    return 0;

 cleanup:
    VIR_FREE(vgargv);
    return -1;
}


static int
virStorageBackendLogicalRefreshPool(virConnectPtr conn ATTRIBUTE_UNUSED,
                                    virStoragePoolObjPtr pool)
{
    /*
     *  # vgs --separator : --noheadings --units b --unbuffered --nosuffix --options "vg_size,vg_free" VGNAME
     *    10603200512:4328521728
     *
     * Pull out size & free
     *
     * NB vgs from some distros (e.g. SLES10 SP2) outputs trailing ":" on each line
     */
    const char *regexes[] = {
        "^\\s*(\\S+):([0-9]+):?\\s*$"
    };
    int vars[] = {
        2
    };
    const char *prog[] = {
        VGS, "--separator", ":", "--noheadings", "--units", "b", "--unbuffered",
        "--nosuffix", "--options", "vg_size,vg_free",
        pool->def->source.name, NULL
    };
    int exitstatus;

    virFileWaitForDevices();

    /* Get list of all logical volumes */
    if (virStorageBackendLogicalFindLVs(pool, NULL) < 0) {
        virStoragePoolObjClearVols(pool);
        return -1;
    }

    /* Now get basic volgrp metadata */
    if (virStorageBackendRunProgRegex(pool,
                                      prog,
                                      1,
                                      regexes,
                                      vars,
                                      virStorageBackendLogicalRefreshPoolFunc,
                                      NULL,
                                      &exitstatus) < 0) {
        virStoragePoolObjClearVols(pool);
        return -1;
    }

    if (exitstatus != 0) {
        virStoragePoolObjClearVols(pool);
        return -1;
    }

    return 0;
}

/*
 * This is actually relatively safe; if you happen to try to "stop" the
 * pool that your / is on, for instance, you will get failure like:
 * "Can't deactivate volume group "VolGroup00" with 3 open logical volume(s)"
 */
static int
virStorageBackendLogicalStopPool(virConnectPtr conn ATTRIBUTE_UNUSED,
                                 virStoragePoolObjPtr pool)
{
    if (virStorageBackendLogicalSetActive(pool, 0) < 0)
        return -1;

    return 0;
}

static int
virStorageBackendLogicalDeletePool(virConnectPtr conn ATTRIBUTE_UNUSED,
                                   virStoragePoolObjPtr pool,
                                   unsigned int flags ATTRIBUTE_UNUSED)
{
    const char *cmdargv[] = {
        VGREMOVE, "-f", pool->def->source.name, NULL
    };
    const char *pvargv[3];
    int i, error;

    /* first remove the volume group */
    if (virRun(cmdargv, NULL) < 0)
        return -1;

    /* now remove the pv devices and clear them out */
    error = 0;
    pvargv[0] = PVREMOVE;
    pvargv[2] = NULL;
    for (i = 0 ; i < pool->def->source.ndevice ; i++) {
        pvargv[1] = pool->def->source.devices[i].path;
        if (virRun(pvargv, NULL) < 0) {
            error = -1;
            virReportSystemError(errno,
                                 _("cannot remove PV device '%s'"),
                                 pool->def->source.devices[i].path);
            break;
        }
    }

    return error;
}


static int
virStorageBackendLogicalDeleteVol(virConnectPtr conn,
                                  virStoragePoolObjPtr pool,
                                  virStorageVolDefPtr vol,
                                  unsigned int flags);


static int
virStorageBackendLogicalCreateVol(virConnectPtr conn,
                                  virStoragePoolObjPtr pool,
                                  virStorageVolDefPtr vol)
{
    int fdret, fd = -1;
    char size[100];
    const char *cmdargvnew[] = {
        LVCREATE, "--name", vol->name, "-L", size,
        pool->def->target.path, NULL
    };
    const char *cmdargvsnap[] = {
        LVCREATE, "--name", vol->name, "-L", size,
        "-s", vol->backingStore.path, NULL
    };
    const char **cmdargv = cmdargvnew;

    if (vol->target.encryption != NULL) {
        virStorageReportError(VIR_ERR_NO_SUPPORT,
                              "%s", _("storage pool does not support encrypted "
                                      "volumes"));
        return -1;
    }

    if (vol->backingStore.path) {
        cmdargv = cmdargvsnap;
    }

    snprintf(size, sizeof(size)-1, "%lluK", VIR_DIV_UP(vol->capacity, 1024));
    size[sizeof(size)-1] = '\0';

    vol->type = VIR_STORAGE_VOL_BLOCK;

    if (vol->target.path != NULL) {
        /* A target path passed to CreateVol has no meaning */
        VIR_FREE(vol->target.path);
    }

    if (virAsprintf(&vol->target.path, "%s/%s",
                    pool->def->target.path,
                    vol->name) == -1) {
        virReportOOMError();
        return -1;
    }

    if (virRun(cmdargv, NULL) < 0)
        return -1;

    if ((fdret = virStorageBackendVolOpen(vol->target.path)) < 0)
        goto cleanup;
    fd = fdret;

    /* We can only chown/grp if root */
    if (getuid() == 0) {
        if (fchown(fd, vol->target.perms.uid, vol->target.perms.gid) < 0) {
            virReportSystemError(errno,
                                 _("cannot set file owner '%s'"),
                                 vol->target.path);
            goto cleanup;
        }
    }
    if (fchmod(fd, vol->target.perms.mode) < 0) {
        virReportSystemError(errno,
                             _("cannot set file mode '%s'"),
                             vol->target.path);
        goto cleanup;
    }

    if (VIR_CLOSE(fd) < 0) {
        virReportSystemError(errno,
                             _("cannot close file '%s'"),
                             vol->target.path);
        goto cleanup;
    }
    fd = -1;

    /* Fill in data about this new vol */
    if (virStorageBackendLogicalFindLVs(pool, vol) < 0) {
        virReportSystemError(errno,
                             _("cannot find newly created volume '%s'"),
                             vol->target.path);
        goto cleanup;
    }

    return 0;

 cleanup:
    VIR_FORCE_CLOSE(fd);
    virStorageBackendLogicalDeleteVol(conn, pool, vol, 0);
    return -1;
}

static int
virStorageBackendLogicalBuildVolFrom(virConnectPtr conn,
                                     virStoragePoolObjPtr pool,
                                     virStorageVolDefPtr vol,
                                     virStorageVolDefPtr inputvol,
                                     unsigned int flags)
{
    virStorageBackendBuildVolFrom build_func;

    build_func = virStorageBackendGetBuildVolFromFunction(vol, inputvol);
    if (!build_func)
        return -1;

    return build_func(conn, pool, vol, inputvol, flags);
}

static int
virStorageBackendLogicalDeleteVol(virConnectPtr conn ATTRIBUTE_UNUSED,
                                  virStoragePoolObjPtr pool ATTRIBUTE_UNUSED,
                                  virStorageVolDefPtr vol,
                                  unsigned int flags ATTRIBUTE_UNUSED)
{
    const char *cmdargv[] = {
        LVREMOVE, "-f", vol->target.path, NULL
    };

    if (virRun(cmdargv, NULL) < 0)
        return -1;

    return 0;
}


virStorageBackend virStorageBackendLogical = {
    .type = VIR_STORAGE_POOL_LOGICAL,

    .findPoolSources = virStorageBackendLogicalFindPoolSources,
    .checkPool = virStorageBackendLogicalCheckPool,
    .startPool = virStorageBackendLogicalStartPool,
    .buildPool = virStorageBackendLogicalBuildPool,
    .refreshPool = virStorageBackendLogicalRefreshPool,
    .stopPool = virStorageBackendLogicalStopPool,
    .deletePool = virStorageBackendLogicalDeletePool,
    .buildVol = NULL,
    .buildVolFrom = virStorageBackendLogicalBuildVolFrom,
    .createVol = virStorageBackendLogicalCreateVol,
    .deleteVol = virStorageBackendLogicalDeleteVol,
};
