/*
 * storage_backend_fs.c: storage backend for FS and directory handling
 *
 * Copyright (C) 2007-2010 Red Hat, Inc.
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

#include <sys/statvfs.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <stdio.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>

#include "virterror_internal.h"
#include "storage_backend_fs.h"
#include "storage_conf.h"
#include "storage_file.h"
#include "util.h"
#include "memory.h"
#include "xml.h"
#include "files.h"

#define VIR_FROM_THIS VIR_FROM_STORAGE

static int ATTRIBUTE_NONNULL(2) ATTRIBUTE_NONNULL(3)
virStorageBackendProbeTarget(virStorageVolTargetPtr target,
                             char **backingStore,
                             int *backingStoreFormat,
                             unsigned long long *allocation,
                             unsigned long long *capacity,
                             virStorageEncryptionPtr *encryption)
{
    int fd, ret;
    virStorageFileMetadata meta;

    *backingStore = NULL;
    *backingStoreFormat = VIR_STORAGE_FILE_AUTO;
    if (encryption)
        *encryption = NULL;

    if ((ret = virStorageBackendVolOpenCheckMode(target->path,
                                                 (VIR_STORAGE_VOL_OPEN_DEFAULT & ~VIR_STORAGE_VOL_OPEN_ERROR))) < 0)
        return ret; /* Take care to propagate ret, it is not always -1 */
    fd = ret;

    if ((ret = virStorageBackendUpdateVolTargetInfoFD(target, fd,
                                                      allocation,
                                                      capacity)) < 0) {
        VIR_FORCE_CLOSE(fd);
        return ret;
    }

    memset(&meta, 0, sizeof(meta));

    if ((target->format = virStorageFileProbeFormatFromFD(target->path, fd)) < 0) {
        VIR_FORCE_CLOSE(fd);
        return -1;
    }

    if (virStorageFileGetMetadataFromFD(target->path, fd,
                                        target->format,
                                        &meta) < 0) {
        VIR_FORCE_CLOSE(fd);
        return -1;
    }

    VIR_FORCE_CLOSE(fd);

    if (meta.backingStore) {
        *backingStore = meta.backingStore;
        meta.backingStore = NULL;
        if (meta.backingStoreFormat == VIR_STORAGE_FILE_AUTO) {
            if ((*backingStoreFormat
                 = virStorageFileProbeFormat(*backingStore)) < 0) {
                VIR_FORCE_CLOSE(fd);
                goto cleanup;
            }
        } else {
            *backingStoreFormat = meta.backingStoreFormat;
        }
    } else {
        VIR_FREE(meta.backingStore);
    }

    if (capacity && meta.capacity)
        *capacity = meta.capacity;

    if (encryption != NULL && meta.encrypted) {
        if (VIR_ALLOC(*encryption) < 0) {
            virReportOOMError();
            goto cleanup;
        }

        switch (target->format) {
        case VIR_STORAGE_FILE_QCOW:
        case VIR_STORAGE_FILE_QCOW2:
            (*encryption)->format = VIR_STORAGE_ENCRYPTION_FORMAT_QCOW;
            break;
        default:
            break;
        }

        /* XXX ideally we'd fill in secret UUID here
         * but we cannot guarentee 'conn' is non-NULL
         * at this point in time :-(  So we only fill
         * in secrets when someone first queries a vol
         */
    }

    return 0;

cleanup:
    VIR_FREE(*backingStore);
    return -1;
}

#if WITH_STORAGE_FS

# include <mntent.h>

struct _virNetfsDiscoverState {
    const char *host;
    virStoragePoolSourceList list;
};

typedef struct _virNetfsDiscoverState virNetfsDiscoverState;

static int
virStorageBackendFileSystemNetFindPoolSourcesFunc(virStoragePoolObjPtr pool ATTRIBUTE_UNUSED,
                                                  char **const groups,
                                                  void *data)
{
    virNetfsDiscoverState *state = data;
    const char *name, *path;
    virStoragePoolSource *src = NULL;
    int ret = -1;

    path = groups[0];

    name = strrchr(path, '/');
    if (name == NULL) {
        virStorageReportError(VIR_ERR_INTERNAL_ERROR,
                              _("invalid netfs path (no /): %s"), path);
        goto cleanup;
    }
    name += 1;
    if (*name == '\0') {
        virStorageReportError(VIR_ERR_INTERNAL_ERROR,
                              _("invalid netfs path (ends in /): %s"), path);
        goto cleanup;
    }

    if (!(src = virStoragePoolSourceListNewSource(&state->list)))
        goto cleanup;

    if (!(src->host.name = strdup(state->host)) ||
        !(src->dir = strdup(path))) {
        virReportOOMError();
        goto cleanup;
    }
    src->format = VIR_STORAGE_POOL_NETFS_NFS;

    src = NULL;
    ret = 0;
cleanup:
    virStoragePoolSourceFree(src);
    VIR_FREE(src);
    return ret;
}


