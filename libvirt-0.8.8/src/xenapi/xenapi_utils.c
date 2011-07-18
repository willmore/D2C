/*
 * xenapi_utils.c: Xen API driver -- utils parts.
 * Copyright (C) 2009, 2010 Citrix Ltd.
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
 * Author: Sharadha Prabhakar <sharadha.prabhakar@citrix.com>
 */

#include <config.h>

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <libxml/uri.h>
#include <xen/api/xen_all.h>
#include "internal.h"
#include "domain_conf.h"
#include "virterror_internal.h"
#include "datatypes.h"
#include "util.h"
#include "uuid.h"
#include "memory.h"
#include "buf.h"
#include "logging.h"
#include "qparams.h"
#include "xenapi_driver_private.h"
#include "xenapi_utils.h"

void
xenSessionFree(xen_session *session)
{
    int i;
    if (session->error_description != NULL) {
        for (i = 0; i < session->error_description_count; i++)
            VIR_FREE(session->error_description[i]);
        VIR_FREE(session->error_description);
    }
    VIR_FREE(session->session_id);
    VIR_FREE(session);
}

char *
xenapiUtil_RequestPassword(virConnectAuthPtr auth, const char *username,
                        const char *hostname)
{
    unsigned int ncred;
    virConnectCredential cred;
    char *prompt;

    memset(&cred, 0, sizeof (virConnectCredential));

    if (virAsprintf(&prompt, "Enter %s password for %s", username,
                    hostname) < 0) {
        return NULL;
    }

    for (ncred = 0; ncred < auth->ncredtype; ncred++) {
        if (auth->credtype[ncred] != VIR_CRED_PASSPHRASE &&
            auth->credtype[ncred] != VIR_CRED_NOECHOPROMPT) {
            continue;
        }

        cred.type = auth->credtype[ncred];
        cred.prompt = prompt;
        cred.challenge = hostname;
        cred.defresult = NULL;
        cred.result = NULL;
        cred.resultlen = 0;

        if ((*(auth->cb))(&cred, 1, auth->cbdata) < 0) {
            VIR_FREE(cred.result);
        }

        break;
    }

    VIR_FREE(prompt);

    return cred.result;
}

int
xenapiUtil_ParseQuery(virConnectPtr conn, xmlURIPtr uri, int *noVerify)
{
    int result = 0;
    int i;
    struct qparam_set *queryParamSet = NULL;
    struct qparam *queryParam = NULL;

#ifdef HAVE_XMLURI_QUERY_RAW
    queryParamSet = qparam_query_parse(uri->query_raw);
#else
    queryParamSet = qparam_query_parse(uri->query);
#endif

    if (queryParamSet == NULL) {
        goto failure;
    }

    for (i = 0; i < queryParamSet->n; i++) {
        queryParam = &queryParamSet->p[i];
        if (STRCASEEQ(queryParam->name, "no_verify")) {
            if (noVerify == NULL) {
                continue;
            }
            if (virStrToLong_i(queryParam->value, NULL, 10, noVerify) < 0 ||
                (*noVerify != 0 && *noVerify != 1)) {
                xenapiSessionErrorHandler(conn, VIR_ERR_INVALID_ARG,
      _("Query parameter 'no_verify' has unexpected value (should be 0 or 1)"));
                goto failure;
            }
        }
    }

  cleanup:
    if (queryParamSet != NULL) {
        free_qparam_set(queryParamSet);
    }

    return result;

  failure:
    result = -1;

    goto cleanup;
}



enum xen_on_normal_exit
actionShutdownLibvirt2XenapiEnum(enum virDomainLifecycleAction action)
{
    enum xen_on_normal_exit num = XEN_ON_NORMAL_EXIT_RESTART;
    if (action == VIR_DOMAIN_LIFECYCLE_DESTROY)
        num = XEN_ON_NORMAL_EXIT_DESTROY;
    else if (action == VIR_DOMAIN_LIFECYCLE_RESTART)
        num = XEN_ON_NORMAL_EXIT_RESTART;
    return num;
}


enum xen_on_crash_behaviour
actionCrashLibvirt2XenapiEnum(enum virDomainLifecycleCrashAction action)
{
    enum xen_on_crash_behaviour num = XEN_ON_CRASH_BEHAVIOUR_RESTART;
    if (action == VIR_DOMAIN_LIFECYCLE_CRASH_DESTROY)
        num = XEN_ON_CRASH_BEHAVIOUR_DESTROY;
    else if (action == VIR_DOMAIN_LIFECYCLE_CRASH_RESTART)
        num = XEN_ON_CRASH_BEHAVIOUR_RESTART;
    else if (action == VIR_DOMAIN_LIFECYCLE_CRASH_PRESERVE)
        num = XEN_ON_CRASH_BEHAVIOUR_PRESERVE;
    else if (action == VIR_DOMAIN_LIFECYCLE_CRASH_RESTART_RENAME)
        num = XEN_ON_CRASH_BEHAVIOUR_RENAME_RESTART;
    else if (action == VIR_DOMAIN_LIFECYCLE_CRASH_COREDUMP_DESTROY)
        num = XEN_ON_CRASH_BEHAVIOUR_COREDUMP_AND_DESTROY;
    else if (action == VIR_DOMAIN_LIFECYCLE_CRASH_COREDUMP_RESTART)
        num = XEN_ON_CRASH_BEHAVIOUR_COREDUMP_AND_RESTART;
    return num;
}

/* generate XenAPI boot order format from libvirt format */
char *
createXenAPIBootOrderString(int nboot, int *bootDevs)
{
    virBuffer ret = VIR_BUFFER_INITIALIZER;
    char *val = NULL;
    int i;
    for (i = 0; i < nboot; i++) {
        if (bootDevs[i] == VIR_DOMAIN_BOOT_FLOPPY)
            val = (char *)"a";
        else if (bootDevs[i] == VIR_DOMAIN_BOOT_DISK)
            val = (char *)"c";
        else if (bootDevs[i] == VIR_DOMAIN_BOOT_CDROM)
            val = (char *)"d";
        else if (bootDevs[i] == VIR_DOMAIN_BOOT_NET)
            val = (char *)"n";
        if (val)
            virBufferEscapeString(&ret,"%s",val);
    }
    return virBufferContentAndReset(&ret);
}

/* convert boot order string to libvirt boot order enum */
enum virDomainBootOrder
map2LibvirtBootOrder(char c) {
    switch (c) {
    case 'a':
        return VIR_DOMAIN_BOOT_FLOPPY;
    case 'c':
        return VIR_DOMAIN_BOOT_DISK;
    case 'd':
        return VIR_DOMAIN_BOOT_CDROM;
    case 'n':
        return VIR_DOMAIN_BOOT_NET;
    default:
        return -1;
    }
}

enum virDomainLifecycleAction
xenapiNormalExitEnum2virDomainLifecycle(enum xen_on_normal_exit action)
{
    enum virDomainLifecycleAction num = VIR_DOMAIN_LIFECYCLE_RESTART;
    if (action == XEN_ON_NORMAL_EXIT_DESTROY)
        num = VIR_DOMAIN_LIFECYCLE_DESTROY;
    else if (action == XEN_ON_NORMAL_EXIT_RESTART)
        num = VIR_DOMAIN_LIFECYCLE_RESTART;
    return num;
}


enum virDomainLifecycleCrashAction
xenapiCrashExitEnum2virDomainLifecycle(enum xen_on_crash_behaviour action)
{
    enum virDomainLifecycleCrashAction num = VIR_DOMAIN_LIFECYCLE_CRASH_RESTART;
    if (action == XEN_ON_CRASH_BEHAVIOUR_DESTROY)
        num = VIR_DOMAIN_LIFECYCLE_CRASH_DESTROY;
    else if (action == XEN_ON_CRASH_BEHAVIOUR_RESTART)
        num = VIR_DOMAIN_LIFECYCLE_CRASH_RESTART;
    else if (action == XEN_ON_CRASH_BEHAVIOUR_PRESERVE)
        num = VIR_DOMAIN_LIFECYCLE_CRASH_PRESERVE;
    else if (action == XEN_ON_CRASH_BEHAVIOUR_RENAME_RESTART)
        num = VIR_DOMAIN_LIFECYCLE_CRASH_RESTART_RENAME;
    else if (action == XEN_ON_CRASH_BEHAVIOUR_COREDUMP_AND_DESTROY)
        num = VIR_DOMAIN_LIFECYCLE_CRASH_COREDUMP_DESTROY;
    else if (action == XEN_ON_CRASH_BEHAVIOUR_COREDUMP_AND_RESTART)
        num = VIR_DOMAIN_LIFECYCLE_CRASH_COREDUMP_RESTART;
    return num;
}