static char *
virStorageBackendFileSystemNetFindPoolSources(virConnectPtr conn ATTRIBUTE_UNUSED,
                                              const char *srcSpec,
                                              unsigned int flags ATTRIBUTE_UNUSED)
{
    /*
     *  # showmount --no-headers -e HOSTNAME
     *  /tmp   *
     *  /A dir demo1.foo.bar,demo2.foo.bar
     *
     * Extract directory name (including possible interior spaces ...).
     */

    const char *regexes[] = {
        "^(/.*\\S) +\\S+$"
    };
    int vars[] = {
        1
    };
    virNetfsDiscoverState state = {
        .host = NULL,
        .list = {
            .type = VIR_STORAGE_POOL_NETFS,
            .nsources = 0,
            .sources = NULL
        }
    };
    const char *prog[] = { SHOWMOUNT, "--no-headers", "--exports", NULL, NULL };
    virStoragePoolSourcePtr source = NULL;
    int exitstatus;
    char *retval = NULL;
    unsigned int i;

    source = virStoragePoolDefParseSourceString(srcSpec,
                                                VIR_STORAGE_POOL_NETFS);
    if (!source)
        goto cleanup;

    state.host = source->host.name;
    prog[3] = source->host.name;

    if (virStorageBackendRunProgRegex(NULL, prog, 1, regexes, vars,
                                      virStorageBackendFileSystemNetFindPoolSourcesFunc,
                                      &state, &exitstatus) < 0)
        goto cleanup;

    retval = virStoragePoolSourceListFormat(&state.list);
    if (retval == NULL) {
        virReportOOMError();
        goto cleanup;
    }

 cleanup:
    for (i = 0; i < state.list.nsources; i++)
        virStoragePoolSourceFree(&state.list.sources[i]);

    virStoragePoolSourceFree(source);
    VIR_FREE(source);

    VIR_FREE(state.list.sources);

    return retval;
}


/**
 * @conn connection to report errors against
 * @pool storage pool to check for status
 *
 * Determine if a storage pool is already mounted
 *
 * Return 0 if not mounted, 1 if mounted, -1 on error
 */
static int
virStorageBackendFileSystemIsMounted(virStoragePoolObjPtr pool) {
    FILE *mtab;
    struct mntent ent;
    char buf[1024];

    if ((mtab = fopen(_PATH_MOUNTED, "r")) == NULL) {
        virReportSystemError(errno,
                             _("cannot read mount list '%s'"),
                             _PATH_MOUNTED);
        return -1;
    }

    while ((getmntent_r(mtab, &ent, buf, sizeof(buf))) != NULL) {
        if (STREQ(ent.mnt_dir, pool->def->target.path)) {
            VIR_FORCE_FCLOSE(mtab);
            return 1;
        }
    }

    VIR_FORCE_FCLOSE(mtab);
    return 0;
}

/**
 * @conn connection to report errors against
 * @pool storage pool to mount
 *
 * Ensure that a FS storage pool is mounted on its target location.
 * If already mounted, this is a no-op
 *
 * Returns 0 if successfully mounted, -1 on error
 */
static int
virStorageBackendFileSystemMount(virStoragePoolObjPtr pool) {
    char *src;
    char *options = NULL;
    const char **mntargv;

    /* 'mount -t auto' doesn't seem to auto determine nfs (or cifs),
     *  while plain 'mount' does. We have to craft separate argvs to
     *  accommodate this */
    int netauto = (pool->def->type == VIR_STORAGE_POOL_NETFS &&
                   pool->def->source.format == VIR_STORAGE_POOL_NETFS_AUTO);
    int glusterfs = (pool->def->type == VIR_STORAGE_POOL_NETFS &&
                 pool->def->source.format == VIR_STORAGE_POOL_NETFS_GLUSTERFS);

    int option_index;
    int source_index;

    const char *netfs_auto_argv[] = {
        MOUNT,
        NULL, /* source path */
        pool->def->target.path,
        NULL,
    };

    const char *fs_argv[] =  {
        MOUNT,
        "-t",
        pool->def->type == VIR_STORAGE_POOL_FS ?
        virStoragePoolFormatFileSystemTypeToString(pool->def->source.format) :
        virStoragePoolFormatFileSystemNetTypeToString(pool->def->source.format),
        NULL, /* Fill in shortly - careful not to add extra fields
                 before this */
        pool->def->target.path,
        NULL,
    };

    const char *glusterfs_argv[] = {
        MOUNT,
        "-t",
        pool->def->type == VIR_STORAGE_POOL_FS ?
        virStoragePoolFormatFileSystemTypeToString(pool->def->source.format) :
        virStoragePoolFormatFileSystemNetTypeToString(pool->def->source.format),
        NULL,
        "-o",
        NULL,
        pool->def->target.path,
        NULL,
    };

    if (netauto) {
        mntargv = netfs_auto_argv;
        source_index = 1;
    } else if (glusterfs) {
        mntargv = glusterfs_argv;
        source_index = 3;
        option_index = 5;
    } else {
        mntargv = fs_argv;
        source_index = 3;
    }

    int ret;

    if (pool->def->type == VIR_STORAGE_POOL_NETFS) {
        if (pool->def->source.host.name == NULL) {
            virStorageReportError(VIR_ERR_INTERNAL_ERROR,
                                  "%s", _("missing source host"));
            return -1;
        }
        if (pool->def->source.dir == NULL) {
            virStorageReportError(VIR_ERR_INTERNAL_ERROR,
                                  "%s", _("missing source path"));
            return -1;
        }
    } else {
        if (pool->def->source.ndevice != 1) {
            virStorageReportError(VIR_ERR_INTERNAL_ERROR,
                                  "%s", _("missing source device"));
            return -1;
        }
    }

    /* Short-circuit if already mounted */
    if ((ret = virStorageBackendFileSystemIsMounted(pool)) != 0) {
        if (ret < 0)
            return -1;
        else
            return 0;
    }

    if (pool->def->type == VIR_STORAGE_POOL_NETFS) {
        if (pool->def->source.format == VIR_STORAGE_POOL_NETFS_GLUSTERFS) {
            if ((options = strdup("direct-io-mode=1")) == NULL) {
                virReportOOMError();
                return -1;
            }
        }
        if (virAsprintf(&src, "%s:%s",
                        pool->def->source.host.name,
                        pool->def->source.dir) == -1) {
            virReportOOMError();
            return -1;
        }

    } else {
        if ((src = strdup(pool->def->source.devices[0].path)) == NULL) {
            virReportOOMError();
            return -1;
        }
    }
    mntargv[source_index] = src;

    if (glusterfs) {
        mntargv[option_index] = options;
    }

    if (virRun(mntargv, NULL) < 0) {
        VIR_FREE(src);
        return -1;
    }
    VIR_FREE(src);
    return 0;
}

/**
 * @conn connection to report errors against
 * @pool storage pool to unmount
 *
 * Ensure that a FS storage pool is not mounted on its target location.
 * If already unmounted, this is a no-op
 *
 * Returns 0 if successfully unmounted, -1 on error
 */
static int
virStorageBackendFileSystemUnmount(virStoragePoolObjPtr pool) {
    const char *mntargv[3];
    int ret;

    if (pool->def->type == VIR_STORAGE_POOL_NETFS) {
        if (pool->def->source.host.name == NULL) {
            virStorageReportError(VIR_ERR_INTERNAL_ERROR,
                                  "%s", _("missing source host"));
            return -1;
        }
        if (pool->def->source.dir == NULL) {
            virStorageReportError(VIR_ERR_INTERNAL_ERROR,
                                  "%s", _("missing source dir"));
            return -1;
        }
    } else {
        if (pool->def->source.ndevice != 1) {
            virStorageReportError(VIR_ERR_INTERNAL_ERROR,
                                  "%s", _("missing source device"));
            return -1;
        }
    }

    /* Short-circuit if already unmounted */
    if ((ret = virStorageBackendFileSystemIsMounted(pool)) != 1) {
        if (ret < 0)
            return -1;
        else
            return 0;
    }

    mntargv[0] = UMOUNT;
    mntargv[1] = pool->def->target.path;
    mntargv[2] = NULL;

    if (virRun(mntargv, NULL) < 0) {
        return -1;
    }
    return 0;
}
#endif /* WITH_STORAGE_FS */