/* returns 'file' or 'block' for the storage type */
int
getStorageVolumeType(char *type)
{
    if (STREQ(type,"lvmoiscsi") ||
        STREQ(type,"lvmohba") ||
        STREQ(type,"lvm") ||
        STREQ(type,"file") ||
        STREQ(type,"iso") ||
        STREQ(type,"ext") ||
        STREQ(type,"nfs"))
        return (int)VIR_STORAGE_VOL_FILE;
    else if (STREQ(type,"iscsi") ||
             STREQ(type,"equal") ||
             STREQ(type,"hba") ||
             STREQ(type,"cslg") ||
             STREQ(type,"udev") ||
             STREQ(type,"netapp"))
        return (int)VIR_STORAGE_VOL_BLOCK;
    return -1;
}

/* returns error description if any received from the server */
char *
returnErrorFromSession(xen_session *session)
{
    int i;
    virBuffer buf = VIR_BUFFER_INITIALIZER;
    for (i = 0; i < session->error_description_count - 1; i++) {
        if (!i)
            virBufferEscapeString(&buf, "%s", session->error_description[i]);
        else
            virBufferEscapeString(&buf, " : %s", session->error_description[i]);
    }
    return virBufferContentAndReset(&buf);
}

/* converts bitmap to string of the form '1,2...' */
char *
mapDomainPinVcpu(unsigned char *cpumap, int maplen)
{
    virBuffer buf = VIR_BUFFER_INITIALIZER;
    size_t len;
    char *ret = NULL;
    int i, j;
    for (i = 0; i < maplen; i++) {
        for (j = 0; j < 8; j++) {
            if (cpumap[i] & (1 << j)) {
                virBufferVSprintf(&buf, "%d,", (8*i)+j);
            }
        }
    }
    if (virBufferError(&buf)) {
        virReportOOMError();
        virBufferFreeAndReset(&buf);
        return NULL;
    }
    ret = virBufferContentAndReset(&buf);
    len = strlen(ret);
    if (len > 0 && ret[len - 1] == ',')
        ret[len - 1] = 0;
    return ret;
}

/* obtains the CPU bitmap from the string passed */
void
getCpuBitMapfromString(char *mask, unsigned char *cpumap, int maplen)
{
    int pos;
    int max_bits = maplen * 8;
    char *num = NULL, *bp = NULL;
    bzero(cpumap, maplen);
    num = strtok_r(mask, ",", &bp);
    while (num != NULL) {
        if (virStrToLong_i(num, NULL, 10, &pos) < 0)
            return;
        if (pos < 0 || pos > max_bits - 1)
            VIR_WARN ("number in str %d exceeds cpumap's max bits %d", pos, max_bits);
        else
            (cpumap)[pos / 8] |= (1 << (pos % 8));
        num = strtok_r(NULL, ",", &bp);
    }
}


/* mapping XenServer power state to Libvirt power state */
virDomainState
mapPowerState(enum xen_vm_power_state state)
{
    virDomainState virState;
    switch (state) {
    case XEN_VM_POWER_STATE_HALTED:
    case XEN_VM_POWER_STATE_SUSPENDED:
        virState = VIR_DOMAIN_SHUTOFF;
        break;
    case XEN_VM_POWER_STATE_PAUSED:
        virState = VIR_DOMAIN_PAUSED;
        break;
    case XEN_VM_POWER_STATE_RUNNING:
        virState = VIR_DOMAIN_RUNNING;
        break;
    case XEN_VM_POWER_STATE_UNDEFINED:
    default:
        /* Includes XEN_VM_POWER_STATE_UNKNOWN from libxenserver
         * 5.5.0, which is gone in 5.6.0.  */
        virState = VIR_DOMAIN_NOSTATE;
        break;
    }
    return virState;
}

/* allocate a flexible array and fill values(key,val) */
int
allocStringMap (xen_string_string_map **strings, char *key, char *val)
{
    int sz = ((*strings) == NULL) ? 0 : (*strings)->size;
    sz++;
    if (VIR_REALLOC_N(*strings, sizeof(xen_string_string_map) +
                                sizeof(xen_string_string_map_contents) * sz) < 0) {
        virReportOOMError();
        return -1;
    }
    (*strings)->size = sz;
    if (!((*strings)->contents[sz-1].key = strdup(key))) goto error;
    if (!((*strings)->contents[sz-1].val = strdup(val))) goto error;
    return 0;
  error:
    xen_string_string_map_free(*strings);
    virReportOOMError();
    return -1;
}