static int
virStorageBackendFileSystemCheck(virConnectPtr conn ATTRIBUTE_UNUSED,
                                 virStoragePoolObjPtr pool,
                                 bool *isActive)
{
    *isActive = false;
    if (pool->def->type == VIR_STORAGE_POOL_DIR) {
        if (access(pool->def->target.path, F_OK) == 0)
            *isActive = true;
#if WITH_STORAGE_FS
    } else {
        int ret;
        if ((ret = virStorageBackendFileSystemIsMounted(pool)) != 0) {
            if (ret < 0)
                return -1;
            *isActive = true;
        }
#endif /* WITH_STORAGE_FS */
    }

    return 0;
}

#if WITH_STORAGE_FS
/**
 * @conn connection to report errors against
 * @pool storage pool to start
 *
 * Starts a directory or FS based storage pool.
 *
 *  - If it is a FS based pool, mounts the unlying source device on the pool
 *
 * Returns 0 on success, -1 on error
 */
static int
virStorageBackendFileSystemStart(virConnectPtr conn ATTRIBUTE_UNUSED,
                                 virStoragePoolObjPtr pool)
{
    if (pool->def->type != VIR_STORAGE_POOL_DIR &&
        virStorageBackendFileSystemMount(pool) < 0)
        return -1;

    return 0;
}
#endif /* WITH_STORAGE_FS */


/**
 * @conn connection to report errors against
 * @pool storage pool to build
 *
 * Build a directory or FS based storage pool.
 *
 *  - If it is a FS based pool, mounts the unlying source device on the pool
 *
 * Returns 0 on success, -1 on error
 */
static int
virStorageBackendFileSystemBuild(virConnectPtr conn ATTRIBUTE_UNUSED,
                                 virStoragePoolObjPtr pool,
                                 unsigned int flags ATTRIBUTE_UNUSED)
{
    int err, ret = -1;
    char *parent;
    char *p;

    if ((parent = strdup(pool->def->target.path)) == NULL) {
        virReportOOMError();
        goto error;
    }
    if (!(p = strrchr(parent, '/'))) {
        virStorageReportError(VIR_ERR_INVALID_ARG,
                              _("path '%s' is not absolute"),
                              pool->def->target.path);
        goto error;
    }

    if (p != parent) {
        /* assure all directories in the path prior to the final dir
         * exist, with default uid/gid/mode. */
        *p = '\0';
        if ((err = virFileMakePath(parent)) != 0) {
            virReportSystemError(err, _("cannot create path '%s'"),
                                 parent);
            goto error;
        }
    }

    /* Now create the final dir in the path with the uid/gid/mode
     * requested in the config. If the dir already exists, just set
     * the perms. */

    struct stat st;

    if ((stat(pool->def->target.path, &st) < 0)
        || (pool->def->target.perms.uid != -1)) {

        uid_t uid = (pool->def->target.perms.uid == -1)
            ? getuid() : pool->def->target.perms.uid;
        gid_t gid = (pool->def->target.perms.gid == -1)
            ? getgid() : pool->def->target.perms.gid;

        if ((err = virDirCreate(pool->def->target.path,
                                pool->def->target.perms.mode,
                                uid, gid,
                                VIR_DIR_CREATE_FORCE_PERMS |
                                VIR_DIR_CREATE_ALLOW_EXIST |
                                (pool->def->type == VIR_STORAGE_POOL_NETFS
                                 ? VIR_DIR_CREATE_AS_UID : 0)) < 0)) {
            virReportSystemError(-err, _("cannot create path '%s'"),
                                 pool->def->target.path);
            goto error;
        }
    }
    ret = 0;
error:
    VIR_FREE(parent);
    return ret;
}


/**
 * Iterate over the pool's directory and enumerate all disk images
 * within it. This is non-recursive.
 */
static int
virStorageBackendFileSystemRefresh(virConnectPtr conn ATTRIBUTE_UNUSED,
                                   virStoragePoolObjPtr pool)
{
    DIR *dir;
    struct dirent *ent;
    struct statvfs sb;
    virStorageVolDefPtr vol = NULL;

    if (!(dir = opendir(pool->def->target.path))) {
        virReportSystemError(errno,
                             _("cannot open path '%s'"),
                             pool->def->target.path);
        goto cleanup;
    }

    while ((ent = readdir(dir)) != NULL) {
        int ret;
        char *backingStore;
        int backingStoreFormat;

        if (VIR_ALLOC(vol) < 0)
            goto no_memory;

        if ((vol->name = strdup(ent->d_name)) == NULL)
            goto no_memory;

        vol->type = VIR_STORAGE_VOL_FILE;
        vol->target.format = VIR_STORAGE_FILE_RAW; /* Real value is filled in during probe */
        if (virAsprintf(&vol->target.path, "%s/%s",
                        pool->def->target.path,
                        vol->name) == -1)
            goto no_memory;

        if ((vol->key = strdup(vol->target.path)) == NULL)
            goto no_memory;

        if ((ret = virStorageBackendProbeTarget(&vol->target,
                                                &backingStore,
                                                &backingStoreFormat,
                                                &vol->allocation,
                                                &vol->capacity,
                                                &vol->target.encryption)) < 0) {
            if (ret == -1)
                goto cleanup;
            else {
                /* Silently ignore non-regular files,
                 * eg '.' '..', 'lost+found', dangling symbolic link */
                virStorageVolDefFree(vol);
                vol = NULL;
                continue;
            }
        }

        if (backingStore != NULL) {
            vol->backingStore.path = backingStore;
            vol->backingStore.format = backingStoreFormat;

            if (virStorageBackendUpdateVolTargetInfo(&vol->backingStore,
                                                     NULL,
                                                     NULL) < 0) {
                VIR_FREE(vol->backingStore.path);
                goto cleanup;
            }
        }


        if (VIR_REALLOC_N(pool->volumes.objs,
                          pool->volumes.count+1) < 0)
            goto no_memory;
        pool->volumes.objs[pool->volumes.count++] = vol;
        vol = NULL;
    }
    closedir(dir);


    if (statvfs(pool->def->target.path, &sb) < 0) {
        virReportSystemError(errno,
                             _("cannot statvfs path '%s'"),
                             pool->def->target.path);
        return -1;
    }
    pool->def->capacity = ((unsigned long long)sb.f_frsize *
                           (unsigned long long)sb.f_blocks);
    pool->def->available = ((unsigned long long)sb.f_bfree *
                            (unsigned long long)sb.f_bsize);
    pool->def->allocation = pool->def->capacity - pool->def->available;

    return 0;

no_memory:
    virReportOOMError();
    /* fallthrough */

 cleanup:
    if (dir)
        closedir(dir);
    virStorageVolDefFree(vol);
    virStoragePoolObjClearVols(pool);
    return -1;
}


/**
 * @conn connection to report errors against
 * @pool storage pool to start
 *
 * Stops a directory or FS based storage pool.
 *
 *  - If it is a FS based pool, unmounts the unlying source device on the pool
 *  - Releases all cached data about volumes
 */
#if WITH_STORAGE_FS
static int
virStorageBackendFileSystemStop(virConnectPtr conn ATTRIBUTE_UNUSED,
                                virStoragePoolObjPtr pool)
{
    if (pool->def->type != VIR_STORAGE_POOL_DIR &&
        virStorageBackendFileSystemUnmount(pool) < 0)
        return -1;

    return 0;
}
#endif /* WITH_STORAGE_FS */


/**
 * @conn connection to report errors against
 * @pool storage pool to build
 *
 * Build a directory or FS based storage pool.
 *
 *  - If it is a FS based pool, mounts the unlying source device on the pool
 *
 * Returns 0 on success, -1 on error
 */
static int
virStorageBackendFileSystemDelete(virConnectPtr conn ATTRIBUTE_UNUSED,
                                  virStoragePoolObjPtr pool,
                                  unsigned int flags ATTRIBUTE_UNUSED)
{
    /* XXX delete all vols first ? */

    if (rmdir(pool->def->target.path) < 0) {
        virReportSystemError(errno,
                             _("failed to remove pool '%s'"),
                             pool->def->target.path);
        return -1;
    }

    return 0;
}


/**
 * Set up a volume definition to be added to a pool's volume list, but
 * don't do any file creation or allocation. By separating the two processes,
 * we allow allocation progress reporting (by polling the volume's 'info'
 * function), and can drop the parent pool lock during the (slow) allocation.
 */