/* Error handling function returns error messages from the server if any */
void
xenapiSessionErrorHandle(virConnectPtr conn, virErrorNumber errNum,
                         const char *buf, const char *filename, const char *func,
                         size_t lineno)
{
    struct _xenapiPrivate *priv = conn->privateData;

    if (buf == NULL && priv != NULL && priv->session != NULL) {
        char *ret = returnErrorFromSession(priv->session);
        virReportErrorHelper(conn, VIR_FROM_XENAPI, errNum, filename, func, lineno, _("%s"), ret);
        xen_session_clear_error(priv->session);
        VIR_FREE(ret);
    } else {
        virReportErrorHelper(conn, VIR_FROM_XENAPI, errNum, filename, func, lineno, _("%s"), buf);
    }
}

/* creates network intereface for VM */
static int
createVifNetwork (virConnectPtr conn, xen_vm vm, int device,
                  char *bridge, char *mac)
{
    xen_session *session = ((struct _xenapiPrivate *)(conn->privateData))->session;
    xen_vm xvm = NULL;
    char *uuid = NULL;
    xen_vm_get_uuid(session, &uuid, vm);
    if (uuid) {
        if (!xen_vm_get_by_uuid(session, &xvm, uuid))
            return -1;
        VIR_FREE(uuid);
    }
    xen_vm_record_opt *vm_opt = xen_vm_record_opt_alloc();
    vm_opt->is_record = 0;
    vm_opt->u.handle = xvm;
    xen_network_set *net_set = NULL;
    xen_network_record *net_rec = NULL;
    int cnt = 0;
    if (xen_network_get_all(session, &net_set)) {
        for(cnt = 0; cnt < net_set->size; cnt++) {
            if (xen_network_get_record(session, &net_rec, net_set->contents[cnt])) {
                if (STREQ(net_rec->bridge, bridge)) {
                    break;
                } else {
                    xen_network_record_free(net_rec);
                }
            }
        }
    }
    if (cnt < net_set->size && net_rec) {
        xen_network network = NULL;
        xen_network_get_by_uuid(session, &network, net_rec->uuid);
        xen_network_record_opt *network_opt = xen_network_record_opt_alloc();
        network_opt->is_record = 0;
        network_opt->u.handle = network;
        xen_vif_record *vif_record = xen_vif_record_alloc();
        vif_record->mac = mac;
        vif_record->vm = vm_opt;
        vif_record->network = network_opt;
        xen_vif vif = NULL;

        vif_record->other_config = xen_string_string_map_alloc(0);
        vif_record->runtime_properties = xen_string_string_map_alloc(0);
        vif_record->qos_algorithm_params = xen_string_string_map_alloc(0);
        if (virAsprintf(&vif_record->device, "%d", device) < 0)
            return -1;
        xen_vif_create(session, &vif, vif_record);
        if (!vif) {
            xen_vif_free(vif);
            xen_vif_record_free(vif_record);
            xen_network_record_free(net_rec);
            xen_network_set_free(net_set);
            return 0;
        }
        xen_vif_record_free(vif_record);
        xen_network_record_free(net_rec);
    }
    if (net_set != NULL) xen_network_set_free(net_set);
    return -1;
}