static int
virStorageBackendFileSystemVolCreate(virConnectPtr conn ATTRIBUTE_UNUSED,
                                     virStoragePoolObjPtr pool,
                                     virStorageVolDefPtr vol)
{

    vol->type = VIR_STORAGE_VOL_FILE;

    VIR_FREE(vol->target.path);
    if (virAsprintf(&vol->target.path, "%s/%s",
                    pool->def->target.path,
                    vol->name) == -1) {
        virReportOOMError();
        return -1;
    }

    VIR_FREE(vol->key);
    vol->key = strdup(vol->target.path);
    if (vol->key == NULL) {
        virReportOOMError();
        return -1;
    }

    return 0;
}

static int createFileDir(virConnectPtr conn ATTRIBUTE_UNUSED,
                         virStoragePoolObjPtr pool,
                         virStorageVolDefPtr vol,
                         virStorageVolDefPtr inputvol,
                         unsigned int flags ATTRIBUTE_UNUSED) {
    int err;

    if (inputvol) {
        virStorageReportError(VIR_ERR_INTERNAL_ERROR,
                              "%s",
                              _("cannot copy from volume to a directory volume"));
        return -1;
    }

    uid_t uid = (vol->target.perms.uid == -1)
        ? getuid() : vol->target.perms.uid;
    gid_t gid = (vol->target.perms.gid == -1)
        ? getgid() : vol->target.perms.gid;

    if ((err = virDirCreate(vol->target.path, vol->target.perms.mode,
                            uid, gid,
                            VIR_DIR_CREATE_FORCE_PERMS |
                            (pool->def->type == VIR_STORAGE_POOL_NETFS
                             ? VIR_DIR_CREATE_AS_UID : 0))) < 0) {
        virReportSystemError(-err, _("cannot create path '%s'"),
                             vol->target.path);
        return -1;
    }

    return 0;
}

static int
_virStorageBackendFileSystemVolBuild(virConnectPtr conn,
                                     virStoragePoolObjPtr pool,
                                     virStorageVolDefPtr vol,
                                     virStorageVolDefPtr inputvol)
{
    virStorageBackendBuildVolFrom create_func;
    int tool_type;

    if (inputvol) {
        if (vol->target.encryption != NULL) {
            virStorageReportError(VIR_ERR_NO_SUPPORT,
                                  "%s", _("storage pool does not support "
                                          "building encrypted volumes from "
                                          "other volumes"));
            return -1;
        }
        create_func = virStorageBackendGetBuildVolFromFunction(vol,
                                                               inputvol);
        if (!create_func)
            return -1;
    } else if (vol->target.format == VIR_STORAGE_FILE_RAW) {
        create_func = virStorageBackendCreateRaw;
    } else if (vol->target.format == VIR_STORAGE_FILE_DIR) {
        create_func = createFileDir;
    } else if ((tool_type = virStorageBackendFindFSImageTool(NULL)) != -1) {
        create_func = virStorageBackendFSImageToolTypeToFunc(tool_type);

        if (!create_func)
            return -1;
    } else {
        virStorageReportError(VIR_ERR_INTERNAL_ERROR,
                              "%s", _("creation of non-raw images "
                                      "is not supported without qemu-img"));
        return -1;
    }

    if (create_func(conn, pool, vol, inputvol, 0) < 0)
        return -1;
    return 0;
}

/**
 * Allocate a new file as a volume. This is either done directly
 * for raw/sparse files, or by calling qemu-img/qcow-create for
 * special kinds of files
 */
static int
virStorageBackendFileSystemVolBuild(virConnectPtr conn,
                                    virStoragePoolObjPtr pool,
                                    virStorageVolDefPtr vol) {
    return _virStorageBackendFileSystemVolBuild(conn, pool, vol, NULL);
}

/*
 * Create a storage vol using 'inputvol' as input
 */
static int
virStorageBackendFileSystemVolBuildFrom(virConnectPtr conn,
                                        virStoragePoolObjPtr pool,
                                        virStorageVolDefPtr vol,
                                        virStorageVolDefPtr inputvol,
                                        unsigned int flags ATTRIBUTE_UNUSED) {
    return _virStorageBackendFileSystemVolBuild(conn, pool, vol, inputvol);
}

/**
 * Remove a volume - just unlinks for now
 */
static int
virStorageBackendFileSystemVolDelete(virConnectPtr conn ATTRIBUTE_UNUSED,
                                     virStoragePoolObjPtr pool ATTRIBUTE_UNUSED,
                                     virStorageVolDefPtr vol,
                                     unsigned int flags ATTRIBUTE_UNUSED)
{
    if (unlink(vol->target.path) < 0) {
        /* Silently ignore failures where the vol has already gone away */
        if (errno != ENOENT) {
            virReportSystemError(errno,
                                 _("cannot unlink file '%s'"),
                                 vol->target.path);
            return -1;
        }
    }
    return 0;
}


/**
 * Update info about a volume's capacity/allocation
 */
static int
virStorageBackendFileSystemVolRefresh(virConnectPtr conn,
                                      virStoragePoolObjPtr pool ATTRIBUTE_UNUSED,
                                      virStorageVolDefPtr vol)
{
    int ret;

    /* Refresh allocation / permissions info in case its changed */
    ret = virStorageBackendUpdateVolInfo(vol, 0);
    if (ret < 0)
        return ret;

    /* Load any secrets if posible */
    if (vol->target.encryption &&
        vol->target.encryption->format == VIR_STORAGE_ENCRYPTION_FORMAT_QCOW &&
        vol->target.encryption->nsecrets == 0) {
        virSecretPtr sec;
        virStorageEncryptionSecretPtr encsec = NULL;

        sec = virSecretLookupByUsage(conn,
                                     VIR_SECRET_USAGE_TYPE_VOLUME,
                                     vol->target.path);
        if (sec) {
            if (VIR_ALLOC_N(vol->target.encryption->secrets, 1) < 0 ||
                VIR_ALLOC(encsec) < 0) {
                VIR_FREE(vol->target.encryption->secrets);
                virReportOOMError();
                virSecretFree(sec);
                return -1;
            }

            vol->target.encryption->nsecrets = 1;
            vol->target.encryption->secrets[0] = encsec;

            encsec->type = VIR_STORAGE_ENCRYPTION_SECRET_TYPE_PASSPHRASE;
            virSecretGetUUID(sec, encsec->uuid);
            virSecretFree(sec);
        }
    }

    return 0;
}

virStorageBackend virStorageBackendDirectory = {
    .type = VIR_STORAGE_POOL_DIR,

    .buildPool = virStorageBackendFileSystemBuild,
    .checkPool = virStorageBackendFileSystemCheck,
    .refreshPool = virStorageBackendFileSystemRefresh,
    .deletePool = virStorageBackendFileSystemDelete,
    .buildVol = virStorageBackendFileSystemVolBuild,
    .buildVolFrom = virStorageBackendFileSystemVolBuildFrom,
    .createVol = virStorageBackendFileSystemVolCreate,
    .refreshVol = virStorageBackendFileSystemVolRefresh,
    .deleteVol = virStorageBackendFileSystemVolDelete,
};

#if WITH_STORAGE_FS
virStorageBackend virStorageBackendFileSystem = {
    .type = VIR_STORAGE_POOL_FS,

    .buildPool = virStorageBackendFileSystemBuild,
    .checkPool = virStorageBackendFileSystemCheck,
    .startPool = virStorageBackendFileSystemStart,
    .refreshPool = virStorageBackendFileSystemRefresh,
    .stopPool = virStorageBackendFileSystemStop,
    .deletePool = virStorageBackendFileSystemDelete,
    .buildVol = virStorageBackendFileSystemVolBuild,
    .buildVolFrom = virStorageBackendFileSystemVolBuildFrom,
    .createVol = virStorageBackendFileSystemVolCreate,
    .refreshVol = virStorageBackendFileSystemVolRefresh,
    .deleteVol = virStorageBackendFileSystemVolDelete,
};
virStorageBackend virStorageBackendNetFileSystem = {
    .type = VIR_STORAGE_POOL_NETFS,

    .buildPool = virStorageBackendFileSystemBuild,
    .checkPool = virStorageBackendFileSystemCheck,
    .startPool = virStorageBackendFileSystemStart,
    .findPoolSources = virStorageBackendFileSystemNetFindPoolSources,
    .refreshPool = virStorageBackendFileSystemRefresh,
    .stopPool = virStorageBackendFileSystemStop,
    .deletePool = virStorageBackendFileSystemDelete,
    .buildVol = virStorageBackendFileSystemVolBuild,
    .buildVolFrom = virStorageBackendFileSystemVolBuildFrom,
    .createVol = virStorageBackendFileSystemVolCreate,
    .refreshVol = virStorageBackendFileSystemVolRefresh,
    .deleteVol = virStorageBackendFileSystemVolDelete,
};
#endif /* WITH_STORAGE_FS */