/* Create a VM record from the XML description */
int
createVMRecordFromXml (virConnectPtr conn, virDomainDefPtr def,
                       xen_vm_record **record, xen_vm *vm)
{
    char uuidStr[VIR_UUID_STRING_BUFLEN];
    *record = xen_vm_record_alloc();
    if (!((*record)->name_label = strdup(def->name)))
        goto error_cleanup;
    if (def->uuid) {
        virUUIDFormat(def->uuid, uuidStr);
        if (!((*record)->uuid = strdup(uuidStr)))
            goto error_cleanup;
    }
    if (STREQ(def->os.type, "hvm")) {
        char *boot_order = NULL;
        if (!((*record)->hvm_boot_policy = strdup("BIOS order")))
            goto error_cleanup;
        if (def->os.nBootDevs != 0)
            boot_order = createXenAPIBootOrderString(def->os.nBootDevs, &def->os.bootDevs[0]);
        if (boot_order != NULL) {
            xen_string_string_map *hvm_boot_params = NULL;
            allocStringMap(&hvm_boot_params, (char *)"order", boot_order);
            (*record)->hvm_boot_params = hvm_boot_params;
            VIR_FREE(boot_order);
        }
    } else if (STREQ(def->os.type, "xen")) {
        if (!((*record)->pv_bootloader = strdup("pygrub")))
            goto error_cleanup;
        if (def->os.kernel) {
            if (!((*record)->pv_kernel = strdup(def->os.kernel)))
                goto error_cleanup;
        }
        if (def->os.initrd) {
            if (!((*record)->pv_ramdisk = strdup(def->os.initrd)))
                goto error_cleanup;
        }
        if (def->os.cmdline) {
            if (!((*record)->pv_args = strdup(def->os.cmdline)))
                goto error_cleanup;
        }
        (*record)->hvm_boot_params = xen_string_string_map_alloc(0);
    }
    if (def->os.bootloaderArgs)
        if (!((*record)->pv_bootloader_args = strdup(def->os.bootloaderArgs)))
            goto error_cleanup;

    if (def->mem.cur_balloon)
        (*record)->memory_static_max = (int64_t) (def->mem.cur_balloon * 1024);
    if (def->mem.max_balloon)
        (*record)->memory_dynamic_max = (int64_t) (def->mem.max_balloon * 1024);
    else
        (*record)->memory_dynamic_max = (*record)->memory_static_max;

    if (def->maxvcpus) {
        (*record)->vcpus_max = (int64_t) def->maxvcpus;
        (*record)->vcpus_at_startup = (int64_t) def->vcpus;
    }
    if (def->onPoweroff)
        (*record)->actions_after_shutdown = actionShutdownLibvirt2XenapiEnum(def->onPoweroff);
    if (def->onReboot)
        (*record)->actions_after_reboot = actionShutdownLibvirt2XenapiEnum(def->onReboot);
    if (def->onCrash)
        (*record)->actions_after_crash = actionCrashLibvirt2XenapiEnum(def->onCrash);

    xen_string_string_map *strings = NULL;
    if (def->features) {
        if (def->features & (1 << VIR_DOMAIN_FEATURE_ACPI))
            allocStringMap(&strings, (char *)"acpi", (char *)"true");
        if (def->features & (1 << VIR_DOMAIN_FEATURE_APIC))
            allocStringMap(&strings, (char *)"apic", (char *)"true");
        if (def->features & (1 << VIR_DOMAIN_FEATURE_PAE))
            allocStringMap(&strings, (char *)"pae", (char *)"true");
        if (def->features & (1 << VIR_DOMAIN_FEATURE_HAP))
            allocStringMap(&strings, (char *)"hap", (char *)"true");
    }
    if (strings != NULL)
        (*record)->platform = strings;

    (*record)->vcpus_params = xen_string_string_map_alloc(0);
    (*record)->other_config = xen_string_string_map_alloc(0);
    (*record)->last_boot_cpu_flags = xen_string_string_map_alloc(0);
    (*record)->xenstore_data = xen_string_string_map_alloc(0);
    (*record)->hvm_shadow_multiplier = 1.000;
    if (!xen_vm_create(((struct _xenapiPrivate *)(conn->privateData))->session,
                        vm, *record)) {
        xenapiSessionErrorHandler(conn, VIR_ERR_INTERNAL_ERROR, NULL);
        return -1;
    }

    int device_number = 0;
    char *bridge = NULL, *mac = NULL;
    int i;
    for (i = 0; i < def->nnets; i++) {
        if (def->nets[i]->type == VIR_DOMAIN_NET_TYPE_BRIDGE) {
            if (def->nets[i]->data.bridge.brname)
                if (!(bridge = strdup(def->nets[i]->data.bridge.brname)))
                    goto error_cleanup;
            if (def->nets[i]->mac) {
                char macStr[VIR_MAC_STRING_BUFLEN];
                virFormatMacAddr(def->nets[i]->mac, macStr);
                if (!(mac = strdup(macStr))) {
                    VIR_FREE(bridge);
                    goto error_cleanup;
                }
            }
            if (mac != NULL && bridge != NULL) {
                if (createVifNetwork(conn, *vm, device_number, bridge,
                                     mac) < 0) {
                    VIR_FREE(bridge);
                    goto error_cleanup;
                }
                VIR_FREE(bridge);
                device_number++;
            }
            VIR_FREE(bridge);
        }
    }
    return 0;

  error_cleanup:
    virReportOOMError();
    xen_vm_record_free(*record);
    return -1;
}
