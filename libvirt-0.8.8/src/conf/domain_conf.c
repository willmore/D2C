/*
 * domain_conf.c: domain XML processing
 *
 * Copyright (C) 2006-2011 Red Hat, Inc.
 * Copyright (C) 2006-2008 Daniel P. Berrange
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
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/time.h>

#include "virterror_internal.h"
#include "datatypes.h"
#include "domain_conf.h"
#include "memory.h"
#include "verify.h"
#include "xml.h"
#include "uuid.h"
#include "util.h"
#include "buf.h"
#include "c-ctype.h"
#include "logging.h"
#include "network.h"
#include "nwfilter_conf.h"
#include "ignore-value.h"
#include "storage_file.h"
#include "files.h"
#include "bitmap.h"

#define VIR_FROM_THIS VIR_FROM_DOMAIN

VIR_ENUM_IMPL(virDomainVirt, VIR_DOMAIN_VIRT_LAST,
              "qemu",
              "kqemu",
              "kvm",
              "xen",
              "lxc",
              "uml",
              "openvz",
              "vserver",
              "ldom",
              "test",
              "vmware",
              "hyperv",
              "vbox",
              "one",
              "phyp")

VIR_ENUM_IMPL(virDomainBoot, VIR_DOMAIN_BOOT_LAST,
              "fd",
              "cdrom",
              "hd",
              "network")

VIR_ENUM_IMPL(virDomainFeature, VIR_DOMAIN_FEATURE_LAST,
              "acpi",
              "apic",
              "pae",
              "hap")

VIR_ENUM_IMPL(virDomainLifecycle, VIR_DOMAIN_LIFECYCLE_LAST,
              "destroy",
              "restart",
              "rename-restart",
              "preserve")

VIR_ENUM_IMPL(virDomainLifecycleCrash, VIR_DOMAIN_LIFECYCLE_CRASH_LAST,
              "destroy",
              "restart",
              "rename-restart",
              "preserve",
              "coredump-destroy",
              "coredump-restart")

VIR_ENUM_IMPL(virDomainDevice, VIR_DOMAIN_DEVICE_LAST,
              "disk",
              "filesystem",
              "interface",
              "input",
              "sound",
              "video",
              "hostdev",
              "watchdog",
              "controller",
              "graphics")

VIR_ENUM_IMPL(virDomainDeviceAddress, VIR_DOMAIN_DEVICE_ADDRESS_TYPE_LAST,
              "none",
              "pci",
              "drive",
              "virtio-serial",
              "ccid")

VIR_ENUM_IMPL(virDomainDisk, VIR_DOMAIN_DISK_TYPE_LAST,
              "block",
              "file",
              "dir",
              "network")

VIR_ENUM_IMPL(virDomainDiskDevice, VIR_DOMAIN_DISK_DEVICE_LAST,
              "disk",
              "cdrom",
              "floppy")

VIR_ENUM_IMPL(virDomainDiskBus, VIR_DOMAIN_DISK_BUS_LAST,
              "ide",
              "fdc",
              "scsi",
              "virtio",
              "xen",
              "usb",
              "uml",
              "sata")

VIR_ENUM_IMPL(virDomainDiskCache, VIR_DOMAIN_DISK_CACHE_LAST,
              "default",
              "none",
              "writethrough",
              "writeback")

VIR_ENUM_IMPL(virDomainDiskErrorPolicy, VIR_DOMAIN_DISK_ERROR_POLICY_LAST,
              "default",
              "stop",
              "ignore",
              "enospace")

VIR_ENUM_IMPL(virDomainDiskProtocol, VIR_DOMAIN_DISK_PROTOCOL_LAST,
              "nbd",
              "rbd",
              "sheepdog")

VIR_ENUM_IMPL(virDomainDiskIo, VIR_DOMAIN_DISK_IO_LAST,
              "default",
              "native",
              "threads")

VIR_ENUM_IMPL(virDomainController, VIR_DOMAIN_CONTROLLER_TYPE_LAST,
              "ide",
              "fdc",
              "scsi",
              "sata",
              "virtio-serial",
              "ccid")

VIR_ENUM_IMPL(virDomainControllerModel, VIR_DOMAIN_CONTROLLER_MODEL_LAST,
              "auto",
              "buslogic",
              "lsilogic",
              "lsisas1068",
              "vmpvscsi")

VIR_ENUM_IMPL(virDomainFS, VIR_DOMAIN_FS_TYPE_LAST,
              "mount",
              "block",
              "file",
              "template")

VIR_ENUM_IMPL(virDomainFSAccessMode, VIR_DOMAIN_FS_ACCESSMODE_LAST,
              "passthrough",
              "mapped",
              "squash")


VIR_ENUM_IMPL(virDomainNet, VIR_DOMAIN_NET_TYPE_LAST,
              "user",
              "ethernet",
              "server",
              "client",
              "mcast",
              "network",
              "bridge",
              "internal",
              "direct")

VIR_ENUM_IMPL(virDomainNetBackend, VIR_DOMAIN_NET_BACKEND_TYPE_LAST,
              "default",
              "qemu",
              "vhost")

VIR_ENUM_IMPL(virDomainChrChannelTarget,
              VIR_DOMAIN_CHR_CHANNEL_TARGET_TYPE_LAST,
              "guestfwd",
              "virtio")

VIR_ENUM_IMPL(virDomainChrConsoleTarget,
              VIR_DOMAIN_CHR_CONSOLE_TARGET_TYPE_LAST,
              "serial",
              "xen",
              "uml",
              "virtio")

VIR_ENUM_IMPL(virDomainChrDevice, VIR_DOMAIN_CHR_DEVICE_TYPE_LAST,
              "parallel",
              "serial",
              "console",
              "channel")

VIR_ENUM_IMPL(virDomainChr, VIR_DOMAIN_CHR_TYPE_LAST,
              "null",
              "vc",
              "pty",
              "dev",
              "file",
              "pipe",
              "stdio",
              "udp",
              "tcp",
              "unix",
              "spicevmc")

VIR_ENUM_IMPL(virDomainChrTcpProtocol, VIR_DOMAIN_CHR_TCP_PROTOCOL_LAST,
              "raw",
              "telnet",
              "telnets",
              "tls")

VIR_ENUM_IMPL(virDomainChrSpicevmc, VIR_DOMAIN_CHR_SPICEVMC_LAST,
              "vdagent",
              "smartcard")

VIR_ENUM_IMPL(virDomainSmartcard, VIR_DOMAIN_SMARTCARD_TYPE_LAST,
              "host",
              "host-certificates",
              "passthrough")

VIR_ENUM_IMPL(virDomainSoundModel, VIR_DOMAIN_SOUND_MODEL_LAST,
              "sb16",
              "es1370",
              "pcspk",
              "ac97",
              "ich6")

VIR_ENUM_IMPL(virDomainMemballoonModel, VIR_DOMAIN_MEMBALLOON_MODEL_LAST,
              "virtio",
              "xen",
              "none")

VIR_ENUM_IMPL(virDomainSysinfo, VIR_DOMAIN_SYSINFO_LAST,
              "smbios")

VIR_ENUM_IMPL(virDomainSmbiosMode, VIR_DOMAIN_SMBIOS_LAST,
              "none",
              "emulate",
              "host",
              "sysinfo")

VIR_ENUM_IMPL(virDomainWatchdogModel, VIR_DOMAIN_WATCHDOG_MODEL_LAST,
              "i6300esb",
              "ib700")

VIR_ENUM_IMPL(virDomainWatchdogAction, VIR_DOMAIN_WATCHDOG_ACTION_LAST,
              "reset",
              "shutdown",
              "poweroff",
              "pause",
              "dump",
              "none")

VIR_ENUM_IMPL(virDomainVideo, VIR_DOMAIN_VIDEO_TYPE_LAST,
              "vga",
              "cirrus",
              "vmvga",
              "xen",
              "vbox",
              "qxl")

VIR_ENUM_IMPL(virDomainInput, VIR_DOMAIN_INPUT_TYPE_LAST,
              "mouse",
              "tablet")

VIR_ENUM_IMPL(virDomainInputBus, VIR_DOMAIN_INPUT_BUS_LAST,
              "ps2",
              "usb",
              "xen")

VIR_ENUM_IMPL(virDomainGraphics, VIR_DOMAIN_GRAPHICS_TYPE_LAST,
              "sdl",
              "vnc",
              "rdp",
              "desktop",
              "spice")

VIR_ENUM_IMPL(virDomainGraphicsSpiceChannelName,
              VIR_DOMAIN_GRAPHICS_SPICE_CHANNEL_LAST,
              "main",
              "display",
              "inputs",
              "cursor",
              "playback",
              "record",
              "smartcard");

VIR_ENUM_IMPL(virDomainGraphicsSpiceChannelMode,
              VIR_DOMAIN_GRAPHICS_SPICE_CHANNEL_MODE_LAST,
              "any",
              "secure",
              "insecure");

VIR_ENUM_IMPL(virDomainHostdevMode, VIR_DOMAIN_HOSTDEV_MODE_LAST,
              "subsystem",
              "capabilities")

VIR_ENUM_IMPL(virDomainHostdevSubsys, VIR_DOMAIN_HOSTDEV_SUBSYS_TYPE_LAST,
              "usb",
              "pci")

VIR_ENUM_IMPL(virDomainState, VIR_DOMAIN_CRASHED+1,
              "nostate",
              "running",
              "blocked",
              "paused",
              "shutdown",
              "shutoff",
              "crashed")

VIR_ENUM_IMPL(virDomainSeclabel, VIR_DOMAIN_SECLABEL_LAST,
              "dynamic",
              "static")

VIR_ENUM_IMPL(virDomainNetdevMacvtap, VIR_DOMAIN_NETDEV_MACVTAP_MODE_LAST,
              "vepa",
              "private",
              "bridge")

VIR_ENUM_IMPL(virVirtualPort, VIR_VIRTUALPORT_TYPE_LAST,
              "none",
              "802.1Qbg",
              "802.1Qbh")

VIR_ENUM_IMPL(virDomainClockOffset, VIR_DOMAIN_CLOCK_OFFSET_LAST,
              "utc",
              "localtime",
              "variable",
              "timezone");

VIR_ENUM_IMPL(virDomainTimerName, VIR_DOMAIN_TIMER_NAME_LAST,
              "platform",
              "pit",
              "rtc",
              "hpet",
              "tsc");

VIR_ENUM_IMPL(virDomainTimerTrack, VIR_DOMAIN_TIMER_TRACK_LAST,
              "boot",
              "guest",
              "wall");

VIR_ENUM_IMPL(virDomainTimerTickpolicy, VIR_DOMAIN_TIMER_TICKPOLICY_LAST,
              "delay",
              "catchup",
              "merge",
              "discard");

VIR_ENUM_IMPL(virDomainTimerMode, VIR_DOMAIN_TIMER_MODE_LAST,
              "auto",
              "native",
              "emulate",
              "paravirt",
              "smpsafe");

#define virDomainReportError(code, ...)                              \
    virReportErrorHelper(NULL, VIR_FROM_DOMAIN, code, __FILE__,      \
                         __FUNCTION__, __LINE__, __VA_ARGS__)

#define VIR_DOMAIN_XML_WRITE_FLAGS  VIR_DOMAIN_XML_SECURE
#define VIR_DOMAIN_XML_READ_FLAGS   VIR_DOMAIN_XML_INACTIVE

int virDomainObjListInit(virDomainObjListPtr doms)
{
    doms->objs = virHashCreate(50);
    if (!doms->objs) {
        virReportOOMError();
        return -1;
    }
    return 0;
}


static void virDomainObjListDeallocator(void *payload, const char *name ATTRIBUTE_UNUSED)
{
    virDomainObjPtr obj = payload;
    virDomainObjLock(obj);
    if (virDomainObjUnref(obj) > 0)
        virDomainObjUnlock(obj);
}

void virDomainObjListDeinit(virDomainObjListPtr doms)
{
    if (doms->objs)
        virHashFree(doms->objs, virDomainObjListDeallocator);
}


static int virDomainObjListSearchID(const void *payload,
                                    const char *name ATTRIBUTE_UNUSED,
                                    const void *data)
{
    virDomainObjPtr obj = (virDomainObjPtr)payload;
    const int *id = data;
    int want = 0;

    virDomainObjLock(obj);
    if (virDomainObjIsActive(obj) &&
        obj->def->id == *id)
        want = 1;
    virDomainObjUnlock(obj);
    return want;
}

virDomainObjPtr virDomainFindByID(const virDomainObjListPtr doms,
                                  int id)
{
    virDomainObjPtr obj;
    obj = virHashSearch(doms->objs, virDomainObjListSearchID, &id);
    if (obj)
        virDomainObjLock(obj);
    return obj;
}


virDomainObjPtr virDomainFindByUUID(const virDomainObjListPtr doms,
                                    const unsigned char *uuid)
{
    char uuidstr[VIR_UUID_STRING_BUFLEN];
    virDomainObjPtr obj;

    virUUIDFormat(uuid, uuidstr);

    obj = virHashLookup(doms->objs, uuidstr);
    if (obj)
        virDomainObjLock(obj);
    return obj;
}

static int virDomainObjListSearchName(const void *payload,
                                      const char *name ATTRIBUTE_UNUSED,
                                      const void *data)
{
    virDomainObjPtr obj = (virDomainObjPtr)payload;
    int want = 0;

    virDomainObjLock(obj);
    if (STREQ(obj->def->name, (const char *)data))
        want = 1;
    virDomainObjUnlock(obj);
    return want;
}

virDomainObjPtr virDomainFindByName(const virDomainObjListPtr doms,
                                    const char *name)
{
    virDomainObjPtr obj;
    obj = virHashSearch(doms->objs, virDomainObjListSearchName, name);
    if (obj)
        virDomainObjLock(obj);
    return obj;
}

static void
virDomainGraphicsAuthDefClear(virDomainGraphicsAuthDefPtr def)
{
    if (!def)
        return;

    VIR_FREE(def->passwd);

    /* Don't free def */
}

void virDomainGraphicsDefFree(virDomainGraphicsDefPtr def)
{
    if (!def)
        return;

    switch (def->type) {
    case VIR_DOMAIN_GRAPHICS_TYPE_VNC:
        VIR_FREE(def->data.vnc.listenAddr);
        VIR_FREE(def->data.vnc.socket);
        VIR_FREE(def->data.vnc.keymap);
        virDomainGraphicsAuthDefClear(&def->data.vnc.auth);
        break;

    case VIR_DOMAIN_GRAPHICS_TYPE_SDL:
        VIR_FREE(def->data.sdl.display);
        VIR_FREE(def->data.sdl.xauth);
        break;

    case VIR_DOMAIN_GRAPHICS_TYPE_RDP:
        VIR_FREE(def->data.rdp.listenAddr);
        break;

    case VIR_DOMAIN_GRAPHICS_TYPE_DESKTOP:
        VIR_FREE(def->data.desktop.display);
        break;

    case VIR_DOMAIN_GRAPHICS_TYPE_SPICE:
        VIR_FREE(def->data.spice.listenAddr);
        VIR_FREE(def->data.spice.keymap);
        virDomainGraphicsAuthDefClear(&def->data.spice.auth);
        break;
    }

    VIR_FREE(def);
}

void virDomainInputDefFree(virDomainInputDefPtr def)
{
    if (!def)
        return;

    virDomainDeviceInfoClear(&def->info);
    VIR_FREE(def);
}

void virDomainDiskDefFree(virDomainDiskDefPtr def)
{
    unsigned int i;

    if (!def)
        return;

    VIR_FREE(def->serial);
    VIR_FREE(def->src);
    VIR_FREE(def->dst);
    VIR_FREE(def->driverName);
    VIR_FREE(def->driverType);
    virStorageEncryptionFree(def->encryption);
    virDomainDeviceInfoClear(&def->info);

    for (i = 0 ; i < def->nhosts ; i++)
        virDomainDiskHostDefFree(&def->hosts[i]);
    VIR_FREE(def->hosts);

    VIR_FREE(def);
}

void virDomainDiskHostDefFree(virDomainDiskHostDefPtr def)
{
    if (!def)
        return;

    VIR_FREE(def->name);
    VIR_FREE(def->port);
}

void virDomainControllerDefFree(virDomainControllerDefPtr def)
{
    if (!def)
        return;

    virDomainDeviceInfoClear(&def->info);

    VIR_FREE(def);
}

void virDomainFSDefFree(virDomainFSDefPtr def)
{
    if (!def)
        return;

    VIR_FREE(def->src);
    VIR_FREE(def->dst);
    virDomainDeviceInfoClear(&def->info);

    VIR_FREE(def);
}

void virDomainNetDefFree(virDomainNetDefPtr def)
{
    if (!def)
        return;

    VIR_FREE(def->model);

    switch (def->type) {
    case VIR_DOMAIN_NET_TYPE_ETHERNET:
        VIR_FREE(def->data.ethernet.dev);
        VIR_FREE(def->data.ethernet.script);
        VIR_FREE(def->data.ethernet.ipaddr);
        break;

    case VIR_DOMAIN_NET_TYPE_SERVER:
    case VIR_DOMAIN_NET_TYPE_CLIENT:
    case VIR_DOMAIN_NET_TYPE_MCAST:
        VIR_FREE(def->data.socket.address);
        break;

    case VIR_DOMAIN_NET_TYPE_NETWORK:
        VIR_FREE(def->data.network.name);
        break;

    case VIR_DOMAIN_NET_TYPE_BRIDGE:
        VIR_FREE(def->data.bridge.brname);
        VIR_FREE(def->data.bridge.script);
        VIR_FREE(def->data.bridge.ipaddr);
        break;

    case VIR_DOMAIN_NET_TYPE_INTERNAL:
        VIR_FREE(def->data.internal.name);
        break;

    case VIR_DOMAIN_NET_TYPE_DIRECT:
        VIR_FREE(def->data.direct.linkdev);
        break;

    case VIR_DOMAIN_NET_TYPE_USER:
    case VIR_DOMAIN_NET_TYPE_LAST:
        break;
    }

    VIR_FREE(def->ifname);

    virDomainDeviceInfoClear(&def->info);

    VIR_FREE(def->filter);
    virNWFilterHashTableFree(def->filterparams);

    VIR_FREE(def);
}

static void ATTRIBUTE_NONNULL(1)
virDomainChrSourceDefClear(virDomainChrSourceDefPtr def)
{
    switch (def->type) {
    case VIR_DOMAIN_CHR_TYPE_PTY:
    case VIR_DOMAIN_CHR_TYPE_DEV:
    case VIR_DOMAIN_CHR_TYPE_FILE:
    case VIR_DOMAIN_CHR_TYPE_PIPE:
        VIR_FREE(def->data.file.path);
        break;

    case VIR_DOMAIN_CHR_TYPE_UDP:
        VIR_FREE(def->data.udp.bindHost);
        VIR_FREE(def->data.udp.bindService);
        VIR_FREE(def->data.udp.connectHost);
        VIR_FREE(def->data.udp.connectService);
        break;

    case VIR_DOMAIN_CHR_TYPE_TCP:
        VIR_FREE(def->data.tcp.host);
        VIR_FREE(def->data.tcp.service);
        break;

    case VIR_DOMAIN_CHR_TYPE_UNIX:
        VIR_FREE(def->data.nix.path);
        break;
    }
}

void virDomainChrSourceDefFree(virDomainChrSourceDefPtr def)
{
    if (!def)
        return;

    virDomainChrSourceDefClear(def);

    VIR_FREE(def);
}

void virDomainChrDefFree(virDomainChrDefPtr def)
{
    if (!def)
        return;

    switch (def->deviceType) {
    case VIR_DOMAIN_CHR_DEVICE_TYPE_CHANNEL:
        switch (def->targetType) {
        case VIR_DOMAIN_CHR_CHANNEL_TARGET_TYPE_GUESTFWD:
            VIR_FREE(def->target.addr);
            break;

        case VIR_DOMAIN_CHR_CHANNEL_TARGET_TYPE_VIRTIO:
            VIR_FREE(def->target.name);
            break;
        }
        break;

    default:
        break;
    }

    virDomainChrSourceDefClear(&def->source);
    virDomainDeviceInfoClear(&def->info);

    VIR_FREE(def);
}

void virDomainSmartcardDefFree(virDomainSmartcardDefPtr def)
{
    size_t i;
    if (!def)
        return;

    switch (def->type) {
    case VIR_DOMAIN_SMARTCARD_TYPE_HOST:
        break;

    case VIR_DOMAIN_SMARTCARD_TYPE_HOST_CERTIFICATES:
        for (i = 0; i < VIR_DOMAIN_SMARTCARD_NUM_CERTIFICATES; i++)
            VIR_FREE(def->data.cert.file[i]);
        VIR_FREE(def->data.cert.database);
        break;

    case VIR_DOMAIN_SMARTCARD_TYPE_PASSTHROUGH:
        virDomainChrSourceDefClear(&def->data.passthru);
        break;

    default:
        break;
    }

    virDomainDeviceInfoClear(&def->info);

    VIR_FREE(def);
}

void virDomainSoundDefFree(virDomainSoundDefPtr def)
{
    if (!def)
        return;

    virDomainDeviceInfoClear(&def->info);

    VIR_FREE(def);
}

void virDomainMemballoonDefFree(virDomainMemballoonDefPtr def)
{
    if (!def)
        return;

    virDomainDeviceInfoClear(&def->info);

    VIR_FREE(def);
}

void virDomainWatchdogDefFree(virDomainWatchdogDefPtr def)
{
    if (!def)
        return;

    virDomainDeviceInfoClear(&def->info);

    VIR_FREE(def);
}

void virDomainVideoDefFree(virDomainVideoDefPtr def)
{
    if (!def)
        return;

    virDomainDeviceInfoClear(&def->info);

    VIR_FREE(def->accel);
    VIR_FREE(def);
}

void virDomainHostdevDefFree(virDomainHostdevDefPtr def)
{
    if (!def)
        return;

    VIR_FREE(def->target);
    virDomainDeviceInfoClear(&def->info);
    VIR_FREE(def);
}

void virDomainDeviceDefFree(virDomainDeviceDefPtr def)
{
    if (!def)
        return;

    switch (def->type) {
    case VIR_DOMAIN_DEVICE_DISK:
        virDomainDiskDefFree(def->data.disk);
        break;
    case VIR_DOMAIN_DEVICE_NET:
        virDomainNetDefFree(def->data.net);
        break;
    case VIR_DOMAIN_DEVICE_INPUT:
        virDomainInputDefFree(def->data.input);
        break;
    case VIR_DOMAIN_DEVICE_SOUND:
        virDomainSoundDefFree(def->data.sound);
        break;
    case VIR_DOMAIN_DEVICE_VIDEO:
        virDomainVideoDefFree(def->data.video);
        break;
    case VIR_DOMAIN_DEVICE_HOSTDEV:
        virDomainHostdevDefFree(def->data.hostdev);
        break;
    case VIR_DOMAIN_DEVICE_WATCHDOG:
        virDomainWatchdogDefFree(def->data.watchdog);
        break;
    case VIR_DOMAIN_DEVICE_CONTROLLER:
        virDomainControllerDefFree(def->data.controller);
        break;
    case VIR_DOMAIN_DEVICE_GRAPHICS:
        virDomainGraphicsDefFree(def->data.graphics);
        break;
    }

    VIR_FREE(def);
}

void virSecurityLabelDefFree(virDomainDefPtr def);

void virSecurityLabelDefFree(virDomainDefPtr def)
{
    VIR_FREE(def->seclabel.model);
    VIR_FREE(def->seclabel.label);
    VIR_FREE(def->seclabel.imagelabel);
}

static void
virDomainClockDefClear(virDomainClockDefPtr def)
{
    if (def->offset == VIR_DOMAIN_CLOCK_OFFSET_TIMEZONE)
        VIR_FREE(def->data.timezone);

    int i;
    for (i = 0; i < def->ntimers; i++)
        VIR_FREE(def->timers[i]);
    VIR_FREE(def->timers);
}

void virDomainDefFree(virDomainDefPtr def)
{
    unsigned int i;

    if (!def)
        return;

    for (i = 0 ; i < def->ngraphics ; i++)
        virDomainGraphicsDefFree(def->graphics[i]);
    VIR_FREE(def->graphics);

    for (i = 0 ; i < def->ninputs ; i++)
        virDomainInputDefFree(def->inputs[i]);
    VIR_FREE(def->inputs);

    for (i = 0 ; i < def->ndisks ; i++)
        virDomainDiskDefFree(def->disks[i]);
    VIR_FREE(def->disks);

    for (i = 0 ; i < def->ncontrollers ; i++)
        virDomainControllerDefFree(def->controllers[i]);
    VIR_FREE(def->controllers);

    for (i = 0 ; i < def->nfss ; i++)
        virDomainFSDefFree(def->fss[i]);
    VIR_FREE(def->fss);

    for (i = 0 ; i < def->nnets ; i++)
        virDomainNetDefFree(def->nets[i]);
    VIR_FREE(def->nets);

    for (i = 0 ; i < def->nsmartcards ; i++)
        virDomainSmartcardDefFree(def->smartcards[i]);
    VIR_FREE(def->smartcards);

    for (i = 0 ; i < def->nserials ; i++)
        virDomainChrDefFree(def->serials[i]);
    VIR_FREE(def->serials);

    for (i = 0 ; i < def->nparallels ; i++)
        virDomainChrDefFree(def->parallels[i]);
    VIR_FREE(def->parallels);

    for (i = 0 ; i < def->nchannels ; i++)
        virDomainChrDefFree(def->channels[i]);
    VIR_FREE(def->channels);

    virDomainChrDefFree(def->console);

    for (i = 0 ; i < def->nsounds ; i++)
        virDomainSoundDefFree(def->sounds[i]);
    VIR_FREE(def->sounds);

    for (i = 0 ; i < def->nvideos ; i++)
        virDomainVideoDefFree(def->videos[i]);
    VIR_FREE(def->videos);

    for (i = 0 ; i < def->nhostdevs ; i++)
        virDomainHostdevDefFree(def->hostdevs[i]);
    VIR_FREE(def->hostdevs);

    VIR_FREE(def->os.type);
    VIR_FREE(def->os.arch);
    VIR_FREE(def->os.machine);
    VIR_FREE(def->os.init);
    VIR_FREE(def->os.kernel);
    VIR_FREE(def->os.initrd);
    VIR_FREE(def->os.cmdline);
    VIR_FREE(def->os.root);
    VIR_FREE(def->os.loader);
    VIR_FREE(def->os.bootloader);
    VIR_FREE(def->os.bootloaderArgs);

    virDomainClockDefClear(&def->clock);

    VIR_FREE(def->name);
    VIR_FREE(def->cpumask);
    VIR_FREE(def->emulator);
    VIR_FREE(def->description);

    virDomainWatchdogDefFree(def->watchdog);

    virDomainMemballoonDefFree(def->memballoon);

    virSecurityLabelDefFree(def);

    virCPUDefFree(def->cpu);

    virSysinfoDefFree(def->sysinfo);

    if (def->namespaceData && def->ns.free)
        (def->ns.free)(def->namespaceData);

    VIR_FREE(def);
}

static void virDomainSnapshotObjListDeinit(virDomainSnapshotObjListPtr snapshots);
static void virDomainObjFree(virDomainObjPtr dom)
{
    if (!dom)
        return;

    VIR_DEBUG("obj=%p", dom);
    virDomainDefFree(dom->def);
    virDomainDefFree(dom->newDef);

    if (dom->privateDataFreeFunc)
        (dom->privateDataFreeFunc)(dom->privateData);

    virMutexDestroy(&dom->lock);

    virDomainSnapshotObjListDeinit(&dom->snapshots);

    VIR_FREE(dom);
}

void virDomainObjRef(virDomainObjPtr dom)
{
    dom->refs++;
    VIR_DEBUG("obj=%p refs=%d", dom, dom->refs);
}


int virDomainObjUnref(virDomainObjPtr dom)
{
    dom->refs--;
    VIR_DEBUG("obj=%p refs=%d", dom, dom->refs);
    if (dom->refs == 0) {
        virDomainObjUnlock(dom);
        virDomainObjFree(dom);
        return 0;
    }
    return dom->refs;
}

static virDomainObjPtr virDomainObjNew(virCapsPtr caps)
{
    virDomainObjPtr domain;

    if (VIR_ALLOC(domain) < 0) {
        virReportOOMError();
        return NULL;
    }

    if (caps->privateDataAllocFunc &&
        !(domain->privateData = (caps->privateDataAllocFunc)())) {
        virReportOOMError();
        VIR_FREE(domain);
        return NULL;
    }
    domain->privateDataFreeFunc = caps->privateDataFreeFunc;

    if (virMutexInit(&domain->lock) < 0) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                             "%s", _("cannot initialize mutex"));
        if (domain->privateDataFreeFunc)
            (domain->privateDataFreeFunc)(domain->privateData);
        VIR_FREE(domain);
        return NULL;
    }

    virDomainObjLock(domain);
    domain->state = VIR_DOMAIN_SHUTOFF;
    domain->refs = 1;

    virDomainSnapshotObjListInit(&domain->snapshots);

    VIR_DEBUG("obj=%p", domain);
    return domain;
}

void virDomainObjAssignDef(virDomainObjPtr domain,
                           const virDomainDefPtr def,
                           bool live)
{
    if (!virDomainObjIsActive(domain)) {
        if (live) {
            /* save current configuration to be restored on domain shutdown */
            if (!domain->newDef)
                domain->newDef = domain->def;
            domain->def = def;
        } else {
            virDomainDefFree(domain->def);
            domain->def = def;
        }
    } else {
        virDomainDefFree(domain->newDef);
        domain->newDef = def;
    }
}

virDomainObjPtr virDomainAssignDef(virCapsPtr caps,
                                   virDomainObjListPtr doms,
                                   const virDomainDefPtr def,
                                   bool live)
{
    virDomainObjPtr domain;
    char uuidstr[VIR_UUID_STRING_BUFLEN];

    if ((domain = virDomainFindByUUID(doms, def->uuid))) {
        virDomainObjAssignDef(domain, def, live);
        return domain;
    }

    if (!(domain = virDomainObjNew(caps)))
        return NULL;
    domain->def = def;

    virUUIDFormat(def->uuid, uuidstr);
    if (virHashAddEntry(doms->objs, uuidstr, domain) < 0) {
        VIR_FREE(domain);
        virReportOOMError();
        return NULL;
    }

    return domain;
}

/*
 * Mark the running VM config as transient. Ensures transient hotplug
 * operations do not persist past shutdown.
 *
 * @param caps pointer to capabilities info
 * @param domain domain object pointer
 * @param live if true, run this operation even for an inactive domain.
 *   this allows freely updated domain->def with runtime defaults before
 *   starting the VM, which will be discarded on VM shutdown. Any cleanup
 *   paths need to be sure to handle newDef if the domain is never started.
 * @return 0 on success, -1 on failure
 */
int
virDomainObjSetDefTransient(virCapsPtr caps,
                            virDomainObjPtr domain,
                            bool live)
{
    int ret = -1;
    char *xml = NULL;
    virDomainDefPtr newDef = NULL;

    if (!virDomainObjIsActive(domain) && !live)
        return 0;

    if (!domain->persistent)
        return 0;

    if (domain->newDef)
        return 0;

    if (!(xml = virDomainDefFormat(domain->def, VIR_DOMAIN_XML_WRITE_FLAGS)))
        goto out;

    if (!(newDef = virDomainDefParseString(caps, xml,
                                           VIR_DOMAIN_XML_READ_FLAGS)))
        goto out;

    domain->newDef = newDef;
    ret = 0;
out:
    VIR_FREE(xml);
    return ret;
}

/*
 * Return the persistent domain configuration. If domain is transient,
 * return the running config.
 *
 * @param caps pointer to capabilities info
 * @param domain domain object pointer
 * @return NULL on error, virDOmainDefPtr on success
 */
virDomainDefPtr
virDomainObjGetPersistentDef(virCapsPtr caps,
                             virDomainObjPtr domain)
{
    if (virDomainObjSetDefTransient(caps, domain, false) < 0)
        return NULL;

    if (domain->newDef)
        return domain->newDef;
    else
        return domain->def;
}

/*
 * The caller must hold a lock  on the driver owning 'doms',
 * and must also have locked 'dom', to ensure no one else
 * is either waiting for 'dom' or still usingn it
 */
void virDomainRemoveInactive(virDomainObjListPtr doms,
                             virDomainObjPtr dom)
{
    char uuidstr[VIR_UUID_STRING_BUFLEN];
    virUUIDFormat(dom->def->uuid, uuidstr);

    virDomainObjUnlock(dom);

    virHashRemoveEntry(doms->objs, uuidstr, virDomainObjListDeallocator);
}


int virDomainDeviceAddressIsValid(virDomainDeviceInfoPtr info,
                                  int type)
{
    if (info->type != type)
        return 0;

    switch (info->type) {
    case VIR_DOMAIN_DEVICE_ADDRESS_TYPE_PCI:
        return virDomainDevicePCIAddressIsValid(&info->addr.pci);

    case VIR_DOMAIN_DEVICE_ADDRESS_TYPE_DRIVE:
        return virDomainDeviceDriveAddressIsValid(&info->addr.drive);
    }

    return 0;
}


int virDomainDevicePCIAddressIsValid(virDomainDevicePCIAddressPtr addr)
{
    return addr->domain || addr->bus || addr->slot;
}


int virDomainDeviceDriveAddressIsValid(virDomainDeviceDriveAddressPtr addr ATTRIBUTE_UNUSED)
{
    /*return addr->controller || addr->bus || addr->unit;*/
    return 1; /* 0 is valid for all fields, so any successfully parsed addr is valid */
}


int virDomainDeviceVirtioSerialAddressIsValid(
    virDomainDeviceVirtioSerialAddressPtr addr ATTRIBUTE_UNUSED)
{
    return 1; /* 0 is valid for all fields, so any successfully parsed addr is valid */
}


int virDomainDeviceInfoIsSet(virDomainDeviceInfoPtr info)
{
    if (info->type != VIR_DOMAIN_DEVICE_ADDRESS_TYPE_NONE)
        return 1;
    if (info->alias)
        return 1;
    return 0;
}


void virDomainDeviceInfoClear(virDomainDeviceInfoPtr info)
{
    VIR_FREE(info->alias);
    memset(&info->addr, 0, sizeof(info->addr));
    info->type = VIR_DOMAIN_DEVICE_ADDRESS_TYPE_NONE;
}


static int virDomainDeviceInfoClearAlias(virDomainDefPtr def ATTRIBUTE_UNUSED,
                                         virDomainDeviceInfoPtr info,
                                         void *opaque ATTRIBUTE_UNUSED)
{
    VIR_FREE(info->alias);
    return 0;
}

static int virDomainDeviceInfoClearPCIAddress(virDomainDefPtr def ATTRIBUTE_UNUSED,
                                              virDomainDeviceInfoPtr info,
                                              void *opaque ATTRIBUTE_UNUSED)
{
    if (info->type == VIR_DOMAIN_DEVICE_ADDRESS_TYPE_PCI) {
        memset(&info->addr, 0, sizeof(info->addr));
        info->type = VIR_DOMAIN_DEVICE_ADDRESS_TYPE_NONE;
    }
    return 0;
}

int virDomainDeviceInfoIterate(virDomainDefPtr def,
                               virDomainDeviceInfoCallback cb,
                               void *opaque)
{
    int i;

    for (i = 0; i < def->ndisks ; i++)
        if (cb(def, &def->disks[i]->info, opaque) < 0)
            return -1;
    for (i = 0; i < def->nnets ; i++)
        if (cb(def, &def->nets[i]->info, opaque) < 0)
            return -1;
    for (i = 0; i < def->nsounds ; i++)
        if (cb(def, &def->sounds[i]->info, opaque) < 0)
            return -1;
    for (i = 0; i < def->nhostdevs ; i++)
        if (cb(def, &def->hostdevs[i]->info, opaque) < 0)
            return -1;
    for (i = 0; i < def->nvideos ; i++)
        if (cb(def, &def->videos[i]->info, opaque) < 0)
            return -1;
    for (i = 0; i < def->ncontrollers ; i++)
        if (cb(def, &def->controllers[i]->info, opaque) < 0)
            return -1;
    for (i = 0; i < def->nsmartcards ; i++)
        if (cb(def, &def->smartcards[i]->info, opaque) < 0)
            return -1;
    for (i = 0; i < def->nserials ; i++)
        if (cb(def, &def->serials[i]->info, opaque) < 0)
            return -1;
    for (i = 0; i < def->nparallels ; i++)
        if (cb(def, &def->parallels[i]->info, opaque) < 0)
            return -1;
    for (i = 0; i < def->nchannels ; i++)
        if (cb(def, &def->channels[i]->info, opaque) < 0)
            return -1;
    for (i = 0; i < def->ninputs ; i++)
        if (cb(def, &def->inputs[i]->info, opaque) < 0)
            return -1;
    for (i = 0; i < def->nfss ; i++)
        if (cb(def, &def->fss[i]->info, opaque) < 0)
            return -1;
    if (def->watchdog)
        if (cb(def, &def->watchdog->info, opaque) < 0)
            return -1;
    if (def->memballoon)
        if (cb(def, &def->memballoon->info, opaque) < 0)
            return -1;
    if (def->console)
        if (cb(def, &def->console->info, opaque) < 0)
            return -1;
    return 0;
}


void virDomainDefClearPCIAddresses(virDomainDefPtr def)
{
    virDomainDeviceInfoIterate(def, virDomainDeviceInfoClearPCIAddress, NULL);
}

void virDomainDefClearDeviceAliases(virDomainDefPtr def)
{
    virDomainDeviceInfoIterate(def, virDomainDeviceInfoClearAlias, NULL);
}


/* Generate a string representation of a device address
 * @param address Device address to stringify
 */
static int ATTRIBUTE_NONNULL(2)
virDomainDeviceInfoFormat(virBufferPtr buf,
                          virDomainDeviceInfoPtr info,
                          int flags)
{
    if (info->alias &&
        !(flags & VIR_DOMAIN_XML_INACTIVE)) {
        virBufferVSprintf(buf, "      <alias name='%s'/>\n", info->alias);
    }

    if (info->type == VIR_DOMAIN_DEVICE_ADDRESS_TYPE_NONE)
        return 0;

    /* We'll be in domain/devices/[device type]/ so 3 level indent */
    virBufferVSprintf(buf, "      <address type='%s'",
                      virDomainDeviceAddressTypeToString(info->type));

    switch (info->type) {
    case VIR_DOMAIN_DEVICE_ADDRESS_TYPE_PCI:
        virBufferVSprintf(buf, " domain='0x%.4x' bus='0x%.2x' slot='0x%.2x' function='0x%.1x'",
                          info->addr.pci.domain,
                          info->addr.pci.bus,
                          info->addr.pci.slot,
                          info->addr.pci.function);
        break;

    case VIR_DOMAIN_DEVICE_ADDRESS_TYPE_DRIVE:
        virBufferVSprintf(buf, " controller='%d' bus='%d' unit='%d'",
                          info->addr.drive.controller,
                          info->addr.drive.bus,
                          info->addr.drive.unit);
        break;

    case VIR_DOMAIN_DEVICE_ADDRESS_TYPE_VIRTIO_SERIAL:
        virBufferVSprintf(buf, " controller='%d' bus='%d' port='%d'",
                          info->addr.vioserial.controller,
                          info->addr.vioserial.bus,
                          info->addr.vioserial.port);
        break;

    case VIR_DOMAIN_DEVICE_ADDRESS_TYPE_CCID:
        virBufferVSprintf(buf, " controller='%d' slot='%d'",
                          info->addr.ccid.controller,
                          info->addr.ccid.slot);
        break;

    default:
        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                             _("unknown address type '%d'"), info->type);
        return -1;
    }

    virBufferAddLit(buf, "/>\n");

    return 0;
}


static int
virDomainDevicePCIAddressParseXML(xmlNodePtr node,
                                  virDomainDevicePCIAddressPtr addr)
{
    char *domain, *slot, *bus, *function;
    int ret = -1;

    memset(addr, 0, sizeof(*addr));

    domain   = virXMLPropString(node, "domain");
    bus      = virXMLPropString(node, "bus");
    slot     = virXMLPropString(node, "slot");
    function = virXMLPropString(node, "function");

    if (domain &&
        virStrToLong_ui(domain, NULL, 0, &addr->domain) < 0) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR, "%s",
                             _("Cannot parse <address> 'domain' attribute"));
        goto cleanup;
    }

    if (bus &&
        virStrToLong_ui(bus, NULL, 0, &addr->bus) < 0) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR, "%s",
                             _("Cannot parse <address> 'bus' attribute"));
        goto cleanup;
    }

    if (slot &&
        virStrToLong_ui(slot, NULL, 0, &addr->slot) < 0) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR, "%s",
                             _("Cannot parse <address> 'slot' attribute"));
        goto cleanup;
    }

    if (function &&
        virStrToLong_ui(function, NULL, 0, &addr->function) < 0) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR, "%s",
                             _("Cannot parse <address> 'function' attribute"));
        goto cleanup;
    }

    if (!virDomainDevicePCIAddressIsValid(addr)) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR, "%s",
                             _("Insufficient specification for PCI address"));
        goto cleanup;
    }

    ret = 0;

cleanup:
    VIR_FREE(domain);
    VIR_FREE(bus);
    VIR_FREE(slot);
    VIR_FREE(function);
    return ret;
}


static int
virDomainDeviceDriveAddressParseXML(xmlNodePtr node,
                                    virDomainDeviceDriveAddressPtr addr)
{
    char *bus, *unit, *controller;
    int ret = -1;

    memset(addr, 0, sizeof(*addr));

    controller = virXMLPropString(node, "controller");
    bus = virXMLPropString(node, "bus");
    unit = virXMLPropString(node, "unit");

    if (controller &&
        virStrToLong_ui(controller, NULL, 10, &addr->controller) < 0) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR, "%s",
                             _("Cannot parse <address> 'controller' attribute"));
        goto cleanup;
    }

    if (bus &&
        virStrToLong_ui(bus, NULL, 10, &addr->bus) < 0) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR, "%s",
                             _("Cannot parse <address> 'bus' attribute"));
        goto cleanup;
    }

    if (unit &&
        virStrToLong_ui(unit, NULL, 10, &addr->unit) < 0) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR, "%s",
                             _("Cannot parse <address> 'unit' attribute"));
        goto cleanup;
    }

    if (!virDomainDeviceDriveAddressIsValid(addr)) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR, "%s",
                             _("Insufficient specification for drive address"));
        goto cleanup;
    }

    ret = 0;

cleanup:
    VIR_FREE(controller);
    VIR_FREE(bus);
    VIR_FREE(unit);
    return ret;
}


static int
virDomainDeviceVirtioSerialAddressParseXML(
    xmlNodePtr node,
    virDomainDeviceVirtioSerialAddressPtr addr
)
{
    char *controller, *bus, *port;
    int ret = -1;

    memset(addr, 0, sizeof(*addr));

    controller = virXMLPropString(node, "controller");
    bus = virXMLPropString(node, "bus");
    port = virXMLPropString(node, "port");

    if (controller &&
        virStrToLong_ui(controller, NULL, 10, &addr->controller) < 0) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR, "%s",
                             _("Cannot parse <address> 'controller' attribute"));
        goto cleanup;
    }

    if (bus &&
        virStrToLong_ui(bus, NULL, 10, &addr->bus) < 0) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR, "%s",
                             _("Cannot parse <address> 'bus' attribute"));
        goto cleanup;
    }

    if (port &&
        virStrToLong_ui(port, NULL, 10, &addr->port) < 0) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR, "%s",
                             _("Cannot parse <address> 'port' attribute"));
        goto cleanup;
    }

    if (!virDomainDeviceVirtioSerialAddressIsValid(addr)) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR, "%s",
                             _("Insufficient specification for "
                               "virtio serial address"));
        goto cleanup;
    }

    ret = 0;

cleanup:
    VIR_FREE(controller);
    VIR_FREE(bus);
    VIR_FREE(port);
    return ret;
}

static int
virDomainDeviceCcidAddressParseXML(xmlNodePtr node,
                                   virDomainDeviceCcidAddressPtr addr)
{
    char *controller, *slot;
    int ret = -1;

    memset(addr, 0, sizeof(*addr));

    controller = virXMLPropString(node, "controller");
    slot = virXMLPropString(node, "slot");

    if (controller &&
        virStrToLong_ui(controller, NULL, 10, &addr->controller) < 0) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR, "%s",
                             _("Cannot parse <address> 'controller' attribute"));
        goto cleanup;
    }

    if (slot &&
        virStrToLong_ui(slot, NULL, 10, &addr->slot) < 0) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR, "%s",
                             _("Cannot parse <address> 'slot' attribute"));
        goto cleanup;
    }

    ret = 0;

cleanup:
    VIR_FREE(controller);
    VIR_FREE(slot);
    return ret;
}

/* Parse the XML definition for a device address
 * @param node XML nodeset to parse for device address definition
 */
static int
virDomainDeviceInfoParseXML(xmlNodePtr node,
                            virDomainDeviceInfoPtr info,
                            int flags)
{
    xmlNodePtr cur;
    xmlNodePtr address = NULL;
    xmlNodePtr alias = NULL;
    char *type = NULL;
    int ret = -1;

    virDomainDeviceInfoClear(info);

    cur = node->children;
    while (cur != NULL) {
        if (cur->type == XML_ELEMENT_NODE) {
            if (alias == NULL &&
                !(flags & VIR_DOMAIN_XML_INACTIVE) &&
                xmlStrEqual(cur->name, BAD_CAST "alias")) {
                alias = cur;
            } else if (address == NULL &&
                       xmlStrEqual(cur->name, BAD_CAST "address")) {
                address = cur;
            }
        }
        cur = cur->next;
    }

    if (alias)
        info->alias = virXMLPropString(alias, "name");

    if (!address)
        return 0;

    type = virXMLPropString(address, "type");

    if (type) {
        if ((info->type = virDomainDeviceAddressTypeFromString(type)) < 0) {
            virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                                 _("unknown address type '%s'"), type);
            goto cleanup;
        }
    } else {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                             "%s", _("No type specified for device address"));
        goto cleanup;
    }

    switch (info->type) {
    case VIR_DOMAIN_DEVICE_ADDRESS_TYPE_PCI:
        if (virDomainDevicePCIAddressParseXML(address, &info->addr.pci) < 0)
            goto cleanup;
        break;

    case VIR_DOMAIN_DEVICE_ADDRESS_TYPE_DRIVE:
        if (virDomainDeviceDriveAddressParseXML(address, &info->addr.drive) < 0)
            goto cleanup;
        break;

    case VIR_DOMAIN_DEVICE_ADDRESS_TYPE_VIRTIO_SERIAL:
        if (virDomainDeviceVirtioSerialAddressParseXML
                (address, &info->addr.vioserial) < 0)
            goto cleanup;
        break;

    case VIR_DOMAIN_DEVICE_ADDRESS_TYPE_CCID:
        if (virDomainDeviceCcidAddressParseXML(address, &info->addr.ccid) < 0)
            goto cleanup;
        break;

    default:
        /* Should not happen */
        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                             "%s", _("Unknown device address type"));
        goto cleanup;
    }

    ret = 0;

cleanup:
    if (ret == -1)
        VIR_FREE(info->alias);
    VIR_FREE(type);
    return ret;
}

static int
virDomainDeviceBootParseXML(xmlNodePtr node,
                            int *bootIndex,
                            virBitmapPtr bootMap)
{
    char *order;
    int boot;
    int ret = -1;

    order = virXMLPropString(node, "order");
    if (!order) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                            "%s", _("missing boot order attribute"));
        goto cleanup;
    } else if (virStrToLong_i(order, NULL, 10, &boot) < 0 ||
               boot <= 0) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                _("incorrect boot order '%s', expecting positive integer"),
                order);
        goto cleanup;
    }

    if (bootMap) {
        bool set;
        if (virBitmapGetBit(bootMap, boot - 1, &set) < 0) {
            virDomainReportError(VIR_ERR_INTERNAL_ERROR, "%s",
                    _("boot orders have to be contiguous and starting from 1"));
            goto cleanup;
        } else if (set) {
            virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                    _("boot order %d used for more than one device"), boot);
            goto cleanup;
        }
        ignore_value(virBitmapSetBit(bootMap, boot - 1));
    }

    *bootIndex = boot;
    ret = 0;

cleanup:
    VIR_FREE(order);
    return ret;
}

static int
virDomainParseLegacyDeviceAddress(char *devaddr,
                                  virDomainDevicePCIAddressPtr pci)
{
    char *tmp;

    /* expected format: <domain>:<bus>:<slot> */
    if (/* domain */
        virStrToLong_ui(devaddr, &tmp, 16, &pci->domain) < 0 || *tmp != ':' ||
        /* bus */
        virStrToLong_ui(tmp + 1, &tmp, 16, &pci->bus) < 0 || *tmp != ':' ||
        /* slot */
        virStrToLong_ui(tmp + 1, NULL, 16, &pci->slot) < 0)
        return -1;

    return 0;
}

int
virDomainDiskDefAssignAddress(virCapsPtr caps, virDomainDiskDefPtr def)
{
    int idx = virDiskNameToIndex(def->dst);
    if (idx < 0)
        return -1;

    switch (def->bus) {
    case VIR_DOMAIN_DISK_BUS_SCSI:
        def->info.type = VIR_DOMAIN_DEVICE_ADDRESS_TYPE_DRIVE;

        if (caps->hasWideScsiBus) {
            /* For a wide SCSI bus we define the default mapping to be
             * 16 units per bus, 1 bus per controller, many controllers.
             * Unit 7 is the SCSI controller itself. Therefore unit 7
             * cannot be assigned to disks and is skipped.
             */
            def->info.addr.drive.controller = idx / 15;
            def->info.addr.drive.bus = 0;
            def->info.addr.drive.unit = idx % 15;

            /* Skip the SCSI controller at unit 7 */
            if (def->info.addr.drive.unit >= 7) {
                ++def->info.addr.drive.unit;
            }
        } else {
            /* For a narrow SCSI bus we define the default mapping to be
             * 7 units per bus, 1 bus per controller, many controllers */
            def->info.addr.drive.controller = idx / 7;
            def->info.addr.drive.bus = 0;
            def->info.addr.drive.unit = idx % 7;
        }

        break;

    case VIR_DOMAIN_DISK_BUS_IDE:
        /* For IDE we define the default mapping to be 2 units
         * per bus, 2 bus per controller, many controllers */
        def->info.type = VIR_DOMAIN_DEVICE_ADDRESS_TYPE_DRIVE;
        def->info.addr.drive.controller = idx / 4;
        def->info.addr.drive.bus = (idx % 4) / 2;
        def->info.addr.drive.unit = (idx % 2);
        break;

    case VIR_DOMAIN_DISK_BUS_FDC:
        /* For FDC we define the default mapping to be 2 units
         * per bus, 1 bus per controller, many controllers */
        def->info.type = VIR_DOMAIN_DEVICE_ADDRESS_TYPE_DRIVE;
        def->info.addr.drive.controller = idx / 2;
        def->info.addr.drive.bus = 0;
        def->info.addr.drive.unit = idx % 2;
        break;

    default:
        /* Other disk bus's aren't controller based */
        break;
    }

    return 0;
}

/* Parse the XML definition for a disk
 * @param node XML nodeset to parse for disk definition
 */
static virDomainDiskDefPtr
virDomainDiskDefParseXML(virCapsPtr caps,
                         xmlNodePtr node,
                         virBitmapPtr bootMap,
                         int flags)
{
    virDomainDiskDefPtr def;
    xmlNodePtr cur, host;
    char *type = NULL;
    char *device = NULL;
    char *driverName = NULL;
    char *driverType = NULL;
    char *source = NULL;
    char *target = NULL;
    char *protocol = NULL;
    virDomainDiskHostDefPtr hosts = NULL;
    int nhosts = 0;
    char *bus = NULL;
    char *cachetag = NULL;
    char *error_policy = NULL;
    char *iotag = NULL;
    char *devaddr = NULL;
    virStorageEncryptionPtr encryption = NULL;
    char *serial = NULL;

    if (VIR_ALLOC(def) < 0) {
        virReportOOMError();
        return NULL;
    }

    type = virXMLPropString(node, "type");
    if (type) {
        if ((def->type = virDomainDiskTypeFromString(type)) < 0) {
            virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                                 _("unknown disk type '%s'"), type);
            goto error;
        }
    } else {
        def->type = VIR_DOMAIN_DISK_TYPE_FILE;
    }

    cur = node->children;
    while (cur != NULL) {
        if (cur->type == XML_ELEMENT_NODE) {
            if ((source == NULL && hosts == NULL) &&
                (xmlStrEqual(cur->name, BAD_CAST "source"))) {

                switch (def->type) {
                case VIR_DOMAIN_DISK_TYPE_FILE:
                    source = virXMLPropString(cur, "file");
                    break;
                case VIR_DOMAIN_DISK_TYPE_BLOCK:
                    source = virXMLPropString(cur, "dev");
                    break;
                case VIR_DOMAIN_DISK_TYPE_DIR:
                    source = virXMLPropString(cur, "dir");
                    break;
                case VIR_DOMAIN_DISK_TYPE_NETWORK:
                    protocol = virXMLPropString(cur, "protocol");
                    if (protocol == NULL) {
                        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                                             "%s", _("missing protocol type"));
                        goto error;
                    }
                    def->protocol = virDomainDiskProtocolTypeFromString(protocol);
                    if (def->protocol < 0) {
                        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                                             _("unknown protocol type '%s'"),
                                             protocol);
                        goto error;
                    }
                    if (!(source = virXMLPropString(cur, "name")) &&
                        def->protocol != VIR_DOMAIN_DISK_PROTOCOL_NBD) {
                        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                                             _("missing name for disk source"));
                        goto error;
                    }
                    host = cur->children;
                    while (host != NULL) {
                        if (host->type == XML_ELEMENT_NODE &&
                            xmlStrEqual(host->name, BAD_CAST "host")) {
                            if (VIR_REALLOC_N(hosts, nhosts + 1) < 0) {
                                virReportOOMError();
                                goto error;
                            }
                            hosts[nhosts].name = NULL;
                            hosts[nhosts].port = NULL;
                            nhosts++;

                            hosts[nhosts - 1].name = virXMLPropString(host, "name");
                            if (!hosts[nhosts - 1].name) {
                                virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                                                     "%s", _("missing name for host"));
                                goto error;
                            }
                            hosts[nhosts - 1].port = virXMLPropString(host, "port");
                            if (!hosts[nhosts - 1].port) {
                                virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                                                     "%s", _("missing port for host"));
                                goto error;
                            }
                        }
                        host = host->next;
                    }
                    break;
                default:
                    virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                                         _("unexpected disk type %s"),
                                         virDomainDiskTypeToString(def->type));
                    goto error;
                }

                /* People sometimes pass a bogus '' source path
                   when they mean to omit the source element
                   completely. eg CDROM without media. This is
                   just a little compatability check to help
                   those broken apps */
                if (source && STREQ(source, ""))
                    VIR_FREE(source);
            } else if ((target == NULL) &&
                       (xmlStrEqual(cur->name, BAD_CAST "target"))) {
                target = virXMLPropString(cur, "dev");
                bus = virXMLPropString(cur, "bus");

                /* HACK: Work around for compat with Xen
                 * driver in previous libvirt releases */
                if (target &&
                    STRPREFIX(target, "ioemu:"))
                    memmove(target, target+6, strlen(target)-5);
            } else if ((driverName == NULL) &&
                       (xmlStrEqual(cur->name, BAD_CAST "driver"))) {
                driverName = virXMLPropString(cur, "name");
                driverType = virXMLPropString(cur, "type");
                cachetag = virXMLPropString(cur, "cache");
                error_policy = virXMLPropString(cur, "error_policy");
                iotag = virXMLPropString(cur, "io");
            } else if (xmlStrEqual(cur->name, BAD_CAST "readonly")) {
                def->readonly = 1;
            } else if (xmlStrEqual(cur->name, BAD_CAST "shareable")) {
                def->shared = 1;
            } else if ((flags & VIR_DOMAIN_XML_INTERNAL_STATUS) &&
                       xmlStrEqual(cur->name, BAD_CAST "state")) {
                /* Legacy back-compat. Don't add any more attributes here */
                devaddr = virXMLPropString(cur, "devaddr");
            } else if (encryption == NULL &&
                       xmlStrEqual(cur->name, BAD_CAST "encryption")) {
                encryption = virStorageEncryptionParseNode(node->doc,
                                                           cur);
                if (encryption == NULL)
                    goto error;
            } else if ((serial == NULL) &&
                       (xmlStrEqual(cur->name, BAD_CAST "serial"))) {
                serial = (char *)xmlNodeGetContent(cur);
            } else if (xmlStrEqual(cur->name, BAD_CAST "boot")) {
                if (virDomainDeviceBootParseXML(cur, &def->bootIndex,
                                                bootMap))
                    goto error;
            }
        }
        cur = cur->next;
    }

    device = virXMLPropString(node, "device");
    if (device) {
        if ((def->device = virDomainDiskDeviceTypeFromString(device)) < 0) {
            virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                                 _("unknown disk device '%s'"), device);
            goto error;
        }
    } else {
        def->device = VIR_DOMAIN_DISK_DEVICE_DISK;
    }

    /* Only CDROM and Floppy devices are allowed missing source path
     * to indicate no media present */
    if (source == NULL && hosts == NULL &&
        def->device != VIR_DOMAIN_DISK_DEVICE_CDROM &&
        def->device != VIR_DOMAIN_DISK_DEVICE_FLOPPY) {
        virDomainReportError(VIR_ERR_NO_SOURCE,
                             target ? "%s" : NULL, target);
        goto error;
    }

    if (target == NULL) {
        virDomainReportError(VIR_ERR_NO_TARGET,
                             source ? "%s" : NULL, source);
        goto error;
    }

    if (def->device == VIR_DOMAIN_DISK_DEVICE_FLOPPY &&
        !STRPREFIX(target, "fd")) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                             _("Invalid floppy device name: %s"), target);
        goto error;
    }

    /* Force CDROM to be listed as read only */
    if (def->device == VIR_DOMAIN_DISK_DEVICE_CDROM)
        def->readonly = 1;

    if (def->device == VIR_DOMAIN_DISK_DEVICE_DISK &&
        !STRPREFIX((const char *)target, "hd") &&
        !STRPREFIX((const char *)target, "sd") &&
        !STRPREFIX((const char *)target, "vd") &&
        !STRPREFIX((const char *)target, "xvd") &&
        !STRPREFIX((const char *)target, "ubd")) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                             _("Invalid harddisk device name: %s"), target);
        goto error;
    }

    if (bus) {
        if ((def->bus = virDomainDiskBusTypeFromString(bus)) < 0) {
            virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                                 _("unknown disk bus type '%s'"), bus);
            goto error;
        }
    } else {
        if (def->device == VIR_DOMAIN_DISK_DEVICE_FLOPPY) {
            def->bus = VIR_DOMAIN_DISK_BUS_FDC;
        } else {
            if (STRPREFIX(target, "hd"))
                def->bus = VIR_DOMAIN_DISK_BUS_IDE;
            else if (STRPREFIX(target, "sd"))
                def->bus = VIR_DOMAIN_DISK_BUS_SCSI;
            else if (STRPREFIX(target, "vd"))
                def->bus = VIR_DOMAIN_DISK_BUS_VIRTIO;
            else if (STRPREFIX(target, "xvd"))
                def->bus = VIR_DOMAIN_DISK_BUS_XEN;
            else if (STRPREFIX(target, "ubd"))
                def->bus = VIR_DOMAIN_DISK_BUS_UML;
            else
                def->bus = VIR_DOMAIN_DISK_BUS_IDE;
        }
    }

    if (def->device == VIR_DOMAIN_DISK_DEVICE_FLOPPY &&
        def->bus != VIR_DOMAIN_DISK_BUS_FDC) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                             _("Invalid bus type '%s' for floppy disk"), bus);
        goto error;
    }
    if (def->device != VIR_DOMAIN_DISK_DEVICE_FLOPPY &&
        def->bus == VIR_DOMAIN_DISK_BUS_FDC) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                             _("Invalid bus type '%s' for disk"), bus);
        goto error;
    }

    if (cachetag &&
        (def->cachemode = virDomainDiskCacheTypeFromString(cachetag)) < 0) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                             _("unknown disk cache mode '%s'"), cachetag);
        goto error;
    }

    if (error_policy &&
        (def->error_policy = virDomainDiskErrorPolicyTypeFromString(error_policy)) < 0) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                             _("unknown disk error policy '%s'"), error_policy);
        goto error;
    }

    if (iotag) {
        if ((def->iomode = virDomainDiskIoTypeFromString(iotag)) < 0 ||
            def->iomode == VIR_DOMAIN_DISK_IO_DEFAULT) {
            virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                                 _("unknown disk io mode '%s'"), iotag);
            goto error;
        }
    }

    if (devaddr) {
        if (virDomainParseLegacyDeviceAddress(devaddr,
                                              &def->info.addr.pci) < 0) {
            virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                                 _("Unable to parse devaddr parameter '%s'"),
                                 devaddr);
            goto error;
        }
        def->info.type = VIR_DOMAIN_DEVICE_ADDRESS_TYPE_PCI;
    } else {
        if (virDomainDeviceInfoParseXML(node, &def->info, flags) < 0)
            goto error;
    }

    def->src = source;
    source = NULL;
    def->dst = target;
    target = NULL;
    def->hosts = hosts;
    hosts = NULL;
    def->nhosts = nhosts;
    nhosts = 0;
    def->driverName = driverName;
    driverName = NULL;
    def->driverType = driverType;
    driverType = NULL;
    def->encryption = encryption;
    encryption = NULL;
    def->serial = serial;
    serial = NULL;

    if (!def->driverType &&
        caps->defaultDiskDriverType &&
        !(def->driverType = strdup(caps->defaultDiskDriverType)))
        goto no_memory;

    if (!def->driverName &&
        caps->defaultDiskDriverName &&
        !(def->driverName = strdup(caps->defaultDiskDriverName)))
        goto no_memory;

    if (def->info.type == VIR_DOMAIN_DEVICE_ADDRESS_TYPE_NONE
        && virDomainDiskDefAssignAddress(caps, def) < 0)
        goto error;

cleanup:
    VIR_FREE(bus);
    VIR_FREE(type);
    VIR_FREE(target);
    VIR_FREE(source);
    while (nhosts > 0) {
        virDomainDiskHostDefFree(&hosts[nhosts - 1]);
        nhosts--;
    }
    VIR_FREE(hosts);
    VIR_FREE(protocol);
    VIR_FREE(device);
    VIR_FREE(driverType);
    VIR_FREE(driverName);
    VIR_FREE(cachetag);
    VIR_FREE(error_policy);
    VIR_FREE(iotag);
    VIR_FREE(devaddr);
    VIR_FREE(serial);
    virStorageEncryptionFree(encryption);

    return def;

no_memory:
    virReportOOMError();

 error:
    virDomainDiskDefFree(def);
    def = NULL;
    goto cleanup;
}


/* Parse the XML definition for a controller
 * @param node XML nodeset to parse for controller definition
 */
static virDomainControllerDefPtr
virDomainControllerDefParseXML(xmlNodePtr node,
                               int flags)
{
    virDomainControllerDefPtr def;
    char *type = NULL;
    char *idx = NULL;
    char *model = NULL;

    if (VIR_ALLOC(def) < 0) {
        virReportOOMError();
        return NULL;
    }

    type = virXMLPropString(node, "type");
    if (type) {
        if ((def->type = virDomainControllerTypeFromString(type)) < 0) {
            virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                                 _("Unknown controller type '%s'"), type);
            goto error;
        }
    }

    idx = virXMLPropString(node, "index");
    if (idx) {
        if (virStrToLong_i(idx, NULL, 10, &def->idx) < 0) {
            virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                                 _("Cannot parse controller index %s"), idx);
            goto error;
        }
    }

    model = virXMLPropString(node, "model");
    if (model) {
        if ((def->model = virDomainControllerModelTypeFromString(model)) < 0) {
            virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                                 _("Unknown model type '%s'"), model);
            goto error;
        }
    } else {
        def->model = -1;
    }

    if (virDomainDeviceInfoParseXML(node, &def->info, flags) < 0)
        goto error;

    switch (def->type) {
    case VIR_DOMAIN_CONTROLLER_TYPE_VIRTIO_SERIAL: {
        char *ports = virXMLPropString(node, "ports");
        if (ports) {
            int r = virStrToLong_i(ports, NULL, 10,
                                   &def->opts.vioserial.ports);
            if (r != 0 || def->opts.vioserial.ports < 0) {
                virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                                     _("Invalid ports: %s"), ports);
                VIR_FREE(ports);
                goto error;
            }
        } else {
            def->opts.vioserial.ports = -1;
        }
        VIR_FREE(ports);

        char *vectors = virXMLPropString(node, "vectors");
        if (vectors) {
            int r = virStrToLong_i(vectors, NULL, 10,
                                   &def->opts.vioserial.vectors);
            if (r != 0 || def->opts.vioserial.vectors < 0) {
                virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                                     _("Invalid vectors: %s"), vectors);
                VIR_FREE(vectors);
                goto error;
            }
        } else {
            def->opts.vioserial.vectors = -1;
        }
        VIR_FREE(vectors);
        break;
    }

    default:
        break;
    }

    if (def->info.type != VIR_DOMAIN_DEVICE_ADDRESS_TYPE_NONE &&
        def->info.type != VIR_DOMAIN_DEVICE_ADDRESS_TYPE_PCI) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR, "%s",
                             _("Controllers must use the 'pci' address type"));
        goto error;
    }

cleanup:
    VIR_FREE(type);
    VIR_FREE(idx);
    VIR_FREE(model);

    return def;

 error:
    virDomainControllerDefFree(def);
    def = NULL;
    goto cleanup;
}

/* Parse the XML definition for a disk
 * @param node XML nodeset to parse for disk definition
 */
static virDomainFSDefPtr
virDomainFSDefParseXML(xmlNodePtr node,
                       int flags) {
    virDomainFSDefPtr def;
    xmlNodePtr cur;
    char *type = NULL;
    char *source = NULL;
    char *target = NULL;
    char *accessmode = NULL;

    if (VIR_ALLOC(def) < 0) {
        virReportOOMError();
        return NULL;
    }

    type = virXMLPropString(node, "type");
    if (type) {
        if ((def->type = virDomainFSTypeFromString(type)) < 0) {
            virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                                 _("unknown filesystem type '%s'"), type);
            goto error;
        }
    } else {
        def->type = VIR_DOMAIN_FS_TYPE_MOUNT;
    }

    accessmode = virXMLPropString(node, "accessmode");
    if (accessmode) {
        if ((def->accessmode = virDomainFSAccessModeTypeFromString(accessmode)) < 0) {
            virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                                 _("unknown accessmode '%s'"), accessmode);
            goto error;
        }
    } else {
        def->accessmode = VIR_DOMAIN_FS_ACCESSMODE_PASSTHROUGH;
    }

    cur = node->children;
    while (cur != NULL) {
        if (cur->type == XML_ELEMENT_NODE) {
            if ((source == NULL) &&
                (xmlStrEqual(cur->name, BAD_CAST "source"))) {

                if (def->type == VIR_DOMAIN_FS_TYPE_MOUNT)
                    source = virXMLPropString(cur, "dir");
                else if (def->type == VIR_DOMAIN_FS_TYPE_FILE)
                    source = virXMLPropString(cur, "file");
                else if (def->type == VIR_DOMAIN_FS_TYPE_BLOCK)
                    source = virXMLPropString(cur, "dev");
                else if (def->type == VIR_DOMAIN_FS_TYPE_TEMPLATE)
                    source = virXMLPropString(cur, "name");
            } else if ((target == NULL) &&
                       (xmlStrEqual(cur->name, BAD_CAST "target"))) {
                target = virXMLPropString(cur, "dir");
            } else if (xmlStrEqual(cur->name, BAD_CAST "readonly")) {
                def->readonly = 1;
            }
        }
        cur = cur->next;
    }

    if (source == NULL) {
        virDomainReportError(VIR_ERR_NO_SOURCE,
                             target ? "%s" : NULL, target);
        goto error;
    }

    if (target == NULL) {
        virDomainReportError(VIR_ERR_NO_TARGET,
                             source ? "%s" : NULL, source);
        goto error;
    }

    def->src = source;
    source = NULL;
    def->dst = target;
    target = NULL;

    if (virDomainDeviceInfoParseXML(node, &def->info, flags) < 0)
        goto error;

cleanup:
    VIR_FREE(type);
    VIR_FREE(target);
    VIR_FREE(source);
    VIR_FREE(accessmode);

    return def;

 error:
    virDomainFSDefFree(def);
    def = NULL;
    goto cleanup;
}


static int
virVirtualPortProfileParamsParseXML(xmlNodePtr node,
                                    virVirtualPortProfileParamsPtr virtPort)
{
    int ret = -1;
    char *virtPortType;
    char *virtPortManagerID = NULL;
    char *virtPortTypeID = NULL;
    char *virtPortTypeIDVersion = NULL;
    char *virtPortInstanceID = NULL;
    char *virtPortProfileID = NULL;
    xmlNodePtr cur = node->children;
    const char *msg = NULL;

    virtPortType = virXMLPropString(node, "type");
    if (!virtPortType)
        return -1;

    while (cur != NULL) {
        if (xmlStrEqual(cur->name, BAD_CAST "parameters")) {

            virtPortManagerID = virXMLPropString(cur, "managerid");
            virtPortTypeID = virXMLPropString(cur, "typeid");
            virtPortTypeIDVersion = virXMLPropString(cur, "typeidversion");
            virtPortInstanceID = virXMLPropString(cur, "instanceid");
            virtPortProfileID = virXMLPropString(cur, "profileid");

            break;
        }

        cur = cur->next;
    }

    virtPort->virtPortType = VIR_VIRTUALPORT_NONE;

    switch (virVirtualPortTypeFromString(virtPortType)) {

    case VIR_VIRTUALPORT_8021QBG:
        if (virtPortManagerID     != NULL && virtPortTypeID     != NULL &&
            virtPortTypeIDVersion != NULL) {
            unsigned int val;

            if (virStrToLong_ui(virtPortManagerID, NULL, 0, &val)) {
                msg = _("cannot parse value of managerid parameter");
                goto err_exit;
            }

            if (val > 0xff) {
                msg = _("value of managerid out of range");
                goto err_exit;
            }

            virtPort->u.virtPort8021Qbg.managerID = (uint8_t)val;

            if (virStrToLong_ui(virtPortTypeID, NULL, 0, &val)) {
                msg = _("cannot parse value of typeid parameter");
                goto err_exit;
            }

            if (val > 0xffffff) {
                msg = _("value for typeid out of range");
                goto err_exit;
            }

            virtPort->u.virtPort8021Qbg.typeID = (uint32_t)val;

            if (virStrToLong_ui(virtPortTypeIDVersion, NULL, 0, &val)) {
                msg = _("cannot parse value of typeidversion parameter");
                goto err_exit;
            }

            if (val > 0xff) {
                msg = _("value of typeidversion out of range");
                goto err_exit;
            }

            virtPort->u.virtPort8021Qbg.typeIDVersion = (uint8_t)val;

            if (virtPortInstanceID != NULL) {
                if (virUUIDParse(virtPortInstanceID,
                                 virtPort->u.virtPort8021Qbg.instanceID)) {
                    msg = _("cannot parse instanceid parameter as a uuid");
                    goto err_exit;
                }
            } else {
                if (virUUIDGenerate(virtPort->u.virtPort8021Qbg.instanceID)) {
                    msg = _("cannot generate a random uuid for instanceid");
                    goto err_exit;
                }
            }

            virtPort->virtPortType = VIR_VIRTUALPORT_8021QBG;
            ret = 0;
        } else {
            msg = _("a parameter is missing for 802.1Qbg description");
            goto err_exit;
        }
    break;

    case VIR_VIRTUALPORT_8021QBH:
        if (virtPortProfileID != NULL) {
            if (virStrcpyStatic(virtPort->u.virtPort8021Qbh.profileID,
                                virtPortProfileID) != NULL) {
                virtPort->virtPortType = VIR_VIRTUALPORT_8021QBH;
                ret = 0;
            } else {
                msg = _("profileid parameter too long");
                goto err_exit;
            }
        } else {
            msg = _("profileid parameter is missing for 802.1Qbh descripion");
            goto err_exit;
        }
    break;


    default:
    case VIR_VIRTUALPORT_NONE:
    case VIR_VIRTUALPORT_TYPE_LAST:
        msg = _("unknown virtualport type");
        goto err_exit;
    break;
    }

err_exit:

    if (msg)
        virDomainReportError(VIR_ERR_INTERNAL_ERROR, "%s", msg);

    VIR_FREE(virtPortManagerID);
    VIR_FREE(virtPortTypeID);
    VIR_FREE(virtPortTypeIDVersion);
    VIR_FREE(virtPortInstanceID);
    VIR_FREE(virtPortProfileID);
    VIR_FREE(virtPortType);

    return ret;
}


/* Parse the XML definition for a network interface
 * @param node XML nodeset to parse for net definition
 * @return 0 on success, -1 on failure
 */
static virDomainNetDefPtr
virDomainNetDefParseXML(virCapsPtr caps,
                        xmlNodePtr node,
                        xmlXPathContextPtr ctxt,
                        virBitmapPtr bootMap,
                        int flags ATTRIBUTE_UNUSED)
{
    virDomainNetDefPtr def;
    xmlNodePtr cur;
    char *macaddr = NULL;
    char *type = NULL;
    char *network = NULL;
    char *bridge = NULL;
    char *dev = NULL;
    char *ifname = NULL;
    char *script = NULL;
    char *address = NULL;
    char *port = NULL;
    char *model = NULL;
    char *backend = NULL;
    char *filter = NULL;
    char *internal = NULL;
    char *devaddr = NULL;
    char *mode = NULL;
    virNWFilterHashTablePtr filterparams = NULL;
    virVirtualPortProfileParams virtPort;
    bool virtPortParsed = false;
    xmlNodePtr oldnode = ctxt->node;
    int ret;

    if (VIR_ALLOC(def) < 0) {
        virReportOOMError();
        return NULL;
    }

    ctxt->node = node;

    type = virXMLPropString(node, "type");
    if (type != NULL) {
        if ((int)(def->type = virDomainNetTypeFromString(type)) < 0) {
            virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                                 _("unknown interface type '%s'"), type);
            goto error;
        }
    } else {
        def->type = VIR_DOMAIN_NET_TYPE_USER;
    }

    cur = node->children;
    while (cur != NULL) {
        if (cur->type == XML_ELEMENT_NODE) {
            if ((macaddr == NULL) &&
                (xmlStrEqual(cur->name, BAD_CAST "mac"))) {
                macaddr = virXMLPropString(cur, "address");
            } else if ((network == NULL) &&
                       (def->type == VIR_DOMAIN_NET_TYPE_NETWORK) &&
                       (xmlStrEqual(cur->name, BAD_CAST "source"))) {
                network = virXMLPropString(cur, "network");
            } else if ((internal == NULL) &&
                       (def->type == VIR_DOMAIN_NET_TYPE_INTERNAL) &&
                       (xmlStrEqual(cur->name, BAD_CAST "source"))) {
                internal = virXMLPropString(cur, "name");
            } else if ((network == NULL) &&
                       (def->type == VIR_DOMAIN_NET_TYPE_BRIDGE) &&
                       (xmlStrEqual(cur->name, BAD_CAST "source"))) {
                bridge = virXMLPropString(cur, "bridge");
            } else if ((dev == NULL) &&
                       (def->type == VIR_DOMAIN_NET_TYPE_ETHERNET ||
                        def->type == VIR_DOMAIN_NET_TYPE_DIRECT) &&
                       xmlStrEqual(cur->name, BAD_CAST "source")) {
                dev  = virXMLPropString(cur, "dev");
                mode = virXMLPropString(cur, "mode");
            } else if ((virtPortParsed == false) &&
                       (def->type == VIR_DOMAIN_NET_TYPE_DIRECT) &&
                       xmlStrEqual(cur->name, BAD_CAST "virtualport")) {
                if (virVirtualPortProfileParamsParseXML(cur, &virtPort))
                    goto error;
                virtPortParsed = true;
            } else if ((network == NULL) &&
                       ((def->type == VIR_DOMAIN_NET_TYPE_SERVER) ||
                        (def->type == VIR_DOMAIN_NET_TYPE_CLIENT) ||
                        (def->type == VIR_DOMAIN_NET_TYPE_MCAST)) &&
                       (xmlStrEqual(cur->name, BAD_CAST "source"))) {
                address = virXMLPropString(cur, "address");
                port = virXMLPropString(cur, "port");
            } else if ((address == NULL) &&
                       (def->type == VIR_DOMAIN_NET_TYPE_ETHERNET ||
                        def->type == VIR_DOMAIN_NET_TYPE_BRIDGE) &&
                       (xmlStrEqual(cur->name, BAD_CAST "ip"))) {
                address = virXMLPropString(cur, "address");
            } else if ((ifname == NULL) &&
                       xmlStrEqual(cur->name, BAD_CAST "target")) {
                ifname = virXMLPropString(cur, "dev");
                if ((ifname != NULL) &&
                    ((flags & VIR_DOMAIN_XML_INACTIVE) &&
                      (STRPREFIX((const char*)ifname, "vnet")))) {
                    /* An auto-generated target name, blank it out */
                    VIR_FREE(ifname);
                }
            } else if ((script == NULL) &&
                       (def->type == VIR_DOMAIN_NET_TYPE_ETHERNET ||
                        def->type == VIR_DOMAIN_NET_TYPE_BRIDGE) &&
                       xmlStrEqual(cur->name, BAD_CAST "script")) {
                script = virXMLPropString(cur, "path");
            } else if (xmlStrEqual (cur->name, BAD_CAST "model")) {
                model = virXMLPropString(cur, "type");
            } else if (xmlStrEqual (cur->name, BAD_CAST "driver")) {
                backend = virXMLPropString(cur, "name");
            } else if (xmlStrEqual (cur->name, BAD_CAST "filterref")) {
                filter = virXMLPropString(cur, "filter");
                VIR_FREE(filterparams);
                filterparams = virNWFilterParseParamAttributes(cur);
            } else if ((flags & VIR_DOMAIN_XML_INTERNAL_STATUS) &&
                       xmlStrEqual(cur->name, BAD_CAST "state")) {
                /* Legacy back-compat. Don't add any more attributes here */
                devaddr = virXMLPropString(cur, "devaddr");
            } else if (xmlStrEqual(cur->name, BAD_CAST "boot")) {
                if (virDomainDeviceBootParseXML(cur, &def->bootIndex,
                                                bootMap))
                    goto error;
            }
        }
        cur = cur->next;
    }

    if (macaddr) {
        if (virParseMacAddr((const char *)macaddr, def->mac) < 0) {
            virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                                 _("unable to parse mac address '%s'"),
                                 (const char *)macaddr);
            goto error;
        }
    } else {
        virCapabilitiesGenerateMac(caps, def->mac);
    }

    if (devaddr) {
        if (virDomainParseLegacyDeviceAddress(devaddr,
                                              &def->info.addr.pci) < 0) {
            virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                                 _("Unable to parse devaddr parameter '%s'"),
                                 devaddr);
            goto error;
        }
        def->info.type = VIR_DOMAIN_DEVICE_ADDRESS_TYPE_PCI;
    } else {
        if (virDomainDeviceInfoParseXML(node, &def->info, flags) < 0)
            goto error;
    }

    /* XXX what about ISA/USB based NIC models - once we support
     * them we should make sure address type is correct */
    if (def->info.type != VIR_DOMAIN_DEVICE_ADDRESS_TYPE_NONE &&
        def->info.type != VIR_DOMAIN_DEVICE_ADDRESS_TYPE_PCI) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR, "%s",
                             _("Network interfaces must use 'pci' address type"));
        goto error;
    }

    switch (def->type) {
    case VIR_DOMAIN_NET_TYPE_NETWORK:
        if (network == NULL) {
            virDomainReportError(VIR_ERR_INTERNAL_ERROR, "%s",
    _("No <source> 'network' attribute specified with <interface type='network'/>"));
            goto error;
        }
        def->data.network.name = network;
        network = NULL;
        break;

    case VIR_DOMAIN_NET_TYPE_ETHERNET:

        if (script != NULL) {
            def->data.ethernet.script = script;
            script = NULL;
        }
        if (dev != NULL) {
            def->data.ethernet.dev = dev;
            dev = NULL;
        }
        if (address != NULL) {
            def->data.ethernet.ipaddr = address;
            address = NULL;
        }
        break;

    case VIR_DOMAIN_NET_TYPE_BRIDGE:
        if (bridge == NULL) {
            virDomainReportError(VIR_ERR_INTERNAL_ERROR, "%s",
    _("No <source> 'bridge' attribute specified with <interface type='bridge'/>"));
            goto error;
        }
        def->data.bridge.brname = bridge;
        bridge = NULL;
        if (script != NULL) {
            def->data.bridge.script = script;
            script = NULL;
        }
        if (address != NULL) {
            def->data.bridge.ipaddr = address;
            address = NULL;
        }
        break;

    case VIR_DOMAIN_NET_TYPE_CLIENT:
    case VIR_DOMAIN_NET_TYPE_SERVER:
    case VIR_DOMAIN_NET_TYPE_MCAST:
        if (port == NULL) {
            virDomainReportError(VIR_ERR_INTERNAL_ERROR, "%s",
            _("No <source> 'port' attribute specified with socket interface"));
            goto error;
        }
        if (virStrToLong_i(port, NULL, 10, &def->data.socket.port) < 0) {
            virDomainReportError(VIR_ERR_INTERNAL_ERROR, "%s",
            _("Cannot parse <source> 'port' attribute with socket interface"));
            goto error;
        }

        if (address == NULL) {
            if (def->type == VIR_DOMAIN_NET_TYPE_CLIENT ||
                def->type == VIR_DOMAIN_NET_TYPE_MCAST) {
                virDomainReportError(VIR_ERR_INTERNAL_ERROR, "%s",
        _("No <source> 'address' attribute specified with socket interface"));
                goto error;
            }
        } else {
            def->data.socket.address = address;
            address = NULL;
        }
        break;

    case VIR_DOMAIN_NET_TYPE_INTERNAL:
        if (internal == NULL) {
            virDomainReportError(VIR_ERR_INTERNAL_ERROR, "%s",
        _("No <source> 'name' attribute specified with <interface type='internal'/>"));
            goto error;
        }
        def->data.internal.name = internal;
        internal = NULL;
        break;

    case VIR_DOMAIN_NET_TYPE_DIRECT:
        if (dev == NULL) {
            virDomainReportError(VIR_ERR_INTERNAL_ERROR, "%s",
        _("No <source> 'dev' attribute specified with <interface type='direct'/>"));
            goto error;
        }

        if (mode != NULL) {
            int m;
            if ((m = virDomainNetdevMacvtapTypeFromString(mode)) < 0) {
                virDomainReportError(VIR_ERR_INTERNAL_ERROR, "%s",
                                     _("Unkown mode has been specified"));
                goto error;
            }
            def->data.direct.mode = m;
        } else
            def->data.direct.mode = VIR_DOMAIN_NETDEV_MACVTAP_MODE_VEPA;

        if (virtPortParsed)
            def->data.direct.virtPortProfile = virtPort;

        def->data.direct.linkdev = dev;
        dev = NULL;

        if ((flags & VIR_DOMAIN_XML_INACTIVE))
            VIR_FREE(ifname);

        break;

    case VIR_DOMAIN_NET_TYPE_USER:
    case VIR_DOMAIN_NET_TYPE_LAST:
        break;
    }

    if (ifname != NULL) {
        def->ifname = ifname;
        ifname = NULL;
    }

    /* NIC model (see -net nic,model=?).  We only check that it looks
     * reasonable, not that it is a supported NIC type.  FWIW kvm
     * supports these types as of April 2008:
     * i82551 i82557b i82559er ne2k_pci pcnet rtl8139 e1000 virtio
     */
    if (model != NULL) {
        int i;
        for (i = 0 ; i < strlen(model) ; i++) {
            int char_ok = c_isalnum(model[i]) || model[i] == '_';
            if (!char_ok) {
                virDomainReportError(VIR_ERR_INVALID_ARG, "%s",
                                     _("Model name contains invalid characters"));
                goto error;
            }
        }
        def->model = model;
        model = NULL;
    }

    if ((backend != NULL) &&
        (def->model && STREQ(def->model, "virtio"))) {
        int b;
        if (((b = virDomainNetBackendTypeFromString(backend)) < 0) ||
            (b == VIR_DOMAIN_NET_BACKEND_TYPE_DEFAULT)) {
            virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                                 _("Unknown interface <driver name='%s'> "
                                   "has been specified"),
                                 backend);
            goto error;
        }
        def->backend = b;
    }
    if (filter != NULL) {
        switch (def->type) {
        case VIR_DOMAIN_NET_TYPE_ETHERNET:
        case VIR_DOMAIN_NET_TYPE_NETWORK:
        case VIR_DOMAIN_NET_TYPE_BRIDGE:
        case VIR_DOMAIN_NET_TYPE_DIRECT:
            def->filter = filter;
            filter = NULL;
            def->filterparams = filterparams;
            filterparams = NULL;
        break;
        default:
        break;
        }
    }

    ret = virXPathULong("string(./tune/sndbuf)", ctxt, &def->tune.sndbuf);
    if (ret >= 0) {
        def->tune.sndbuf_specified = true;
    } else if (ret == -2) {
        virDomainReportError(VIR_ERR_XML_ERROR, "%s",
                             _("sndbuf must be a positive integer"));
        goto error;
    }

cleanup:
    ctxt->node = oldnode;
    VIR_FREE(macaddr);
    VIR_FREE(network);
    VIR_FREE(address);
    VIR_FREE(port);
    VIR_FREE(ifname);
    VIR_FREE(dev);
    VIR_FREE(script);
    VIR_FREE(bridge);
    VIR_FREE(model);
    VIR_FREE(backend);
    VIR_FREE(filter);
    VIR_FREE(type);
    VIR_FREE(internal);
    VIR_FREE(devaddr);
    VIR_FREE(mode);
    virNWFilterHashTableFree(filterparams);

    return def;

error:
    virDomainNetDefFree(def);
    def = NULL;
    goto cleanup;
}

static int
virDomainChrDefaultTargetType(virCapsPtr caps, int devtype) {

    int target = -1;

    switch (devtype) {
    case VIR_DOMAIN_CHR_DEVICE_TYPE_CHANNEL:
        virDomainReportError(VIR_ERR_XML_ERROR,
                             _("target type must be specified for %s device"),
                             virDomainChrDeviceTypeToString(devtype));
        break;

    case VIR_DOMAIN_CHR_DEVICE_TYPE_CONSOLE:
        target = caps->defaultConsoleTargetType;
        break;

    case VIR_DOMAIN_CHR_DEVICE_TYPE_SERIAL:
    case VIR_DOMAIN_CHR_DEVICE_TYPE_PARALLEL:
    default:
        /* No target type yet*/
        target = 0;
        break;
    }

    return target;
}

static int
virDomainChrTargetTypeFromString(virCapsPtr caps,
                                 int devtype,
                                 const char *targetType)
{
    int ret = -1;
    int target = 0;

    if (!targetType) {
        target = virDomainChrDefaultTargetType(caps, devtype);
        goto out;
    }

    switch (devtype) {
    case VIR_DOMAIN_CHR_DEVICE_TYPE_CHANNEL:
        target = virDomainChrChannelTargetTypeFromString(targetType);
        break;

    case VIR_DOMAIN_CHR_DEVICE_TYPE_CONSOLE:
        target = virDomainChrConsoleTargetTypeFromString(targetType);
        break;

    case VIR_DOMAIN_CHR_DEVICE_TYPE_SERIAL:
    case VIR_DOMAIN_CHR_DEVICE_TYPE_PARALLEL:
    default:
        /* No target type yet*/
        break;
    }

out:
    ret = target;
    return ret;
}

static int
virDomainChrDefParseTargetXML(virCapsPtr caps,
                              virDomainChrDefPtr def,
                              xmlNodePtr cur,
                              int flags ATTRIBUTE_UNUSED)
{
    int ret = -1;
    unsigned int port;
    const char *targetType = virXMLPropString(cur, "type");
    const char *addrStr = NULL;
    const char *portStr = NULL;

    if ((def->targetType =
        virDomainChrTargetTypeFromString(caps,
                                         def->deviceType, targetType)) < 0) {
        goto error;
    }

    switch (def->deviceType) {
    case VIR_DOMAIN_CHR_DEVICE_TYPE_CHANNEL:
        switch (def->targetType) {
        case VIR_DOMAIN_CHR_CHANNEL_TARGET_TYPE_GUESTFWD:
            addrStr = virXMLPropString(cur, "address");
            portStr = virXMLPropString(cur, "port");

            if (addrStr == NULL) {
                virDomainReportError(VIR_ERR_XML_ERROR, "%s",
                                     _("guestfwd channel does not "
                                       "define a target address"));
                goto error;
            }

            if (VIR_ALLOC(def->target.addr) < 0) {
                virReportOOMError();
                goto error;
            }

            if (virSocketParseAddr(addrStr, def->target.addr, AF_UNSPEC) < 0)
                goto error;

            if (def->target.addr->data.stor.ss_family != AF_INET) {
                virDomainReportError(VIR_ERR_CONFIG_UNSUPPORTED,
                                     "%s", _("guestfwd channel only supports "
                                             "IPv4 addresses"));
                goto error;
            }

            if (portStr == NULL) {
                virDomainReportError(VIR_ERR_XML_ERROR, "%s",
                                     _("guestfwd channel does "
                                       "not define a target port"));
                goto error;
            }

            if (virStrToLong_ui(portStr, NULL, 10, &port) < 0) {
                virDomainReportError(VIR_ERR_XML_ERROR,
                                     _("Invalid port number: %s"),
                                     portStr);
                goto error;
            }

            virSocketSetPort(def->target.addr, port);
            break;

        case VIR_DOMAIN_CHR_CHANNEL_TARGET_TYPE_VIRTIO:
            def->target.name = virXMLPropString(cur, "name");
            break;
        }
        break;

    default:
        portStr = virXMLPropString(cur, "port");
        if (portStr == NULL) {
            /* Not required. It will be assigned automatically later */
            break;
        }

        if (virStrToLong_ui(portStr, NULL, 10, &port) < 0) {
            virDomainReportError(VIR_ERR_XML_ERROR,
                                 _("Invalid port number: %s"),
                                 portStr);
            goto error;
        }
        break;
    }


    ret = 0;
error:
    VIR_FREE(targetType);
    VIR_FREE(addrStr);
    VIR_FREE(portStr);

    return ret;
}

/* Parse the source half of the XML definition for a character device,
 * where node is the first element of node->children of the parent
 * element.  def->type must already be valid.  Return -1 on failure,
 * otherwise the number of ignored children (this intentionally skips
 * <target>, which is used by <serial> but not <smartcard>). */
static int
virDomainChrSourceDefParseXML(virDomainChrSourceDefPtr def,
                              xmlNodePtr cur)
{
    char *bindHost = NULL;
    char *bindService = NULL;
    char *connectHost = NULL;
    char *connectService = NULL;
    char *path = NULL;
    char *mode = NULL;
    char *protocol = NULL;
    int remaining = 0;

    while (cur != NULL) {
        if (cur->type == XML_ELEMENT_NODE) {
            if (xmlStrEqual(cur->name, BAD_CAST "source")) {
                if (mode == NULL)
                    mode = virXMLPropString(cur, "mode");

                switch (def->type) {
                case VIR_DOMAIN_CHR_TYPE_PTY:
                case VIR_DOMAIN_CHR_TYPE_DEV:
                case VIR_DOMAIN_CHR_TYPE_FILE:
                case VIR_DOMAIN_CHR_TYPE_PIPE:
                case VIR_DOMAIN_CHR_TYPE_UNIX:
                    if (path == NULL)
                        path = virXMLPropString(cur, "path");

                    break;

                case VIR_DOMAIN_CHR_TYPE_UDP:
                case VIR_DOMAIN_CHR_TYPE_TCP:
                    if (mode == NULL ||
                        STREQ((const char *)mode, "connect")) {

                        if (connectHost == NULL)
                            connectHost = virXMLPropString(cur, "host");
                        if (connectService == NULL)
                            connectService = virXMLPropString(cur, "service");
                    } else if (STREQ((const char *)mode, "bind")) {
                        if (bindHost == NULL)
                            bindHost = virXMLPropString(cur, "host");
                        if (bindService == NULL)
                            bindService = virXMLPropString(cur, "service");
                    } else {
                        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                                             _("Unknown source mode '%s'"),
                                             mode);
                        goto error;
                    }

                    if (def->type == VIR_DOMAIN_CHR_TYPE_UDP)
                        VIR_FREE(mode);
                }
            } else if (xmlStrEqual(cur->name, BAD_CAST "protocol")) {
                if (protocol == NULL)
                    protocol = virXMLPropString(cur, "type");
            } else {
                remaining++;
            }
        }
        cur = cur->next;
    }

    switch (def->type) {
    case VIR_DOMAIN_CHR_TYPE_NULL:
        /* Nada */
        break;

    case VIR_DOMAIN_CHR_TYPE_VC:
        break;

    case VIR_DOMAIN_CHR_TYPE_PTY:
    case VIR_DOMAIN_CHR_TYPE_DEV:
    case VIR_DOMAIN_CHR_TYPE_FILE:
    case VIR_DOMAIN_CHR_TYPE_PIPE:
        if (path == NULL &&
            def->type != VIR_DOMAIN_CHR_TYPE_PTY) {
            virDomainReportError(VIR_ERR_INTERNAL_ERROR, "%s",
                       _("Missing source path attribute for char device"));
            goto error;
        }

        def->data.file.path = path;
        path = NULL;
        break;

    case VIR_DOMAIN_CHR_TYPE_STDIO:
    case VIR_DOMAIN_CHR_TYPE_SPICEVMC:
        /* Nada */
        break;

    case VIR_DOMAIN_CHR_TYPE_TCP:
        if (mode == NULL ||
            STREQ(mode, "connect")) {
            if (connectHost == NULL) {
                virDomainReportError(VIR_ERR_INTERNAL_ERROR, "%s",
                        _("Missing source host attribute for char device"));
                goto error;
            }
            if (connectService == NULL) {
                virDomainReportError(VIR_ERR_INTERNAL_ERROR, "%s",
                     _("Missing source service attribute for char device"));
                goto error;
            }

            def->data.tcp.host = connectHost;
            connectHost = NULL;
            def->data.tcp.service = connectService;
            connectService = NULL;
            def->data.tcp.listen = false;
        } else {
            if (bindHost == NULL) {
                virDomainReportError(VIR_ERR_INTERNAL_ERROR, "%s",
                        _("Missing source host attribute for char device"));
                goto error;
            }
            if (bindService == NULL) {
                virDomainReportError(VIR_ERR_INTERNAL_ERROR, "%s",
                     _("Missing source service attribute for char device"));
                goto error;
            }

            def->data.tcp.host = bindHost;
            bindHost = NULL;
            def->data.tcp.service = bindService;
            bindService = NULL;
            def->data.tcp.listen = true;
        }

        if (protocol == NULL)
            def->data.tcp.protocol = VIR_DOMAIN_CHR_TCP_PROTOCOL_RAW;
        else if ((def->data.tcp.protocol =
                  virDomainChrTcpProtocolTypeFromString(protocol)) < 0) {
            virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                                 _("Unknown protocol '%s'"), protocol);
            goto error;
        }

        break;

    case VIR_DOMAIN_CHR_TYPE_UDP:
        if (connectService == NULL) {
            virDomainReportError(VIR_ERR_INTERNAL_ERROR, "%s",
                   _("Missing source service attribute for char device"));
            goto error;
        }

        def->data.udp.connectHost = connectHost;
        connectHost = NULL;
        def->data.udp.connectService = connectService;
        connectService = NULL;

        def->data.udp.bindHost = bindHost;
        bindHost = NULL;
        def->data.udp.bindService = bindService;
        bindService = NULL;
        break;

    case VIR_DOMAIN_CHR_TYPE_UNIX:
        if (path == NULL) {
            virDomainReportError(VIR_ERR_INTERNAL_ERROR, "%s",
                         _("Missing source path attribute for char device"));
            goto error;
        }

        def->data.nix.listen = mode != NULL && STRNEQ(mode, "connect");

        def->data.nix.path = path;
        path = NULL;
        break;
    }

cleanup:
    VIR_FREE(mode);
    VIR_FREE(protocol);
    VIR_FREE(bindHost);
    VIR_FREE(bindService);
    VIR_FREE(connectHost);
    VIR_FREE(connectService);
    VIR_FREE(path);

    return remaining;

error:
    virDomainChrSourceDefClear(def);
    remaining = -1;
    goto cleanup;
}

/* Parse the XML definition for a character device
 * @param node XML nodeset to parse for net definition
 *
 * The XML we're dealing with looks like
 *
 * <serial type="pty">
 *   <source path="/dev/pts/3"/>
 *   <target port="1"/>
 * </serial>
 *
 * <serial type="dev">
 *   <source path="/dev/ttyS0"/>
 *   <target port="1"/>
 * </serial>
 *
 * <serial type="tcp">
 *   <source mode="connect" host="0.0.0.0" service="2445"/>
 *   <target port="1"/>
 * </serial>
 *
 * <serial type="tcp">
 *   <source mode="bind" host="0.0.0.0" service="2445"/>
 *   <target port="1"/>
 *   <protocol type='raw'/>
 * </serial>
 *
 * <serial type="udp">
 *   <source mode="bind" host="0.0.0.0" service="2445"/>
 *   <source mode="connect" host="0.0.0.0" service="2445"/>
 *   <target port="1"/>
 * </serial>
 *
 * <serial type="unix">
 *   <source mode="bind" path="/tmp/foo"/>
 *   <target port="1"/>
 * </serial>
 *
 */
static virDomainChrDefPtr
virDomainChrDefParseXML(virCapsPtr caps,
                        xmlNodePtr node,
                        int flags) {
    xmlNodePtr cur;
    char *type = NULL;
    const char *nodeName;
    virDomainChrDefPtr def;
    int remaining;

    if (VIR_ALLOC(def) < 0) {
        virReportOOMError();
        return NULL;
    }

    type = virXMLPropString(node, "type");
    if (type == NULL) {
        def->source.type = VIR_DOMAIN_CHR_TYPE_PTY;
    } else if ((def->source.type = virDomainChrTypeFromString(type)) < 0) {
        virDomainReportError(VIR_ERR_XML_ERROR,
                             _("unknown type presented to host for character device: %s"),
                             type);
        goto error;
    }

    nodeName = (const char *) node->name;
    if ((def->deviceType = virDomainChrDeviceTypeFromString(nodeName)) < 0) {
        virDomainReportError(VIR_ERR_XML_ERROR,
                             _("unknown character device type: %s"),
                             nodeName);
    }

    cur = node->children;
    remaining = virDomainChrSourceDefParseXML(&def->source, cur);
    if (remaining < 0)
        goto error;
    if (remaining) {
        while (cur != NULL) {
            if (cur->type == XML_ELEMENT_NODE) {
                if (xmlStrEqual(cur->name, BAD_CAST "target")) {
                    if (virDomainChrDefParseTargetXML(caps, def, cur,
                                                      flags) < 0) {
                        goto error;
                    }
                }
            }
            cur = cur->next;
        }
    }

    if (def->source.type == VIR_DOMAIN_CHR_TYPE_SPICEVMC) {
        if (def->targetType != VIR_DOMAIN_CHR_CHANNEL_TARGET_TYPE_VIRTIO) {
            virDomainReportError(VIR_ERR_CONFIG_UNSUPPORTED, "%s",
                                 _("spicevmc device type only supports "
                                   "virtio"));
            goto error;
        } else {
            def->source.data.spicevmc = VIR_DOMAIN_CHR_SPICEVMC_VDAGENT;
        }
    }

    if (virDomainDeviceInfoParseXML(node, &def->info, flags) < 0)
        goto error;

cleanup:
    VIR_FREE(type);

    return def;

error:
    virDomainChrDefFree(def);
    def = NULL;
    goto cleanup;
}

static virDomainSmartcardDefPtr
virDomainSmartcardDefParseXML(xmlNodePtr node,
                              int flags)
{
    xmlNodePtr cur;
    char *mode = NULL;
    char *type = NULL;
    virDomainSmartcardDefPtr def;
    int i;

    if (VIR_ALLOC(def) < 0) {
        virReportOOMError();
        return NULL;
    }

    mode = virXMLPropString(node, "mode");
    if (mode == NULL) {
        virDomainReportError(VIR_ERR_XML_ERROR, "%s",
                             _("missing smartcard device mode"));
        goto error;
    }
    if ((def->type = virDomainSmartcardTypeFromString(mode)) < 0) {
        virDomainReportError(VIR_ERR_XML_ERROR,
                             _("unknown smartcard device mode: %s"),
                             mode);
        goto error;
    }

    switch (def->type) {
    case VIR_DOMAIN_SMARTCARD_TYPE_HOST:
        break;

    case VIR_DOMAIN_SMARTCARD_TYPE_HOST_CERTIFICATES:
        i = 0;
        cur = node->children;
        while (cur) {
            if (cur->type == XML_ELEMENT_NODE &&
                xmlStrEqual(cur->name, BAD_CAST "certificate")) {
                if (i == 3) {
                    virDomainReportError(VIR_ERR_XML_ERROR, "%s",
                                         _("host-certificates mode needs "
                                           "exactly three certificates"));
                    goto error;
                }
                def->data.cert.file[i] = (char *)xmlNodeGetContent(cur);
                if (!def->data.cert.file[i]) {
                    virReportOOMError();
                    goto error;
                }
                i++;
            } else if (cur->type == XML_ELEMENT_NODE &&
                       xmlStrEqual(cur->name, BAD_CAST "database") &&
                       !def->data.cert.database) {
                def->data.cert.database = (char *)xmlNodeGetContent(cur);
                if (!def->data.cert.database) {
                    virReportOOMError();
                    goto error;
                }
                if (*def->data.cert.database != '/') {
                    virDomainReportError(VIR_ERR_XML_ERROR,
                                         _("expecting absolute path: %s"),
                                         def->data.cert.database);
                    goto error;
                }
            }
            cur = cur->next;
        }
        if (i < 3) {
            virDomainReportError(VIR_ERR_XML_ERROR, "%s",
                                 _("host-certificates mode needs "
                                   "exactly three certificates"));
            goto error;
        }
        break;

    case VIR_DOMAIN_SMARTCARD_TYPE_PASSTHROUGH:
        type = virXMLPropString(node, "type");
        if (type == NULL) {
            virDomainReportError(VIR_ERR_XML_ERROR, "%s",
                                 _("passthrough mode requires a character "
                                   "device type attribute"));
            goto error;
        }
        if ((def->data.passthru.type = virDomainChrTypeFromString(type)) < 0) {
            virDomainReportError(VIR_ERR_XML_ERROR,
                                 _("unknown type presented to host for "
                                   "character device: %s"), type);
            goto error;
        }

        cur = node->children;
        if (virDomainChrSourceDefParseXML(&def->data.passthru, cur) < 0)
            goto error;

        if (def->data.passthru.type == VIR_DOMAIN_CHR_TYPE_SPICEVMC) {
            def->data.passthru.data.spicevmc
                = VIR_DOMAIN_CHR_SPICEVMC_SMARTCARD;
        }

        break;

    default:
        virDomainReportError(VIR_ERR_INTERNAL_ERROR, "%s",
                             _("unknown smartcard mode"));
        goto error;
    }

    if (virDomainDeviceInfoParseXML(node, &def->info, flags) < 0)
        goto error;
    if (def->info.type != VIR_DOMAIN_DEVICE_ADDRESS_TYPE_NONE &&
        def->info.type != VIR_DOMAIN_DEVICE_ADDRESS_TYPE_CCID) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR, "%s",
                             _("Controllers must use the 'ccid' address type"));
        goto error;
    }

cleanup:
    VIR_FREE(mode);
    VIR_FREE(type);

    return def;

error:
    virDomainSmartcardDefFree(def);
    def = NULL;
    goto cleanup;
}

/* Parse the XML definition for a network interface */
static virDomainInputDefPtr
virDomainInputDefParseXML(const char *ostype,
                          xmlNodePtr node,
                          int flags) {
    virDomainInputDefPtr def;
    char *type = NULL;
    char *bus = NULL;

    if (VIR_ALLOC(def) < 0) {
        virReportOOMError();
        return NULL;
    }

    type = virXMLPropString(node, "type");
    bus = virXMLPropString(node, "bus");

    if (!type) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                             "%s", _("missing input device type"));
        goto error;
    }

    if ((def->type = virDomainInputTypeFromString(type)) < 0) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                             _("unknown input device type '%s'"), type);
        goto error;
    }

    if (bus) {
        if ((def->bus = virDomainInputBusTypeFromString(bus)) < 0) {
            virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                                 _("unknown input bus type '%s'"), bus);
            goto error;
        }

        if (STREQ(ostype, "hvm")) {
            if (def->bus == VIR_DOMAIN_INPUT_BUS_PS2 && /* Only allow mouse for ps2 */
                def->type != VIR_DOMAIN_INPUT_TYPE_MOUSE) {
                virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                                 _("ps2 bus does not support %s input device"),
                                     type);
                goto error;
            }
            if (def->bus == VIR_DOMAIN_INPUT_BUS_XEN) {
                virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                                     _("unsupported input bus %s"),
                                     bus);
                goto error;
            }
        } else {
            if (def->bus != VIR_DOMAIN_INPUT_BUS_XEN) {
                virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                                     _("unsupported input bus %s"),
                                     bus);
            }
            if (def->type != VIR_DOMAIN_INPUT_TYPE_MOUSE) {
                virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                                 _("xen bus does not support %s input device"),
                                     type);
                goto error;
            }
        }
    } else {
        if (STREQ(ostype, "hvm")) {
            if (def->type == VIR_DOMAIN_INPUT_TYPE_MOUSE)
                def->bus = VIR_DOMAIN_INPUT_BUS_PS2;
            else
                def->bus = VIR_DOMAIN_INPUT_BUS_USB;
        } else {
            def->bus = VIR_DOMAIN_INPUT_BUS_XEN;
        }
    }

    if (virDomainDeviceInfoParseXML(node, &def->info, flags) < 0)
        goto error;

cleanup:
    VIR_FREE(type);
    VIR_FREE(bus);

    return def;

error:
    virDomainInputDefFree(def);
    def = NULL;
    goto cleanup;
}


/* Parse the XML definition for a clock timer */
static virDomainTimerDefPtr
virDomainTimerDefParseXML(const xmlNodePtr node,
                          xmlXPathContextPtr ctxt,
                          int flags ATTRIBUTE_UNUSED)
{
    char *name = NULL;
    char *present = NULL;
    char *tickpolicy = NULL;
    char *track = NULL;
    char *mode = NULL;

    virDomainTimerDefPtr def;
    xmlNodePtr oldnode = ctxt->node;

    if (VIR_ALLOC(def) < 0) {
        virReportOOMError();
        return NULL;
    }

    ctxt->node = node;

    name = virXMLPropString(node, "name");
    if (name == NULL) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                             "%s", _("missing timer name"));
        goto error;
    }
    if ((def->name = virDomainTimerNameTypeFromString(name)) < 0) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                             _("unknown timer name '%s'"), name);
        goto error;
    }

    def->present = -1; /* unspecified */
    if ((present = virXMLPropString(node, "present")) != NULL) {
        if (STREQ(present, "yes")) {
            def->present = 1;
        } else if (STREQ(present, "no")) {
            def->present = 0;
        } else {
            virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                                 _("unknown timer present value '%s'"), present);
            goto error;
        }
    }

    def->tickpolicy = -1;
    tickpolicy = virXMLPropString(node, "tickpolicy");
    if (tickpolicy != NULL) {
        if ((def->tickpolicy = virDomainTimerTickpolicyTypeFromString(tickpolicy)) < 0) {
            virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                                 _("unknown timer tickpolicy '%s'"), tickpolicy);
            goto error;
        }
    }

    def->track = -1;
    track = virXMLPropString(node, "track");
    if (track != NULL) {
        if ((def->track = virDomainTimerTrackTypeFromString(track)) < 0) {
            virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                                 _("unknown timer track '%s'"), track);
            goto error;
        }
    }

    int ret = virXPathULong("string(./frequency)", ctxt, &def->frequency);
    if (ret == -1) {
        def->frequency = 0;
    } else if (ret < 0) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                             "%s", _("invalid timer frequency"));
        goto error;
    }

    def->mode = -1;
    mode = virXMLPropString(node, "mode");
    if (mode != NULL) {
        if ((def->mode = virDomainTimerModeTypeFromString(mode)) < 0) {
            virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                                 _("unknown timer mode '%s'"), mode);
            goto error;
        }
    }

    xmlNodePtr catchup = virXPathNode("./catchup", ctxt);
    if (catchup != NULL) {
        ret = virXPathULong("string(./catchup/@threshold)", ctxt,
                            &def->catchup.threshold);
        if (ret == -1) {
            def->catchup.threshold = 0;
        } else if (ret < 0) {
            virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                                 "%s", _("invalid catchup threshold"));
            goto error;
        }

        ret = virXPathULong("string(./catchup/@slew)", ctxt, &def->catchup.slew);
        if (ret == -1) {
            def->catchup.slew = 0;
        } else if (ret < 0) {
            virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                                 "%s", _("invalid catchup slew"));
            goto error;
        }

        ret = virXPathULong("string(./catchup/@limit)", ctxt, &def->catchup.limit);
        if (ret == -1) {
            def->catchup.limit = 0;
        } else if (ret < 0) {
            virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                                 "%s", _("invalid catchup limit"));
            goto error;
        }
    }

cleanup:
    VIR_FREE(name);
    VIR_FREE(present);
    VIR_FREE(tickpolicy);
    VIR_FREE(track);
    VIR_FREE(mode);
    ctxt->node = oldnode;

    return def;

error:
    VIR_FREE(def);
    goto cleanup;
}


static int
virDomainGraphicsAuthDefParseXML(xmlNodePtr node, virDomainGraphicsAuthDefPtr def)
{
    char *validTo = NULL;

    def->passwd = virXMLPropString(node, "passwd");

    if (!def->passwd)
        return 0;

    validTo = virXMLPropString(node, "passwdValidTo");
    if (validTo) {
        char *tmp;
        struct tm tm;
        memset(&tm, 0, sizeof(tm));
        /* Expect: YYYY-MM-DDTHH:MM:SS (%d-%d-%dT%d:%d:%d)  eg 2010-11-28T14:29:01 */
        if (/* year */
            virStrToLong_i(validTo, &tmp, 10, &tm.tm_year) < 0 || *tmp != '-' ||
            /* month */
            virStrToLong_i(tmp+1, &tmp, 10, &tm.tm_mon) < 0 || *tmp != '-' ||
            /* day */
            virStrToLong_i(tmp+1, &tmp, 10, &tm.tm_mday) < 0 || *tmp != 'T' ||
            /* hour */
            virStrToLong_i(tmp+1, &tmp, 10, &tm.tm_hour) < 0 || *tmp != ':' ||
            /* minute */
            virStrToLong_i(tmp+1, &tmp, 10, &tm.tm_min) < 0 || *tmp != ':' ||
            /* second */
            virStrToLong_i(tmp+1, &tmp, 10, &tm.tm_sec) < 0 || *tmp != '\0') {
            virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                                 _("cannot parse password validity time '%s', expect YYYY-MM-DDTHH:MM:SS"),
                                 validTo);
            VIR_FREE(validTo);
            VIR_FREE(def->passwd);
            return -1;
        }
        VIR_FREE(validTo);

        tm.tm_year -= 1900; /* Human epoch starts at 0 BC, not 1900BC */
        tm.tm_mon--; /* Humans start months at 1, computers at 0 */

        def->validTo = timegm(&tm);
        def->expires = 1;
    }

    return 0;
}


/* Parse the XML definition for a graphics device */
static virDomainGraphicsDefPtr
virDomainGraphicsDefParseXML(xmlNodePtr node, int flags) {
    virDomainGraphicsDefPtr def;
    char *type = NULL;

    if (VIR_ALLOC(def) < 0) {
        virReportOOMError();
        return NULL;
    }

    type = virXMLPropString(node, "type");

    if (!type) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                             "%s", _("missing graphics device type"));
        goto error;
    }

    if ((def->type = virDomainGraphicsTypeFromString(type)) < 0) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                             _("unknown graphics device type '%s'"), type);
        goto error;
    }

    if (def->type == VIR_DOMAIN_GRAPHICS_TYPE_VNC) {
        char *port = virXMLPropString(node, "port");
        char *autoport;

        if (port) {
            if (virStrToLong_i(port, NULL, 10, &def->data.vnc.port) < 0) {
                virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                                     _("cannot parse vnc port %s"), port);
                VIR_FREE(port);
                goto error;
            }
            VIR_FREE(port);
            /* Legacy compat syntax, used -1 for auto-port */
            if (def->data.vnc.port == -1) {
                if (flags & VIR_DOMAIN_XML_INACTIVE)
                    def->data.vnc.port = 0;
                def->data.vnc.autoport = 1;
            }
        } else {
            def->data.vnc.port = 0;
            def->data.vnc.autoport = 1;
        }

        if ((autoport = virXMLPropString(node, "autoport")) != NULL) {
            if (STREQ(autoport, "yes")) {
                if (flags & VIR_DOMAIN_XML_INACTIVE)
                    def->data.vnc.port = 0;
                def->data.vnc.autoport = 1;
            }
            VIR_FREE(autoport);
        }

        def->data.vnc.listenAddr = virXMLPropString(node, "listen");
        def->data.vnc.socket = virXMLPropString(node, "socket");
        def->data.vnc.keymap = virXMLPropString(node, "keymap");

        if (virDomainGraphicsAuthDefParseXML(node, &def->data.vnc.auth) < 0)
            goto error;
    } else if (def->type == VIR_DOMAIN_GRAPHICS_TYPE_SDL) {
        char *fullscreen = virXMLPropString(node, "fullscreen");

        if (fullscreen != NULL) {
            if (STREQ(fullscreen, "yes")) {
                def->data.sdl.fullscreen = 1;
            } else if (STREQ(fullscreen, "no")) {
                def->data.sdl.fullscreen = 0;
            } else {
                virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                             _("unknown fullscreen value '%s'"), fullscreen);
                VIR_FREE(fullscreen);
                goto error;
            }
            VIR_FREE(fullscreen);
        } else
            def->data.sdl.fullscreen = 0;
        def->data.sdl.xauth = virXMLPropString(node, "xauth");
        def->data.sdl.display = virXMLPropString(node, "display");
    } else if (def->type == VIR_DOMAIN_GRAPHICS_TYPE_RDP) {
        char *port = virXMLPropString(node, "port");
        char *autoport;
        char *replaceUser;
        char *multiUser;

        if (port) {
            if (virStrToLong_i(port, NULL, 10, &def->data.rdp.port) < 0) {
                virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                                     _("cannot parse rdp port %s"), port);
                VIR_FREE(port);
                goto error;
            }
            VIR_FREE(port);
        } else {
            def->data.rdp.port = 0;
            def->data.rdp.autoport = 1;
        }

        if ((autoport = virXMLPropString(node, "autoport")) != NULL) {
            if (STREQ(autoport, "yes")) {
                if (flags & VIR_DOMAIN_XML_INACTIVE)
                    def->data.rdp.port = 0;
                def->data.rdp.autoport = 1;
            }
            VIR_FREE(autoport);
        }

        if ((replaceUser = virXMLPropString(node, "replaceUser")) != NULL) {
            if (STREQ(replaceUser, "yes")) {
                def->data.rdp.replaceUser = 1;
            }
            VIR_FREE(replaceUser);
        }

        if ((multiUser = virXMLPropString(node, "multiUser")) != NULL) {
            if (STREQ(multiUser, "yes")) {
                def->data.rdp.multiUser = 1;
            }
            VIR_FREE(multiUser);
        }

        def->data.rdp.listenAddr = virXMLPropString(node, "listen");
    } else if (def->type == VIR_DOMAIN_GRAPHICS_TYPE_DESKTOP) {
        char *fullscreen = virXMLPropString(node, "fullscreen");

        if (fullscreen != NULL) {
            if (STREQ(fullscreen, "yes")) {
                def->data.desktop.fullscreen = 1;
            } else if (STREQ(fullscreen, "no")) {
                def->data.desktop.fullscreen = 0;
            } else {
                virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                             _("unknown fullscreen value '%s'"), fullscreen);
                VIR_FREE(fullscreen);
                goto error;
            }
            VIR_FREE(fullscreen);
        } else
            def->data.desktop.fullscreen = 0;

        def->data.desktop.display = virXMLPropString(node, "display");
    } else if (def->type == VIR_DOMAIN_GRAPHICS_TYPE_SPICE) {
        xmlNodePtr cur;
        char *port = virXMLPropString(node, "port");
        char *tlsPort;
        char *autoport;

        if (port) {
            if (virStrToLong_i(port, NULL, 10, &def->data.spice.port) < 0) {
                virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                                     _("cannot parse spice port %s"), port);
                VIR_FREE(port);
                goto error;
            }
            VIR_FREE(port);
        } else {
            def->data.spice.port = 5900;
        }

        tlsPort = virXMLPropString(node, "tlsPort");
        if (tlsPort) {
            if (virStrToLong_i(tlsPort, NULL, 10, &def->data.spice.tlsPort) < 0) {
                virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                                     _("cannot parse spice tlsPort %s"), tlsPort);
                VIR_FREE(tlsPort);
                goto error;
            }
            VIR_FREE(tlsPort);
        } else {
            def->data.spice.tlsPort = 0;
        }

        if ((autoport = virXMLPropString(node, "autoport")) != NULL) {
            if (STREQ(autoport, "yes")) {
                if (flags & VIR_DOMAIN_XML_INACTIVE) {
                    def->data.spice.port = 0;
                    def->data.spice.tlsPort = 0;
                }
                def->data.spice.autoport = 1;
            }
            VIR_FREE(autoport);
        }

        def->data.spice.listenAddr = virXMLPropString(node, "listen");
        def->data.spice.keymap = virXMLPropString(node, "keymap");
        if (virDomainGraphicsAuthDefParseXML(node, &def->data.spice.auth) < 0)
            goto error;

        cur = node->children;
        while (cur != NULL) {
            if (cur->type == XML_ELEMENT_NODE) {
                if (xmlStrEqual(cur->name, BAD_CAST "channel")) {
                    const char *name, *mode;
                    int nameval, modeval;
                    name = virXMLPropString(cur, "name");
                    mode = virXMLPropString(cur, "mode");

                    if (!name || !mode) {
                        virDomainReportError(VIR_ERR_INTERNAL_ERROR, "%s",
                                             _("spice channel missing name/mode"));
                        VIR_FREE(name);
                        VIR_FREE(mode);
                        goto error;
                    }

                    if ((nameval = virDomainGraphicsSpiceChannelNameTypeFromString(name)) < 0) {
                        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                                             _("unknown spice channel name %s"),
                                             name);
                        VIR_FREE(name);
                        VIR_FREE(mode);
                        goto error;
                    }
                    if ((modeval = virDomainGraphicsSpiceChannelModeTypeFromString(mode)) < 0) {
                        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                                             _("unknown spice channel mode %s"),
                                             mode);
                        VIR_FREE(name);
                        VIR_FREE(mode);
                        goto error;
                    }
                    VIR_FREE(name);
                    VIR_FREE(mode);

                    def->data.spice.channels[nameval] = modeval;
                }
            }
            cur = cur->next;
        }
    }

cleanup:
    VIR_FREE(type);

    return def;

error:
    virDomainGraphicsDefFree(def);
    def = NULL;
    goto cleanup;
}


static virDomainSoundDefPtr
virDomainSoundDefParseXML(const xmlNodePtr node,
                          int flags)
{
    char *model;
    virDomainSoundDefPtr def;

    if (VIR_ALLOC(def) < 0) {
        virReportOOMError();
        return NULL;
    }

    model = virXMLPropString(node, "model");
    if ((def->model = virDomainSoundModelTypeFromString(model)) < 0) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                             _("unknown sound model '%s'"), model);
        goto error;
    }

    if (virDomainDeviceInfoParseXML(node, &def->info, flags) < 0)
        goto error;

cleanup:
    VIR_FREE(model);

    return def;

error:
    virDomainSoundDefFree(def);
    def = NULL;
    goto cleanup;
}


static virDomainWatchdogDefPtr
virDomainWatchdogDefParseXML(const xmlNodePtr node,
                             int flags)
{

    char *model = NULL;
    char *action = NULL;
    virDomainWatchdogDefPtr def;

    if (VIR_ALLOC (def) < 0) {
        virReportOOMError();
        return NULL;
    }

    model = virXMLPropString (node, "model");
    if (model == NULL) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR, "%s",
                             _("watchdog must contain model name"));
        goto error;
    }
    def->model = virDomainWatchdogModelTypeFromString (model);
    if (def->model < 0) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                             _("unknown watchdog model '%s'"), model);
        goto error;
    }

    action = virXMLPropString (node, "action");
    if (action == NULL)
        def->action = VIR_DOMAIN_WATCHDOG_ACTION_RESET;
    else {
        def->action = virDomainWatchdogActionTypeFromString (action);
        if (def->action < 0) {
            virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                                 _("unknown watchdog action '%s'"), action);
            goto error;
        }
    }

    if (virDomainDeviceInfoParseXML(node, &def->info, flags) < 0)
        goto error;

cleanup:
    VIR_FREE (action);
    VIR_FREE (model);

    return def;

error:
    virDomainWatchdogDefFree (def);
    def = NULL;
    goto cleanup;
}


static virDomainMemballoonDefPtr
virDomainMemballoonDefParseXML(const xmlNodePtr node,
                               int flags)
{
    char *model;
    virDomainMemballoonDefPtr def;

    if (VIR_ALLOC(def) < 0) {
        virReportOOMError();
        return NULL;
    }

    model = virXMLPropString(node, "model");
    if (model == NULL) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR, "%s",
                             _("balloon memory must contain model name"));
        goto error;
    }
    if ((def->model = virDomainMemballoonModelTypeFromString(model)) < 0) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                             _("unknown memory balloon model '%s'"), model);
        goto error;
    }

    if (virDomainDeviceInfoParseXML(node, &def->info, flags) < 0)
        goto error;

cleanup:
    VIR_FREE(model);

    return def;

error:
    virDomainMemballoonDefFree(def);
    def = NULL;
    goto cleanup;
}

static virSysinfoDefPtr
virSysinfoParseXML(const xmlNodePtr node,
                  xmlXPathContextPtr ctxt)
{
    virSysinfoDefPtr def;
    char *type;

    if (!xmlStrEqual(node->name, BAD_CAST "sysinfo")) {
        virDomainReportError(VIR_ERR_XML_ERROR, "%s",
                        _("XML does not contain expected 'sysinfo' element"));
        return(NULL);
    }

    if (VIR_ALLOC(def) < 0) {
        virReportOOMError();
        return(NULL);
    }

    type = virXMLPropString(node, "type");
    if (type == NULL) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR, "%s",
                             _("sysinfo must contain a type attribute"));
        goto error;
    }
    if ((def->type = virDomainSysinfoTypeFromString(type)) < 0) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                             _("unknown sysinfo type '%s'"), type);
        goto error;
    }


    /* Extract BIOS related metadata */
    def->bios_vendor =
        virXPathString("string(bios/entry[@name='vendor'])", ctxt);
    def->bios_version =
        virXPathString("string(bios/entry[@name='version'])", ctxt);
    def->bios_date =
        virXPathString("string(bios/entry[@name='date'])", ctxt);
    def->bios_release =
        virXPathString("string(bios/entry[@name='release'])", ctxt);

    /* Extract system related metadata */
    def->system_manufacturer =
        virXPathString("string(system/entry[@name='manufacturer'])", ctxt);
    def->system_product =
        virXPathString("string(system/entry[@name='product'])", ctxt);
    def->system_version =
        virXPathString("string(system/entry[@name='version'])", ctxt);
    def->system_serial =
        virXPathString("string(system/entry[@name='serial'])", ctxt);
    def->system_uuid =
        virXPathString("string(system/entry[@name='uuid'])", ctxt);
    def->system_sku =
        virXPathString("string(system/entry[@name='sku'])", ctxt);
    def->system_family =
        virXPathString("string(system/entry[@name='family'])", ctxt);

cleanup:
    VIR_FREE(type);
    return(def);

error:
    virSysinfoDefFree(def);
    def = NULL;
    goto cleanup;
}

int
virDomainVideoDefaultRAM(virDomainDefPtr def,
                         int type)
{
    switch (type) {
        /* Wierd, QEMU defaults to 9 MB ??! */
    case VIR_DOMAIN_VIDEO_TYPE_VGA:
    case VIR_DOMAIN_VIDEO_TYPE_CIRRUS:
    case VIR_DOMAIN_VIDEO_TYPE_VMVGA:
        if (def->virtType == VIR_DOMAIN_VIRT_VBOX)
            return 8 * 1024;
        else if (def->virtType == VIR_DOMAIN_VIRT_VMWARE)
            return 4 * 1024;
        else
            return 9 * 1024;
        break;

    case VIR_DOMAIN_VIDEO_TYPE_XEN:
        /* Original Xen PVFB hardcoded to 4 MB */
        return 4 * 1024;

    default:
        return 0;
    }
}


int
virDomainVideoDefaultType(virDomainDefPtr def)
{
    switch (def->virtType) {
    case VIR_DOMAIN_VIRT_TEST:
    case VIR_DOMAIN_VIRT_QEMU:
    case VIR_DOMAIN_VIRT_KQEMU:
    case VIR_DOMAIN_VIRT_KVM:
    case VIR_DOMAIN_VIRT_XEN:
        if (def->os.type &&
            (STREQ(def->os.type, "xen") ||
             STREQ(def->os.type, "linux")))
            return VIR_DOMAIN_VIDEO_TYPE_XEN;
        else
            return VIR_DOMAIN_VIDEO_TYPE_CIRRUS;

    case VIR_DOMAIN_VIRT_VBOX:
        return VIR_DOMAIN_VIDEO_TYPE_VBOX;

    case VIR_DOMAIN_VIRT_VMWARE:
        return VIR_DOMAIN_VIDEO_TYPE_VMVGA;

    default:
        return -1;
    }
}

static virDomainVideoAccelDefPtr
virDomainVideoAccelDefParseXML(const xmlNodePtr node) {
    xmlNodePtr cur;
    virDomainVideoAccelDefPtr def;
    char *support3d = NULL;
    char *support2d = NULL;

    cur = node->children;
    while (cur != NULL) {
        if (cur->type == XML_ELEMENT_NODE) {
            if ((support3d == NULL) && (support2d == NULL) &&
                xmlStrEqual(cur->name, BAD_CAST "acceleration")) {
                support3d = virXMLPropString(cur, "accel3d");
                support2d = virXMLPropString(cur, "accel2d");
            }
        }
        cur = cur->next;
    }

    if ((support3d == NULL) && (support2d == NULL))
        return(NULL);

    if (VIR_ALLOC(def) < 0) {
        virReportOOMError();
        return NULL;
    }

    if (support3d) {
        if (STREQ(support3d, "yes"))
            def->support3d = 1;
        else
            def->support3d = 0;
        VIR_FREE(support3d);
    }

    if (support2d) {
        if (STREQ(support2d, "yes"))
            def->support2d = 1;
        else
            def->support2d = 0;
        VIR_FREE(support2d);
    }

    return def;
}

static virDomainVideoDefPtr
virDomainVideoDefParseXML(const xmlNodePtr node,
                          virDomainDefPtr dom,
                          int flags) {
    virDomainVideoDefPtr def;
    xmlNodePtr cur;
    char *type = NULL;
    char *heads = NULL;
    char *vram = NULL;

    if (VIR_ALLOC(def) < 0) {
        virReportOOMError();
        return NULL;
    }

    cur = node->children;
    while (cur != NULL) {
        if (cur->type == XML_ELEMENT_NODE) {
            if ((type == NULL) && (vram == NULL) && (heads == NULL) &&
                xmlStrEqual(cur->name, BAD_CAST "model")) {
                type = virXMLPropString(cur, "type");
                vram = virXMLPropString(cur, "vram");
                heads = virXMLPropString(cur, "heads");
                def->accel = virDomainVideoAccelDefParseXML(cur);
            }
        }
        cur = cur->next;
    }

    if (type) {
        if ((def->type = virDomainVideoTypeFromString(type)) < 0) {
            virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                                 _("unknown video model '%s'"), type);
            goto error;
        }
    } else {
        if ((def->type = virDomainVideoDefaultType(dom)) < 0) {
            virDomainReportError(VIR_ERR_INTERNAL_ERROR, "%s",
                                 _("missing video model and cannot determine default"));
            goto error;
        }
    }

    if (vram) {
        if (virStrToLong_ui(vram, NULL, 10, &def->vram) < 0) {
            virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                                 _("cannot parse video ram '%s'"), vram);
            goto error;
        }
    } else {
        def->vram = virDomainVideoDefaultRAM(dom, def->type);
    }

    if (heads) {
        if (virStrToLong_ui(heads, NULL, 10, &def->heads) < 0) {
            virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                                 _("cannot parse video heads '%s'"), heads);
            goto error;
        }
    } else {
        def->heads = 1;
    }

    if (virDomainDeviceInfoParseXML(node, &def->info, flags) < 0)
        goto error;

    VIR_FREE(type);
    VIR_FREE(vram);
    VIR_FREE(heads);

    return def;

error:
    virDomainVideoDefFree(def);
    VIR_FREE(type);
    VIR_FREE(vram);
    VIR_FREE(heads);
    return NULL;
}

static int
virDomainHostdevSubsysUsbDefParseXML(const xmlNodePtr node,
                                     virDomainHostdevDefPtr def,
                                     int flags ATTRIBUTE_UNUSED) {

    int ret = -1;
    int got_product, got_vendor;
    xmlNodePtr cur;

    /* Product can validly be 0, so we need some extra help to determine
     * if it is uninitialized*/
    got_product = 0;
    got_vendor = 0;

    cur = node->children;
    while (cur != NULL) {
        if (cur->type == XML_ELEMENT_NODE) {
            if (xmlStrEqual(cur->name, BAD_CAST "vendor")) {
                char *vendor = virXMLPropString(cur, "id");

                if (vendor) {
                    got_vendor = 1;
                    if (virStrToLong_ui(vendor, NULL, 0,
                                    &def->source.subsys.u.usb.vendor) < 0) {
                        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                                 _("cannot parse vendor id %s"), vendor);
                        VIR_FREE(vendor);
                        goto out;
                    }
                    VIR_FREE(vendor);
                } else {
                    virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                                         "%s", _("usb vendor needs id"));
                    goto out;
                }
            } else if (xmlStrEqual(cur->name, BAD_CAST "product")) {
                char* product = virXMLPropString(cur, "id");

                if (product) {
                    got_product = 1;
                    if (virStrToLong_ui(product, NULL, 0,
                                        &def->source.subsys.u.usb.product) < 0) {
                        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                                             _("cannot parse product %s"),
                                             product);
                        VIR_FREE(product);
                        goto out;
                    }
                    VIR_FREE(product);
                } else {
                    virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                                         "%s", _("usb product needs id"));
                    goto out;
                }
            } else if (xmlStrEqual(cur->name, BAD_CAST "address")) {
                char *bus, *device;

                bus = virXMLPropString(cur, "bus");
                if (bus) {
                    if (virStrToLong_ui(bus, NULL, 0,
                                        &def->source.subsys.u.usb.bus) < 0) {
                        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                                             _("cannot parse bus %s"), bus);
                        VIR_FREE(bus);
                        goto out;
                    }
                    VIR_FREE(bus);
                } else {
                    virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                                         "%s", _("usb address needs bus id"));
                    goto out;
                }

                device = virXMLPropString(cur, "device");
                if (device) {
                    if (virStrToLong_ui(device, NULL, 0,
                                        &def->source.subsys.u.usb.device) < 0)  {
                        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                                             _("cannot parse device %s"),
                                             device);
                        VIR_FREE(device);
                        goto out;
                    }
                    VIR_FREE(device);
                } else {
                    virDomainReportError(VIR_ERR_INTERNAL_ERROR, "%s",
                                         _("usb address needs device id"));
                    goto out;
                }
            } else {
                virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                                     _("unknown usb source type '%s'"),
                                     cur->name);
                goto out;
            }
        }
        cur = cur->next;
    }

    if (got_vendor && def->source.subsys.u.usb.vendor == 0) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
            "%s", _("vendor cannot be 0."));
        goto out;
    }

    if (!got_vendor && got_product) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
            "%s", _("missing vendor"));
        goto out;
    }
    if (got_vendor && !got_product) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
            "%s", _("missing product"));
        goto out;
    }

    ret = 0;
out:
    return ret;
}


static int
virDomainHostdevSubsysPciDefParseXML(const xmlNodePtr node,
                                     virDomainHostdevDefPtr def,
                                     int flags) {

    int ret = -1;
    xmlNodePtr cur;

    cur = node->children;
    while (cur != NULL) {
        if (cur->type == XML_ELEMENT_NODE) {
            if (xmlStrEqual(cur->name, BAD_CAST "address")) {
                virDomainDevicePCIAddressPtr addr =
                    &def->source.subsys.u.pci;

                if (virDomainDevicePCIAddressParseXML(cur, addr) < 0)
                    goto out;
            } else if ((flags & VIR_DOMAIN_XML_INTERNAL_STATUS) &&
                       xmlStrEqual(cur->name, BAD_CAST "state")) {
                /* Legacy back-compat. Don't add any more attributes here */
                char *devaddr = virXMLPropString(cur, "devaddr");
                if (devaddr &&
                    virDomainParseLegacyDeviceAddress(devaddr,
                                                      &def->info.addr.pci) < 0) {
                    virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                                         _("Unable to parse devaddr parameter '%s'"),
                                         devaddr);
                    VIR_FREE(devaddr);
                    goto out;
                }
                def->info.type = VIR_DOMAIN_DEVICE_ADDRESS_TYPE_PCI;
            } else {
                virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                                     _("unknown pci source type '%s'"),
                                     cur->name);
                goto out;
            }
        }
        cur = cur->next;
    }

    ret = 0;
out:
    return ret;
}


static virDomainHostdevDefPtr
virDomainHostdevDefParseXML(const xmlNodePtr node,
                            virBitmapPtr bootMap,
                            int flags)
{

    xmlNodePtr cur;
    virDomainHostdevDefPtr def;
    char *mode, *type = NULL, *managed = NULL;

    if (VIR_ALLOC(def) < 0) {
        virReportOOMError();
        return NULL;
    }
    def->target = NULL;

    mode = virXMLPropString(node, "mode");
    if (mode) {
        if ((def->mode=virDomainHostdevModeTypeFromString(mode)) < 0) {
             virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                                  _("unknown hostdev mode '%s'"), mode);
            goto error;
        }
    } else {
        def->mode = VIR_DOMAIN_HOSTDEV_MODE_SUBSYS;
    }

    type = virXMLPropString(node, "type");
    if (type) {
        if ((def->source.subsys.type = virDomainHostdevSubsysTypeFromString(type)) < 0) {
            virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                                 _("unknown host device type '%s'"), type);
            goto error;
        }
    } else {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                             "%s", _("missing type in hostdev"));
        goto error;
    }

    managed = virXMLPropString(node, "managed");
    if (managed != NULL) {
        if (STREQ(managed, "yes"))
            def->managed = 1;
        VIR_FREE(managed);
    }

    cur = node->children;
    while (cur != NULL) {
        if (cur->type == XML_ELEMENT_NODE) {
            if (xmlStrEqual(cur->name, BAD_CAST "source")) {
                if (def->mode == VIR_DOMAIN_HOSTDEV_MODE_SUBSYS &&
                    def->source.subsys.type == VIR_DOMAIN_HOSTDEV_SUBSYS_TYPE_USB) {
                        if (virDomainHostdevSubsysUsbDefParseXML(cur, def, flags) < 0)
                            goto error;
                }
                if (def->mode == VIR_DOMAIN_HOSTDEV_MODE_SUBSYS &&
                    def->source.subsys.type == VIR_DOMAIN_HOSTDEV_SUBSYS_TYPE_PCI) {
                        if (virDomainHostdevSubsysPciDefParseXML(cur, def, flags) < 0)
                            goto error;
                }
            } else if (xmlStrEqual(cur->name, BAD_CAST "address")) {
                /* address is parsed as part of virDomainDeviceInfoParseXML */
            } else if (xmlStrEqual(cur->name, BAD_CAST "alias")) {
                /* alias is parsed as part of virDomainDeviceInfoParseXML */
            } else if (xmlStrEqual(cur->name, BAD_CAST "boot")) {
                if (virDomainDeviceBootParseXML(cur, &def->bootIndex,
                                                bootMap))
                    goto error;
            } else {
                virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                                     _("unknown node %s"), cur->name);
            }
        }
        cur = cur->next;
    }

    if (def->info.type == VIR_DOMAIN_DEVICE_ADDRESS_TYPE_NONE) {
        if (virDomainDeviceInfoParseXML(node, &def->info, flags) < 0)
            goto error;
    }

    if (def->mode == VIR_DOMAIN_HOSTDEV_MODE_SUBSYS) {
        switch (def->source.subsys.type) {
        case VIR_DOMAIN_HOSTDEV_SUBSYS_TYPE_PCI:
            if (def->info.type != VIR_DOMAIN_DEVICE_ADDRESS_TYPE_NONE &&
                def->info.type != VIR_DOMAIN_DEVICE_ADDRESS_TYPE_PCI) {
                virDomainReportError(VIR_ERR_INTERNAL_ERROR, "%s",
                                     _("PCI host devices must use 'pci' address type"));
                goto error;
            }
            break;
        }
    }

cleanup:
    VIR_FREE(type);
    VIR_FREE(mode);
    return def;

error:
    virDomainHostdevDefFree(def);
    def = NULL;
    goto cleanup;
}


static int virDomainLifecycleParseXML(xmlXPathContextPtr ctxt,
                                      const char *xpath,
                                      int *val,
                                      int defaultVal,
                                      virLifecycleFromStringFunc convFunc)
{
    char *tmp = virXPathString(xpath, ctxt);
    if (tmp == NULL) {
        *val = defaultVal;
    } else {
        *val = convFunc(tmp);
        if (*val < 0) {
            virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                                 _("unknown lifecycle action %s"), tmp);
            VIR_FREE(tmp);
            return -1;
        }
        VIR_FREE(tmp);
    }
    return 0;
}

static int
virSecurityLabelDefParseXML(const virDomainDefPtr def,
                            xmlXPathContextPtr ctxt,
                            int flags)
{
    char *p;

    if (virXPathNode("./seclabel", ctxt) == NULL)
        return 0;

    p = virXPathStringLimit("string(./seclabel/@type)",
                            VIR_SECURITY_LABEL_BUFLEN-1, ctxt);
    if (p == NULL) {
        virDomainReportError(VIR_ERR_XML_ERROR,
                             "%s", _("missing security type"));
        goto error;
    }
    def->seclabel.type = virDomainSeclabelTypeFromString(p);
    VIR_FREE(p);
    if (def->seclabel.type < 0) {
        virDomainReportError(VIR_ERR_XML_ERROR,
                             "%s", _("invalid security type"));
        goto error;
    }

    /* Only parse details, if using static labels, or
     * if the 'live' VM XML is requested
     */
    if (def->seclabel.type == VIR_DOMAIN_SECLABEL_STATIC ||
        !(flags & VIR_DOMAIN_XML_INACTIVE)) {
        p = virXPathStringLimit("string(./seclabel/@model)",
                                VIR_SECURITY_MODEL_BUFLEN-1, ctxt);
        if (p == NULL) {
            virDomainReportError(VIR_ERR_XML_ERROR,
                                 "%s", _("missing security model"));
            goto error;
        }
        def->seclabel.model = p;

        p = virXPathStringLimit("string(./seclabel/label[1])",
                                VIR_SECURITY_LABEL_BUFLEN-1, ctxt);
        if (p == NULL) {
            virDomainReportError(VIR_ERR_XML_ERROR,
                                 "%s", _("security label is missing"));
            goto error;
        }

        def->seclabel.label = p;
    }

    /* Only parse imagelabel, if requested live XML for dynamic label */
    if (def->seclabel.type == VIR_DOMAIN_SECLABEL_DYNAMIC &&
        !(flags & VIR_DOMAIN_XML_INACTIVE)) {
        p = virXPathStringLimit("string(./seclabel/imagelabel[1])",
                                VIR_SECURITY_LABEL_BUFLEN-1, ctxt);
        if (p == NULL) {
            virDomainReportError(VIR_ERR_XML_ERROR,
                                 "%s", _("security imagelabel is missing"));
            goto error;
        }
        def->seclabel.imagelabel = p;
    }

    return 0;

error:
    virSecurityLabelDefFree(def);
    return -1;
}

virDomainDeviceDefPtr virDomainDeviceDefParse(virCapsPtr caps,
                                              const virDomainDefPtr def,
                                              const char *xmlStr,
                                              int flags)
{
    xmlDocPtr xml;
    xmlNodePtr node;
    xmlXPathContextPtr ctxt = NULL;
    virDomainDeviceDefPtr dev = NULL;

    if (!(xml = xmlReadDoc(BAD_CAST xmlStr, "device.xml", NULL,
                           XML_PARSE_NOENT | XML_PARSE_NONET |
                           XML_PARSE_NOERROR | XML_PARSE_NOWARNING))) {
        virDomainReportError(VIR_ERR_XML_ERROR, NULL);
        goto error;
    }

    node = xmlDocGetRootElement(xml);
    if (node == NULL) {
        virDomainReportError(VIR_ERR_XML_ERROR,
                             "%s", _("missing root element"));
        goto error;
    }

    ctxt = xmlXPathNewContext(xml);
    if (ctxt == NULL) {
        virReportOOMError();
        goto error;
    }
    ctxt->node = node;

    if (VIR_ALLOC(dev) < 0) {
        virReportOOMError();
        goto error;
    }

    if (xmlStrEqual(node->name, BAD_CAST "disk")) {
        dev->type = VIR_DOMAIN_DEVICE_DISK;
        if (!(dev->data.disk = virDomainDiskDefParseXML(caps, node,
                                                        NULL, flags)))
            goto error;
    } else if (xmlStrEqual(node->name, BAD_CAST "filesystem")) {
        dev->type = VIR_DOMAIN_DEVICE_FS;
        if (!(dev->data.fs = virDomainFSDefParseXML(node, flags)))
            goto error;
    } else if (xmlStrEqual(node->name, BAD_CAST "interface")) {
        dev->type = VIR_DOMAIN_DEVICE_NET;
        if (!(dev->data.net = virDomainNetDefParseXML(caps, node, ctxt,
                                                      NULL, flags)))
            goto error;
    } else if (xmlStrEqual(node->name, BAD_CAST "input")) {
        dev->type = VIR_DOMAIN_DEVICE_INPUT;
        if (!(dev->data.input = virDomainInputDefParseXML(def->os.type,
                                                          node, flags)))
            goto error;
    } else if (xmlStrEqual(node->name, BAD_CAST "sound")) {
        dev->type = VIR_DOMAIN_DEVICE_SOUND;
        if (!(dev->data.sound = virDomainSoundDefParseXML(node, flags)))
            goto error;
    } else if (xmlStrEqual(node->name, BAD_CAST "watchdog")) {
        dev->type = VIR_DOMAIN_DEVICE_WATCHDOG;
        if (!(dev->data.watchdog = virDomainWatchdogDefParseXML(node, flags)))
            goto error;
    } else if (xmlStrEqual(node->name, BAD_CAST "video")) {
        dev->type = VIR_DOMAIN_DEVICE_VIDEO;
        if (!(dev->data.video = virDomainVideoDefParseXML(node, def, flags)))
            goto error;
    } else if (xmlStrEqual(node->name, BAD_CAST "hostdev")) {
        dev->type = VIR_DOMAIN_DEVICE_HOSTDEV;
        if (!(dev->data.hostdev = virDomainHostdevDefParseXML(node, NULL,
                                                              flags)))
            goto error;
    } else if (xmlStrEqual(node->name, BAD_CAST "controller")) {
        dev->type = VIR_DOMAIN_DEVICE_CONTROLLER;
        if (!(dev->data.controller = virDomainControllerDefParseXML(node, flags)))
            goto error;
    } else if (xmlStrEqual(node->name, BAD_CAST "graphics")) {
        dev->type = VIR_DOMAIN_DEVICE_GRAPHICS;
        if (!(dev->data.graphics = virDomainGraphicsDefParseXML(node, flags)))
            goto error;
    } else {
        virDomainReportError(VIR_ERR_XML_ERROR,
                             "%s", _("unknown device type"));
        goto error;
    }

    xmlFreeDoc(xml);
    xmlXPathFreeContext(ctxt);
    return dev;

  error:
    xmlFreeDoc(xml);
    xmlXPathFreeContext(ctxt);
    VIR_FREE(dev);
    return NULL;
}


static const char *
virDomainChrTargetTypeToString(int deviceType,
                               int targetType)
{
    const char *type = NULL;

    switch (deviceType) {
    case VIR_DOMAIN_CHR_DEVICE_TYPE_CHANNEL:
        type = virDomainChrChannelTargetTypeToString(targetType);
        break;
    case VIR_DOMAIN_CHR_DEVICE_TYPE_CONSOLE:
        type = virDomainChrConsoleTargetTypeToString(targetType);
        break;
    default:
        break;
    }

    return type;
}

static void
virVirtualPortProfileFormat(virBufferPtr buf,
                            virVirtualPortProfileParamsPtr virtPort,
                            const char *indent)
{
    char uuidstr[VIR_UUID_STRING_BUFLEN];

    if (virtPort->virtPortType == VIR_VIRTUALPORT_NONE)
        return;

    virBufferVSprintf(buf, "%s<virtualport type='%s'>\n",
                      indent,
                      virVirtualPortTypeToString(virtPort->virtPortType));

    switch (virtPort->virtPortType) {
    case VIR_VIRTUALPORT_NONE:
    case VIR_VIRTUALPORT_TYPE_LAST:
        break;

    case VIR_VIRTUALPORT_8021QBG:
        virUUIDFormat(virtPort->u.virtPort8021Qbg.instanceID,
                      uuidstr);
        virBufferVSprintf(buf,
                          "%s  <parameters managerid='%d' typeid='%d' "
                          "typeidversion='%d' instanceid='%s'/>\n",
                          indent,
                          virtPort->u.virtPort8021Qbg.managerID,
                          virtPort->u.virtPort8021Qbg.typeID,
                          virtPort->u.virtPort8021Qbg.typeIDVersion,
                          uuidstr);
        break;

    case VIR_VIRTUALPORT_8021QBH:
        virBufferVSprintf(buf,
                          "%s  <parameters profileid='%s'/>\n",
                          indent,
                          virtPort->u.virtPort8021Qbh.profileID);
        break;
    }

    virBufferVSprintf(buf, "%s</virtualport>\n", indent);
}

int virDomainDiskInsert(virDomainDefPtr def,
                        virDomainDiskDefPtr disk)
{

    if (VIR_REALLOC_N(def->disks, def->ndisks+1) < 0)
        return -1;

    virDomainDiskInsertPreAlloced(def, disk);

    return 0;
}

void virDomainDiskInsertPreAlloced(virDomainDefPtr def,
                                   virDomainDiskDefPtr disk)
{
    int i;
    /* Tenatively plan to insert disk at the end. */
    int insertAt = -1;

    /* Then work backwards looking for disks on
     * the same bus. If we find a disk with a drive
     * index greater than the new one, insert at
     * that position
     */
    for (i = (def->ndisks - 1) ; i >= 0 ; i--) {
        /* If bus matches and current disk is after
         * new disk, then new disk should go here */
        if (def->disks[i]->bus == disk->bus &&
            (virDiskNameToIndex(def->disks[i]->dst) >
             virDiskNameToIndex(disk->dst))) {
            insertAt = i;
        } else if (def->disks[i]->bus == disk->bus &&
                   insertAt == -1) {
            /* Last disk with match bus is before the
             * new disk, then put new disk just after
             */
            insertAt = i + 1;
        }
    }

    /* No disks with this bus yet, so put at end of list */
    if (insertAt == -1)
        insertAt = def->ndisks;

    if (insertAt < def->ndisks)
        memmove(def->disks + insertAt + 1,
                def->disks + insertAt,
                (sizeof(def->disks[0]) * (def->ndisks-insertAt)));

    def->disks[insertAt] = disk;
    def->ndisks++;
}


void virDomainDiskRemove(virDomainDefPtr def, size_t i)
{
    if (def->ndisks > 1) {
        memmove(def->disks + i,
                def->disks + i + 1,
                sizeof(*def->disks) *
                (def->ndisks - (i + 1)));
        def->ndisks--;
        if (VIR_REALLOC_N(def->disks, def->ndisks) < 0) {
            /* ignore, harmless */
        }
    } else {
        VIR_FREE(def->disks);
        def->ndisks = 0;
    }
}


int virDomainControllerInsert(virDomainDefPtr def,
                              virDomainControllerDefPtr controller)
{

    if (VIR_REALLOC_N(def->controllers, def->ncontrollers+1) < 0)
        return -1;

    virDomainControllerInsertPreAlloced(def, controller);

    return 0;
}

void virDomainControllerInsertPreAlloced(virDomainDefPtr def,
                                         virDomainControllerDefPtr controller)
{
    int i;
    /* Tenatively plan to insert controller at the end. */
    int insertAt = -1;

    /* Then work backwards looking for controllers of
     * the same type. If we find a controller with a
     * index greater than the new one, insert at
     * that position
     */
    for (i = (def->ncontrollers - 1) ; i >= 0 ; i--) {
        /* If bus matches and current controller is after
         * new controller, then new controller should go here */
        if ((def->controllers[i]->type == controller->type) &&
            (def->controllers[i]->idx > controller->idx)) {
            insertAt = i;
        } else if (def->controllers[i]->type == controller->type &&
                   insertAt == -1) {
            /* Last controller with match bus is before the
             * new controller, then put new controller just after
             */
            insertAt = i + 1;
        }
    }

    /* No controllers with this bus yet, so put at end of list */
    if (insertAt == -1)
        insertAt = def->ncontrollers;

    if (insertAt < def->ncontrollers)
        memmove(def->controllers + insertAt + 1,
                def->controllers + insertAt,
                (sizeof(def->controllers[0]) * (def->ncontrollers-insertAt)));

    def->controllers[insertAt] = controller;
    def->ncontrollers++;
}


static char *virDomainDefDefaultEmulator(virDomainDefPtr def,
                                         virCapsPtr caps) {
    const char *type;
    const char *emulator;
    char *retemu;

    type = virDomainVirtTypeToString(def->virtType);
    if (!type) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                             "%s", _("unknown virt type"));
        return NULL;
    }

    emulator = virCapabilitiesDefaultGuestEmulator(caps,
                                                   def->os.type,
                                                   def->os.arch,
                                                   type);

    if (!emulator) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                             _("no emulator for domain %s os type %s on architecture %s"),
                             type, def->os.type, def->os.arch);
        return NULL;
    }

    retemu = strdup(emulator);
    if (!retemu)
        virReportOOMError();

    return retemu;
}

static int
virDomainDefParseBootXML(xmlXPathContextPtr ctxt,
                         virDomainDefPtr def,
                         unsigned long *bootCount)
{
    xmlNodePtr *nodes = NULL;
    int i, n;
    char *bootstr;
    int ret = -1;
    unsigned long deviceBoot;

    if (virXPathULong("count(./devices/disk[boot]"
                      "|./devices/interface[boot]"
                      "|./devices/hostdev[boot])", ctxt, &deviceBoot) < 0) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR, "%s",
                             _("cannot count boot devices"));
        goto cleanup;
    }

    /* analysis of the boot devices */
    if ((n = virXPathNodeSet("./os/boot", ctxt, &nodes)) < 0) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                             "%s", _("cannot extract boot device"));
        goto cleanup;
    }

    if (n > 0 && deviceBoot) {
        virDomainReportError(VIR_ERR_CONFIG_UNSUPPORTED, "%s",
                             _("per-device boot elements cannot be used"
                               " together with os/boot elements"));
        goto cleanup;
    }

    for (i = 0 ; i < n && i < VIR_DOMAIN_BOOT_LAST ; i++) {
        int val;
        char *dev = virXMLPropString(nodes[i], "dev");
        if (!dev) {
            virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                                 "%s", _("missing boot device"));
            goto cleanup;
        }
        if ((val = virDomainBootTypeFromString(dev)) < 0) {
            virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                                 _("unknown boot device '%s'"),
                                 dev);
            VIR_FREE(dev);
            goto cleanup;
        }
        VIR_FREE(dev);
        def->os.bootDevs[def->os.nBootDevs++] = val;
    }
    if (def->os.nBootDevs == 0 && !deviceBoot) {
        def->os.nBootDevs = 1;
        def->os.bootDevs[0] = VIR_DOMAIN_BOOT_DISK;
    }

    bootstr = virXPathString("string(./os/bootmenu[1]/@enable)", ctxt);
    if (bootstr) {
        if (STREQ(bootstr, "yes"))
            def->os.bootmenu = VIR_DOMAIN_BOOT_MENU_ENABLED;
        else
            def->os.bootmenu = VIR_DOMAIN_BOOT_MENU_DISABLED;
        VIR_FREE(bootstr);
    }

    *bootCount = deviceBoot;
    ret = 0;

cleanup:
    VIR_FREE(nodes);
    return ret;
}

static virDomainDefPtr virDomainDefParseXML(virCapsPtr caps,
                                            xmlDocPtr xml,
                                            xmlNodePtr root,
                                            xmlXPathContextPtr ctxt,
                                            int flags)
{
    xmlNodePtr *nodes = NULL, node = NULL;
    char *tmp = NULL;
    int i, n;
    long id = -1;
    virDomainDefPtr def;
    unsigned long count;
    bool uuid_generated = false;
    virBitmapPtr bootMap = NULL;
    unsigned long bootMapSize = 0;

    if (VIR_ALLOC(def) < 0) {
        virReportOOMError();
        return NULL;
    }

    if (!(flags & VIR_DOMAIN_XML_INACTIVE))
        if ((virXPathLong("string(./@id)", ctxt, &id)) < 0)
            id = -1;
    def->id = (int)id;

    /* Find out what type of virtualization to use */
    if (!(tmp = virXPathString("string(./@type)", ctxt))) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                             "%s", _("missing domain type attribute"));
        goto error;
    }

    if ((def->virtType = virDomainVirtTypeFromString(tmp)) < 0) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                             _("invalid domain type %s"), tmp);
        goto error;
    }
    VIR_FREE(tmp);

    /* Extract domain name */
    if (!(def->name = virXPathString("string(./name[1])", ctxt))) {
        virDomainReportError(VIR_ERR_NO_NAME, NULL);
        goto error;
    }

    /* Extract domain uuid. If both uuid and sysinfo/system/entry/uuid
     * exist, they must match; and if only the latter exists, it can
     * also serve as the uuid. */
    tmp = virXPathString("string(./uuid[1])", ctxt);
    if (!tmp) {
        if (virUUIDGenerate(def->uuid)) {
            virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                                 "%s", _("Failed to generate UUID"));
            goto error;
        }
        uuid_generated = true;
    } else {
        if (virUUIDParse(tmp, def->uuid) < 0) {
            virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                                 "%s", _("malformed uuid element"));
            goto error;
        }
        VIR_FREE(tmp);
    }

    /* Extract documentation if present */
    def->description = virXPathString("string(./description[1])", ctxt);

    /* Extract domain memory */
    if (virXPathULong("string(./memory[1])", ctxt,
                      &def->mem.max_balloon) < 0) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                             "%s", _("missing memory element"));
        goto error;
    }

    if (virXPathULong("string(./currentMemory[1])", ctxt,
                      &def->mem.cur_balloon) < 0)
        def->mem.cur_balloon = def->mem.max_balloon;

    node = virXPathNode("./memoryBacking/hugepages", ctxt);
    if (node)
        def->mem.hugepage_backed = 1;

    /* Extract blkio cgroup tunables */
    if (virXPathUInt("string(./blkiotune/weight)", ctxt,
                     &def->blkio.weight) < 0)
        def->blkio.weight = 0;

    /* Extract other memory tunables */
    if (virXPathULong("string(./memtune/hard_limit)", ctxt,
                      &def->mem.hard_limit) < 0)
        def->mem.hard_limit = 0;

    if (virXPathULong("string(./memtune/soft_limit[1])", ctxt,
                      &def->mem.soft_limit) < 0)
        def->mem.soft_limit = 0;

    if (virXPathULong("string(./memtune/min_guarantee[1])", ctxt,
                      &def->mem.min_guarantee) < 0)
        def->mem.min_guarantee = 0;

    if (virXPathULong("string(./memtune/swap_hard_limit[1])", ctxt,
                      &def->mem.swap_hard_limit) < 0)
        def->mem.swap_hard_limit = 0;

    n = virXPathULong("string(./vcpu[1])", ctxt, &count);
    if (n == -2) {
        virDomainReportError(VIR_ERR_XML_ERROR, "%s",
                             _("maximum vcpus must be an integer"));
        goto error;
    } else if (n < 0) {
        def->maxvcpus = 1;
    } else {
        def->maxvcpus = count;
        if (count == 0) {
            virDomainReportError(VIR_ERR_XML_ERROR,
                                 _("invalid maxvcpus %lu"), count);
            goto error;
        }
    }

    n = virXPathULong("string(./vcpu[1]/@current)", ctxt, &count);
    if (n == -2) {
        virDomainReportError(VIR_ERR_XML_ERROR, "%s",
                             _("current vcpus must be an integer"));
        goto error;
    } else if (n < 0) {
        def->vcpus = def->maxvcpus;
    } else {
        def->vcpus = count;
        if (count == 0) {
            virDomainReportError(VIR_ERR_XML_ERROR,
                                 _("invalid current vcpus %lu"), count);
            goto error;
        }

        if (def->maxvcpus < count) {
            virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                _("maxvcpus must not be less than current vcpus (%d < %lu)"),
                def->maxvcpus, count);
            goto error;
        }
    }

    tmp = virXPathString("string(./vcpu[1]/@cpuset)", ctxt);
    if (tmp) {
        char *set = tmp;
        def->cpumasklen = VIR_DOMAIN_CPUMASK_LEN;
        if (VIR_ALLOC_N(def->cpumask, def->cpumasklen) < 0) {
            virReportOOMError();
            goto error;
        }
        if (virDomainCpuSetParse((const char **)&set,
                                 0, def->cpumask,
                                 def->cpumasklen) < 0)
            goto error;
        VIR_FREE(tmp);
    }

    n = virXPathNodeSet("./features/*", ctxt, &nodes);
    if (n < 0)
        goto error;
    if (n) {
        for (i = 0 ; i < n ; i++) {
            int val = virDomainFeatureTypeFromString((const char *)nodes[i]->name);
            if (val < 0) {
                virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                                     _("unexpected feature %s"),
                                     nodes[i]->name);
                goto error;
            }
            def->features |= (1 << val);
        }
        VIR_FREE(nodes);
    }

    if (virDomainLifecycleParseXML(ctxt, "string(./on_reboot[1])",
                                   &def->onReboot, VIR_DOMAIN_LIFECYCLE_RESTART,
                                   virDomainLifecycleTypeFromString) < 0)
        goto error;

    if (virDomainLifecycleParseXML(ctxt, "string(./on_poweroff[1])",
                                   &def->onPoweroff, VIR_DOMAIN_LIFECYCLE_DESTROY,
                                   virDomainLifecycleTypeFromString) < 0)
        goto error;

    if (virDomainLifecycleParseXML(ctxt, "string(./on_crash[1])",
                                        &def->onCrash,
                                   VIR_DOMAIN_LIFECYCLE_CRASH_DESTROY,
                                   virDomainLifecycleCrashTypeFromString) < 0)
        goto error;

    tmp = virXPathString("string(./clock/@offset)", ctxt);
    if (tmp) {
        if ((def->clock.offset = virDomainClockOffsetTypeFromString(tmp)) < 0) {
            virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                                 _("unknown clock offset '%s'"), tmp);
            goto error;
        }
        VIR_FREE(tmp);
    } else {
        def->clock.offset = VIR_DOMAIN_CLOCK_OFFSET_UTC;
    }
    switch (def->clock.offset) {
    case VIR_DOMAIN_CLOCK_OFFSET_VARIABLE:
        if (virXPathLongLong("number(./clock/@adjustment)", ctxt,
                             &def->clock.data.adjustment) < 0)
            def->clock.data.adjustment = 0;
        break;

    case VIR_DOMAIN_CLOCK_OFFSET_TIMEZONE:
        def->clock.data.timezone = virXPathString("string(./clock/@timezone)", ctxt);
        if (!def->clock.data.timezone) {
            virDomainReportError(VIR_ERR_INTERNAL_ERROR, "%s",
                                 _("missing 'timezone' attribute for clock with offset='timezone'"));
            goto error;
        }
        break;
    }

    if ((n = virXPathNodeSet("./clock/timer", ctxt, &nodes)) < 0) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                             "%s", _("failed to parse timers"));
        goto error;
    }
    if (n && VIR_ALLOC_N(def->clock.timers, n) < 0)
        goto no_memory;
    for (i = 0 ; i < n ; i++) {
        virDomainTimerDefPtr timer = virDomainTimerDefParseXML(nodes[i],
                                                               ctxt,
                                                               flags);
        if (!timer)
            goto error;

        def->clock.timers[def->clock.ntimers++] = timer;
    }
    VIR_FREE(nodes);

    def->os.bootloader = virXPathString("string(./bootloader)", ctxt);
    def->os.bootloaderArgs = virXPathString("string(./bootloader_args)", ctxt);

    def->os.type = virXPathString("string(./os/type[1])", ctxt);
    if (!def->os.type) {
        if (def->os.bootloader) {
            def->os.type = strdup("xen");
            if (!def->os.type) {
                virReportOOMError();
                goto error;
            }
        } else {
            virDomainReportError(VIR_ERR_OS_TYPE,
                                 "%s", _("no OS type"));
            goto error;
        }
    }
    /*
     * HACK: For xen driver we previously used bogus 'linux' as the
     * os type for paravirt, whereas capabilities declare it to
     * be 'xen'. So we accept the former and convert
     */
    if (STREQ(def->os.type, "linux") &&
        def->virtType == VIR_DOMAIN_VIRT_XEN) {
        VIR_FREE(def->os.type);
        if (!(def->os.type = strdup("xen"))) {
            virReportOOMError();
            goto error;
        }
    }

    if (!virCapabilitiesSupportsGuestOSType(caps, def->os.type)) {
        virDomainReportError(VIR_ERR_OS_TYPE,
                             "%s", def->os.type);
        goto error;
    }

    def->os.arch = virXPathString("string(./os/type[1]/@arch)", ctxt);
    if (def->os.arch) {
        if (!virCapabilitiesSupportsGuestArch(caps, def->os.type, def->os.arch)) {
            virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                                 _("os type '%s' & arch '%s' combination is not supported"),
                                 def->os.type, def->os.arch);
            goto error;
        }
    } else {
        const char *defaultArch = virCapabilitiesDefaultGuestArch(caps, def->os.type, virDomainVirtTypeToString(def->virtType));
        if (defaultArch == NULL) {
            virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                                 _("no supported architecture for os type '%s'"),
                                 def->os.type);
            goto error;
        }
        if (!(def->os.arch = strdup(defaultArch))) {
            virReportOOMError();
            goto error;
        }
    }

    def->os.machine = virXPathString("string(./os/type[1]/@machine)", ctxt);
    if (!def->os.machine) {
        const char *defaultMachine = virCapabilitiesDefaultGuestMachine(caps,
                                                                        def->os.type,
                                                                        def->os.arch,
                                                                        virDomainVirtTypeToString(def->virtType));
        if (defaultMachine != NULL) {
            if (!(def->os.machine = strdup(defaultMachine))) {
                virReportOOMError();
                goto error;
            }
        }
    }

    /*
     * Booting options for different OS types....
     *
     *   - A bootloader (and optional kernel+initrd)  (xen)
     *   - A kernel + initrd                          (xen)
     *   - A boot device (and optional kernel+initrd) (hvm)
     *   - An init script                             (exe)
     */

    if (STREQ(def->os.type, "exe")) {
        def->os.init = virXPathString("string(./os/init[1])", ctxt);
    }

    if (STREQ(def->os.type, "xen") ||
        STREQ(def->os.type, "hvm") ||
        STREQ(def->os.type, "uml")) {
        def->os.kernel = virXPathString("string(./os/kernel[1])", ctxt);
        def->os.initrd = virXPathString("string(./os/initrd[1])", ctxt);
        def->os.cmdline = virXPathString("string(./os/cmdline[1])", ctxt);
        def->os.root = virXPathString("string(./os/root[1])", ctxt);
        def->os.loader = virXPathString("string(./os/loader[1])", ctxt);
    }

    if (STREQ(def->os.type, "hvm")) {
        if (virDomainDefParseBootXML(ctxt, def, &bootMapSize) < 0)
            goto error;
        if (bootMapSize && !(bootMap = virBitmapAlloc(bootMapSize)))
            goto no_memory;
    }

    def->emulator = virXPathString("string(./devices/emulator[1])", ctxt);
    if (!def->emulator && virCapabilitiesIsEmulatorRequired(caps)) {
        def->emulator = virDomainDefDefaultEmulator(def, caps);
        if (!def->emulator)
            goto error;
    }

    /* analysis of the disk devices */
    if ((n = virXPathNodeSet("./devices/disk", ctxt, &nodes)) < 0) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                             "%s", _("cannot extract disk devices"));
        goto error;
    }
    if (n && VIR_ALLOC_N(def->disks, n) < 0)
        goto no_memory;
    for (i = 0 ; i < n ; i++) {
        virDomainDiskDefPtr disk = virDomainDiskDefParseXML(caps,
                                                            nodes[i],
                                                            bootMap,
                                                            flags);
        if (!disk)
            goto error;

        def->disks[def->ndisks++] = disk;
    }
    VIR_FREE(nodes);

    /* analysis of the controller devices */
    if ((n = virXPathNodeSet("./devices/controller", ctxt, &nodes)) < 0) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                             "%s", _("cannot extract controller devices"));
        goto error;
    }
    if (n && VIR_ALLOC_N(def->controllers, n) < 0)
        goto no_memory;
    for (i = 0 ; i < n ; i++) {
        virDomainControllerDefPtr controller = virDomainControllerDefParseXML(nodes[i],
                                                                              flags);
        if (!controller)
            goto error;

        def->controllers[def->ncontrollers++] = controller;
    }
    VIR_FREE(nodes);

    /* analysis of the filesystems */
    if ((n = virXPathNodeSet("./devices/filesystem", ctxt, &nodes)) < 0) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                             "%s", _("cannot extract filesystem devices"));
        goto error;
    }
    if (n && VIR_ALLOC_N(def->fss, n) < 0)
        goto no_memory;
    for (i = 0 ; i < n ; i++) {
        virDomainFSDefPtr fs = virDomainFSDefParseXML(nodes[i],
                                                      flags);
        if (!fs)
            goto error;

        def->fss[def->nfss++] = fs;
    }
    VIR_FREE(nodes);

    /* analysis of the network devices */
    if ((n = virXPathNodeSet("./devices/interface", ctxt, &nodes)) < 0) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                             "%s", _("cannot extract network devices"));
        goto error;
    }
    if (n && VIR_ALLOC_N(def->nets, n) < 0)
        goto no_memory;
    for (i = 0 ; i < n ; i++) {
        virDomainNetDefPtr net = virDomainNetDefParseXML(caps,
                                                         nodes[i],
                                                         ctxt,
                                                         bootMap,
                                                         flags);
        if (!net)
            goto error;

        def->nets[def->nnets++] = net;
    }
    VIR_FREE(nodes);


    /* analysis of the smartcard devices */
    if ((n = virXPathNodeSet("./devices/smartcard", ctxt, &nodes)) < 0) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                             "%s", _("cannot extract smartcard devices"));
        goto error;
    }
    if (n && VIR_ALLOC_N(def->smartcards, n) < 0)
        goto no_memory;

    for (i = 0 ; i < n ; i++) {
        virDomainSmartcardDefPtr card = virDomainSmartcardDefParseXML(nodes[i],
                                                                      flags);
        if (!card)
            goto error;

        def->smartcards[def->nsmartcards++] = card;
    }
    VIR_FREE(nodes);


    /* analysis of the character devices */
    if ((n = virXPathNodeSet("./devices/parallel", ctxt, &nodes)) < 0) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                             "%s", _("cannot extract parallel devices"));
        goto error;
    }
    if (n && VIR_ALLOC_N(def->parallels, n) < 0)
        goto no_memory;

    for (i = 0 ; i < n ; i++) {
        virDomainChrDefPtr chr = virDomainChrDefParseXML(caps,
                                                         nodes[i],
                                                         flags);
        if (!chr)
            goto error;

        chr->target.port = i;
        def->parallels[def->nparallels++] = chr;
    }
    VIR_FREE(nodes);

    if ((n = virXPathNodeSet("./devices/serial", ctxt, &nodes)) < 0) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                             "%s", _("cannot extract serial devices"));
        goto error;
    }
    if (n && VIR_ALLOC_N(def->serials, n) < 0)
        goto no_memory;

    for (i = 0 ; i < n ; i++) {
        virDomainChrDefPtr chr = virDomainChrDefParseXML(caps,
                                                         nodes[i],
                                                         flags);
        if (!chr)
            goto error;

        chr->target.port = i;
        def->serials[def->nserials++] = chr;
    }
    VIR_FREE(nodes);

    if ((node = virXPathNode("./devices/console[1]", ctxt)) != NULL) {
        virDomainChrDefPtr chr = virDomainChrDefParseXML(caps,
                                                         node,
                                                         flags);
        if (!chr)
            goto error;

        chr->target.port = 0;
        /*
         * For HVM console actually created a serial device
         * while for non-HVM it was a parvirt console
         */
        if (STREQ(def->os.type, "hvm") &&
            chr->targetType == VIR_DOMAIN_CHR_CONSOLE_TARGET_TYPE_SERIAL) {
            if (def->nserials != 0) {
                virDomainChrDefFree(chr);
            } else {
                if (VIR_ALLOC_N(def->serials, 1) < 0) {
                    virDomainChrDefFree(chr);
                    goto no_memory;
                }
                def->nserials = 1;
                def->serials[0] = chr;
                chr->deviceType = VIR_DOMAIN_CHR_DEVICE_TYPE_SERIAL;
            }
        } else {
            def->console = chr;
        }
    }

    if ((n = virXPathNodeSet("./devices/channel", ctxt, &nodes)) < 0) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                             "%s", _("cannot extract channel devices"));
        goto error;
    }
    if (n && VIR_ALLOC_N(def->channels, n) < 0)
        goto no_memory;

    for (i = 0 ; i < n ; i++) {
        virDomainChrDefPtr chr = virDomainChrDefParseXML(caps,
                                                         nodes[i],
                                                         flags);
        if (!chr)
            goto error;

        def->channels[def->nchannels++] = chr;

        if (chr->deviceType == VIR_DOMAIN_CHR_DEVICE_TYPE_CHANNEL &&
            chr->targetType == VIR_DOMAIN_CHR_CHANNEL_TARGET_TYPE_VIRTIO &&
            chr->info.type == VIR_DOMAIN_DEVICE_ADDRESS_TYPE_NONE)
            chr->info.type = VIR_DOMAIN_DEVICE_ADDRESS_TYPE_VIRTIO_SERIAL;

        if (chr->info.type == VIR_DOMAIN_DEVICE_ADDRESS_TYPE_VIRTIO_SERIAL &&
            chr->info.addr.vioserial.port == 0) {
            int maxport = 0;
            int j;
            for (j = 0 ; j < i ; j++) {
                virDomainChrDefPtr thischr = def->channels[j];
                if (thischr->info.type == VIR_DOMAIN_DEVICE_ADDRESS_TYPE_VIRTIO_SERIAL &&
                    thischr->info.addr.vioserial.controller == chr->info.addr.vioserial.controller &&
                    thischr->info.addr.vioserial.bus == chr->info.addr.vioserial.bus &&
                    (int)thischr->info.addr.vioserial.port > maxport)
                    maxport = thischr->info.addr.vioserial.port;
            }
            chr->info.addr.vioserial.port = maxport + 1;
        }
    }
    VIR_FREE(nodes);


    /* analysis of the input devices */
    if ((n = virXPathNodeSet("./devices/input", ctxt, &nodes)) < 0) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                             "%s", _("cannot extract input devices"));
        goto error;
    }
    if (n && VIR_ALLOC_N(def->inputs, n) < 0)
        goto no_memory;

    for (i = 0 ; i < n ; i++) {
        virDomainInputDefPtr input = virDomainInputDefParseXML(def->os.type,
                                                               nodes[i],
                                                               flags);
        if (!input)
            goto error;


        /* With QEMU / KVM / Xen graphics, mouse + PS/2 is implicit
         * with graphics, so don't store it.
         * XXX will this be true for other virt types ? */
        if ((STREQ(def->os.type, "hvm") &&
             input->bus == VIR_DOMAIN_INPUT_BUS_PS2 &&
             input->type == VIR_DOMAIN_INPUT_TYPE_MOUSE) ||
            (STRNEQ(def->os.type, "hvm") &&
             input->bus == VIR_DOMAIN_INPUT_BUS_XEN &&
             input->type == VIR_DOMAIN_INPUT_TYPE_MOUSE)) {
            virDomainInputDefFree(input);
            continue;
        }

        def->inputs[def->ninputs++] = input;
    }
    VIR_FREE(nodes);

    /* analysis of the graphics devices */
    if ((n = virXPathNodeSet("./devices/graphics", ctxt, &nodes)) < 0) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                             "%s", _("cannot extract graphics devices"));
        goto error;
    }
    if (n && VIR_ALLOC_N(def->graphics, n) < 0)
        goto no_memory;
    for (i = 0 ; i < n ; i++) {
        virDomainGraphicsDefPtr graphics = virDomainGraphicsDefParseXML(nodes[i],
                                                                        flags);
        if (!graphics)
            goto error;

        def->graphics[def->ngraphics++] = graphics;
    }
    VIR_FREE(nodes);

    /* If graphics are enabled, there's an implicit PS2 mouse */
    if (def->ngraphics > 0) {
        virDomainInputDefPtr input;

        if (VIR_ALLOC(input) < 0) {
            virReportOOMError();
            goto error;
        }
        if (STREQ(def->os.type, "hvm")) {
            input->type = VIR_DOMAIN_INPUT_TYPE_MOUSE;
            input->bus = VIR_DOMAIN_INPUT_BUS_PS2;
        } else {
            input->type = VIR_DOMAIN_INPUT_TYPE_MOUSE;
            input->bus = VIR_DOMAIN_INPUT_BUS_XEN;
        }

        if (VIR_REALLOC_N(def->inputs, def->ninputs + 1) < 0) {
            virDomainInputDefFree(input);
            goto no_memory;
        }
        def->inputs[def->ninputs] = input;
        def->ninputs++;
    }


    /* analysis of the sound devices */
    if ((n = virXPathNodeSet("./devices/sound", ctxt, &nodes)) < 0) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                             "%s", _("cannot extract sound devices"));
        goto error;
    }
    if (n && VIR_ALLOC_N(def->sounds, n) < 0)
        goto no_memory;
    for (i = 0 ; i < n ; i++) {
        virDomainSoundDefPtr sound = virDomainSoundDefParseXML(nodes[i],
                                                               flags);
        if (!sound)
            goto error;

        def->sounds[def->nsounds++] = sound;
    }
    VIR_FREE(nodes);

    /* analysis of the video devices */
    if ((n = virXPathNodeSet("./devices/video", ctxt, &nodes)) < 0) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                             "%s", _("cannot extract video devices"));
        goto error;
    }
    if (n && VIR_ALLOC_N(def->videos, n) < 0)
        goto no_memory;
    for (i = 0 ; i < n ; i++) {
        virDomainVideoDefPtr video = virDomainVideoDefParseXML(nodes[i],
                                                               def,
                                                               flags);
        if (!video)
            goto error;
        def->videos[def->nvideos++] = video;
    }
    VIR_FREE(nodes);

    /* For backwards compatability, if no <video> tag is set but there
     * is a <graphics> tag, then we add a single video tag */
    if (def->ngraphics && !def->nvideos) {
        virDomainVideoDefPtr video;
        if (VIR_ALLOC(video) < 0)
            goto no_memory;
        video->type = virDomainVideoDefaultType(def);
        if (video->type < 0) {
            virDomainReportError(VIR_ERR_INTERNAL_ERROR, "%s",
                                 _("cannot determine default video type"));
            VIR_FREE(video);
            goto error;
        }
        video->vram = virDomainVideoDefaultRAM(def, video->type);
        video->heads = 1;
        if (VIR_ALLOC_N(def->videos, 1) < 0) {
            virDomainVideoDefFree(video);
            goto no_memory;
        }
        def->videos[def->nvideos++] = video;
    }

    /* analysis of the host devices */
    if ((n = virXPathNodeSet("./devices/hostdev", ctxt, &nodes)) < 0) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                             "%s", _("cannot extract host devices"));
        goto error;
    }
    if (n && VIR_ALLOC_N(def->hostdevs, n) < 0)
        goto no_memory;
    for (i = 0 ; i < n ; i++) {
        virDomainHostdevDefPtr hostdev = virDomainHostdevDefParseXML(nodes[i],
                                                                     bootMap,
                                                                     flags);
        if (!hostdev)
            goto error;

        def->hostdevs[def->nhostdevs++] = hostdev;
    }
    VIR_FREE(nodes);

    /* analysis of the watchdog devices */
    def->watchdog = NULL;
    if ((n = virXPathNodeSet("./devices/watchdog", ctxt, &nodes)) < 0) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                             "%s", _("cannot extract watchdog devices"));
        goto error;
    }
    if (n > 1) {
        virDomainReportError (VIR_ERR_INTERNAL_ERROR,
                              "%s", _("only a single watchdog device is supported"));
        goto error;
    }
    if (n > 0) {
        virDomainWatchdogDefPtr watchdog =
            virDomainWatchdogDefParseXML(nodes[0], flags);
        if (!watchdog)
            goto error;

        def->watchdog = watchdog;
        VIR_FREE(nodes);
    }

    /* analysis of the memballoon devices */
    def->memballoon = NULL;
    if ((n = virXPathNodeSet("./devices/memballoon", ctxt, &nodes)) < 0) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                             "%s", _("cannot extract memory balloon devices"));
        goto error;
    }
    if (n > 1) {
        virDomainReportError (VIR_ERR_INTERNAL_ERROR,
                              "%s", _("only a single memory balloon device is supported"));
        goto error;
    }
    if (n > 0) {
        virDomainMemballoonDefPtr memballoon =
            virDomainMemballoonDefParseXML(nodes[0], flags);
        if (!memballoon)
            goto error;

        def->memballoon = memballoon;
        VIR_FREE(nodes);
    } else {
        if (def->virtType == VIR_DOMAIN_VIRT_XEN ||
            def->virtType == VIR_DOMAIN_VIRT_QEMU ||
            def->virtType == VIR_DOMAIN_VIRT_KQEMU ||
            def->virtType == VIR_DOMAIN_VIRT_KVM) {
            virDomainMemballoonDefPtr memballoon;
            if (VIR_ALLOC(memballoon) < 0)
                goto no_memory;
            memballoon->model = def->virtType == VIR_DOMAIN_VIRT_XEN ?
                VIR_DOMAIN_MEMBALLOON_MODEL_XEN :
                VIR_DOMAIN_MEMBALLOON_MODEL_VIRTIO;
            def->memballoon = memballoon;
        }
    }

    /* analysis of security label */
    if (virSecurityLabelDefParseXML(def, ctxt, flags) == -1)
        goto error;

    if ((node = virXPathNode("./cpu[1]", ctxt)) != NULL) {
        xmlNodePtr oldnode = ctxt->node;
        ctxt->node = node;
        def->cpu = virCPUDefParseXML(node, ctxt, VIR_CPU_TYPE_GUEST);
        ctxt->node = oldnode;

        if (def->cpu == NULL)
            goto error;
    }

    if ((node = virXPathNode("./sysinfo[1]", ctxt)) != NULL) {
        xmlNodePtr oldnode = ctxt->node;
        ctxt->node = node;
        def->sysinfo = virSysinfoParseXML(node, ctxt);
        ctxt->node = oldnode;

        if (def->sysinfo == NULL)
            goto error;
        if (def->sysinfo->system_uuid != NULL) {
            unsigned char uuidbuf[VIR_UUID_BUFLEN];
            if (virUUIDParse(def->sysinfo->system_uuid, uuidbuf) < 0) {
                virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                                     "%s", _("malformed uuid element"));
                goto error;
            }
            if (uuid_generated)
                memcpy(def->uuid, uuidbuf, VIR_UUID_BUFLEN);
            else if (memcmp(def->uuid, uuidbuf, VIR_UUID_BUFLEN) != 0) {
                virDomainReportError(VIR_ERR_INTERNAL_ERROR, "%s",
                                     _("UUID mismatch between <uuid> and "
                                       "<sysinfo>"));
                goto error;
            }
        }
    }
    tmp = virXPathString("string(./os/smbios/@mode)", ctxt);
    if (tmp) {
        int mode;

        if ((mode = virDomainSmbiosModeTypeFromString(tmp)) < 0) {
            virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                                 _("unknown smbios mode '%s'"), tmp);
            goto error;
        }
        def->os.smbios_mode = mode;
        VIR_FREE(tmp);
    } else {
        def->os.smbios_mode = VIR_DOMAIN_SMBIOS_NONE; /* not present */
    }

    /* we have to make a copy of all of the callback pointers here since
     * we won't have the virCaps structure available during free
     */
    def->ns = caps->ns;

    if (def->ns.parse) {
        if ((def->ns.parse)(xml, root, ctxt, &def->namespaceData) < 0)
            goto error;
    }

    /* Auto-add any implied controllers which aren't present
     */
    if (virDomainDefAddImplicitControllers(def) < 0)
        goto error;

    virBitmapFree(bootMap);

    return def;

no_memory:
    virReportOOMError();
    /* fallthrough */

 error:
    VIR_FREE(tmp);
    VIR_FREE(nodes);
    virBitmapFree(bootMap);
    virDomainDefFree(def);
    return NULL;
}


static virDomainObjPtr virDomainObjParseXML(virCapsPtr caps,
                                            xmlDocPtr xml,
                                            xmlXPathContextPtr ctxt)
{
    char *tmp = NULL;
    long val;
    xmlNodePtr config;
    xmlNodePtr oldnode;
    virDomainObjPtr obj;

    if (!(obj = virDomainObjNew(caps)))
        return NULL;

    if (!(config = virXPathNode("./domain", ctxt))) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                             "%s", _("no domain config"));
        goto error;
    }

    oldnode = ctxt->node;
    ctxt->node = config;
    obj->def = virDomainDefParseXML(caps, xml, config, ctxt,
                                    VIR_DOMAIN_XML_INTERNAL_STATUS);
    ctxt->node = oldnode;
    if (!obj->def)
        goto error;

    if (!(tmp = virXPathString("string(./@state)", ctxt))) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                             "%s", _("missing domain state"));
        goto error;
    }
    if ((obj->state = virDomainStateTypeFromString(tmp)) < 0) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                             _("invalid domain state '%s'"), tmp);
        VIR_FREE(tmp);
        goto error;
    }
    VIR_FREE(tmp);

    if ((virXPathLong("string(./@pid)", ctxt, &val)) < 0) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                             "%s", _("invalid pid"));
        goto error;
    }
    obj->pid = (pid_t)val;

    if (caps->privateDataXMLParse &&
        ((caps->privateDataXMLParse)(ctxt, obj->privateData)) < 0)
        goto error;

    return obj;

error:
    virDomainObjUnref(obj);
    return NULL;
}


static virDomainDefPtr
virDomainDefParse(const char *xmlStr,
                  const char *filename,
                  virCapsPtr caps,
                  int flags)
{
    xmlDocPtr xml;
    virDomainDefPtr def = NULL;

    if ((xml = virXMLParse(filename, xmlStr, "domain.xml"))) {
        def = virDomainDefParseNode(caps, xml, xmlDocGetRootElement(xml), flags);
        xmlFreeDoc(xml);
    }

    return def;
}

virDomainDefPtr virDomainDefParseString(virCapsPtr caps,
                                        const char *xmlStr,
                                        int flags)
{
    return virDomainDefParse(xmlStr, NULL, caps, flags);
}

virDomainDefPtr virDomainDefParseFile(virCapsPtr caps,
                                      const char *filename,
                                      int flags)
{
    return virDomainDefParse(NULL, filename, caps, flags);
}


virDomainDefPtr virDomainDefParseNode(virCapsPtr caps,
                                      xmlDocPtr xml,
                                      xmlNodePtr root,
                                      int flags)
{
    xmlXPathContextPtr ctxt = NULL;
    virDomainDefPtr def = NULL;

    if (!xmlStrEqual(root->name, BAD_CAST "domain")) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                              "%s", _("incorrect root element"));
        goto cleanup;
    }

    ctxt = xmlXPathNewContext(xml);
    if (ctxt == NULL) {
        virReportOOMError();
        goto cleanup;
    }

    ctxt->node = root;
    def = virDomainDefParseXML(caps, xml, root, ctxt, flags);

cleanup:
    xmlXPathFreeContext(ctxt);
    return def;
}


virDomainObjPtr virDomainObjParseFile(virCapsPtr caps,
                                      const char *filename)
{
    xmlDocPtr xml;
    virDomainObjPtr obj = NULL;

    if ((xml = virXMLParseFile(filename))) {
        obj = virDomainObjParseNode(caps, xml, xmlDocGetRootElement(xml));
        xmlFreeDoc(xml);
    }

    return obj;
}


virDomainObjPtr virDomainObjParseNode(virCapsPtr caps,
                                      xmlDocPtr xml,
                                      xmlNodePtr root)
{
    xmlXPathContextPtr ctxt = NULL;
    virDomainObjPtr obj = NULL;

    if (!xmlStrEqual(root->name, BAD_CAST "domstatus")) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                             "%s", _("incorrect root element"));
        goto cleanup;
    }

    ctxt = xmlXPathNewContext(xml);
    if (ctxt == NULL) {
        virReportOOMError();
        goto cleanup;
    }

    ctxt->node = root;
    obj = virDomainObjParseXML(caps, xml, ctxt);

cleanup:
    xmlXPathFreeContext(ctxt);
    return obj;
}

static int virDomainDefMaybeAddController(virDomainDefPtr def,
                                          int type,
                                          int idx)
{
    int found = 0;
    int i;
    virDomainControllerDefPtr cont;

    for (i = 0 ; (i < def->ncontrollers) && !found; i++) {
        if (def->controllers[i]->type == type &&
            def->controllers[i]->idx == idx)
            found = 1;
    }

    if (found)
        return 0;

    if (VIR_ALLOC(cont) < 0) {
        virReportOOMError();
        return -1;
    }

    cont->type = type;
    cont->idx = idx;
    cont->model = -1;

    if (cont->type == VIR_DOMAIN_CONTROLLER_TYPE_VIRTIO_SERIAL) {
        cont->opts.vioserial.ports = -1;
        cont->opts.vioserial.vectors = -1;
    }


    if (VIR_REALLOC_N(def->controllers, def->ncontrollers+1) < 0) {
        VIR_FREE(cont);
        virReportOOMError();
        return -1;
    }
    def->controllers[def->ncontrollers] = cont;
    def->ncontrollers++;

    return 0;
}

static int virDomainDefAddDiskControllersForType(virDomainDefPtr def,
                                                 int controllerType,
                                                 int diskBus)
{
    int i;
    int maxController = -1;

    for (i = 0 ; i < def->ndisks ; i++) {
        if (def->disks[i]->bus != diskBus)
            continue;

        if (def->disks[i]->info.type != VIR_DOMAIN_DEVICE_ADDRESS_TYPE_DRIVE)
            continue;

        if ((int)def->disks[i]->info.addr.drive.controller > maxController)
            maxController = def->disks[i]->info.addr.drive.controller;
    }

    for (i = 0 ; i <= maxController ; i++) {
        if (virDomainDefMaybeAddController(def, controllerType, i) < 0)
            return -1;
    }

    return 0;
}


static int virDomainDefMaybeAddVirtioSerialController(virDomainDefPtr def)
{
    /* Look for any virtio serial or virtio console devs */
    int i;

    for (i = 0 ; i < def->nchannels ; i++) {
        virDomainChrDefPtr channel = def->channels[i];

        if (channel->targetType == VIR_DOMAIN_CHR_CHANNEL_TARGET_TYPE_VIRTIO) {
            int idx = 0;
            if (channel->info.type == VIR_DOMAIN_DEVICE_ADDRESS_TYPE_VIRTIO_SERIAL)
                idx = channel->info.addr.vioserial.controller;

            if (virDomainDefMaybeAddController(def,
                VIR_DOMAIN_CONTROLLER_TYPE_VIRTIO_SERIAL, idx) < 0)
                return -1;
        }
    }

    if (def->console) {
        virDomainChrDefPtr console = def->console;

        if (console->targetType == VIR_DOMAIN_CHR_CONSOLE_TARGET_TYPE_VIRTIO) {
            int idx = 0;
            if (console->info.type ==
                VIR_DOMAIN_DEVICE_ADDRESS_TYPE_VIRTIO_SERIAL)
                idx = console->info.addr.vioserial.controller;

            if (virDomainDefMaybeAddController(def,
                VIR_DOMAIN_CONTROLLER_TYPE_VIRTIO_SERIAL, idx) < 0)
                return -1;
        }
    }

    return 0;
}


static int
virDomainDefMaybeAddSmartcardController(virDomainDefPtr def)
{
    /* Look for any smartcard devs */
    int i;

    for (i = 0 ; i < def->nsmartcards ; i++) {
        virDomainSmartcardDefPtr smartcard = def->smartcards[i];
        int idx = 0;

        if (smartcard->info.type == VIR_DOMAIN_DEVICE_ADDRESS_TYPE_CCID) {
            idx = smartcard->info.addr.ccid.controller;
        } else if (smartcard->info.type
                   == VIR_DOMAIN_DEVICE_ADDRESS_TYPE_NONE) {
            int j;
            int max = -1;

            for (j = 0; j < def->nsmartcards; j++) {
                virDomainDeviceInfoPtr info = &def->smartcards[j]->info;
                if (info->type == VIR_DOMAIN_DEVICE_ADDRESS_TYPE_CCID &&
                    info->addr.ccid.controller == 0 &&
                    (int) info->addr.ccid.slot > max)
                    max = info->addr.ccid.slot;
            }
            smartcard->info.type = VIR_DOMAIN_DEVICE_ADDRESS_TYPE_CCID;
            smartcard->info.addr.ccid.controller = 0;
            smartcard->info.addr.ccid.slot = max + 1;
        }

        if (virDomainDefMaybeAddController(def,
                                           VIR_DOMAIN_CONTROLLER_TYPE_CCID,
                                           idx) < 0)
            return -1;
    }

    return 0;
}


/*
 * Based on the declared <address/> info for any devices,
 * add neccessary drive controllers which are not already present
 * in the XML. This is for compat with existing apps which will
 * not know/care about <controller> info in the XML
 */
int virDomainDefAddImplicitControllers(virDomainDefPtr def)
{
    if (virDomainDefAddDiskControllersForType(def,
                                              VIR_DOMAIN_CONTROLLER_TYPE_SCSI,
                                              VIR_DOMAIN_DISK_BUS_SCSI) < 0)
        return -1;

    if (virDomainDefAddDiskControllersForType(def,
                                              VIR_DOMAIN_CONTROLLER_TYPE_FDC,
                                              VIR_DOMAIN_DISK_BUS_FDC) < 0)
        return -1;

    if (virDomainDefAddDiskControllersForType(def,
                                              VIR_DOMAIN_CONTROLLER_TYPE_IDE,
                                              VIR_DOMAIN_DISK_BUS_IDE) < 0)
        return -1;

    if (virDomainDefMaybeAddVirtioSerialController(def) < 0)
        return -1;

    if (virDomainDefMaybeAddSmartcardController(def) < 0)
        return -1;

    return 0;
}


/************************************************************************
 *                                                                        *
 * Parser and converter for the CPUset strings used in libvirt                *
 *                                                                        *
 ************************************************************************/
/**
 * virDomainCpuNumberParse
 * @str: pointer to the char pointer used
 * @maxcpu: maximum CPU number allowed
 *
 * Parse a CPU number
 *
 * Returns the CPU number or -1 in case of error. @str will be
 *         updated to skip the number.
 */
static int
virDomainCpuNumberParse(const char **str, int maxcpu)
{
    int ret = 0;
    const char *cur = *str;

    if (!c_isdigit(*cur))
        return (-1);

    while (c_isdigit(*cur)) {
        ret = ret * 10 + (*cur - '0');
        if (ret >= maxcpu)
            return (-1);
        cur++;
    }
    *str = cur;
    return (ret);
}

/**
 * virDomainCpuSetFormat:
 * @conn: connection
 * @cpuset: pointer to a char array for the CPU set
 * @maxcpu: number of elements available in @cpuset
 *
 * Serialize the cpuset to a string
 *
 * Returns the new string NULL in case of error. The string needs to be
 *         freed by the caller.
 */
char *
virDomainCpuSetFormat(char *cpuset, int maxcpu)
{
    virBuffer buf = VIR_BUFFER_INITIALIZER;
    int start, cur;
    int first = 1;

    if ((cpuset == NULL) || (maxcpu <= 0) || (maxcpu > 100000))
        return (NULL);

    cur = 0;
    start = -1;
    while (cur < maxcpu) {
        if (cpuset[cur]) {
            if (start == -1)
                start = cur;
        } else if (start != -1) {
            if (!first)
                virBufferAddLit(&buf, ",");
            else
                first = 0;
            if (cur == start + 1)
                virBufferVSprintf(&buf, "%d", start);
            else
                virBufferVSprintf(&buf, "%d-%d", start, cur - 1);
            start = -1;
        }
        cur++;
    }
    if (start != -1) {
        if (!first)
            virBufferAddLit(&buf, ",");
        if (maxcpu == start + 1)
            virBufferVSprintf(&buf, "%d", start);
        else
            virBufferVSprintf(&buf, "%d-%d", start, maxcpu - 1);
    }

    if (virBufferError(&buf)) {
        virBufferFreeAndReset(&buf);
        virReportOOMError();
        return NULL;
    }

    return virBufferContentAndReset(&buf);
}

/**
 * virDomainCpuSetParse:
 * @conn: connection
 * @str: pointer to a CPU set string pointer
 * @sep: potential character used to mark the end of string if not 0
 * @cpuset: pointer to a char array for the CPU set
 * @maxcpu: number of elements available in @cpuset
 *
 * Parse the cpu set, it will set the value for enabled CPUs in the @cpuset
 * to 1, and 0 otherwise. The syntax allows comma separated entries; each
 * can be either a CPU number, ^N to unset that CPU, or N-M for ranges.
 *
 * Returns the number of CPU found in that set, or -1 in case of error.
 *         @cpuset is modified accordingly to the value parsed.
 *         @str is updated to the end of the part parsed
 */
int
virDomainCpuSetParse(const char **str, char sep,
                     char *cpuset, int maxcpu)
{
    const char *cur;
    int ret = 0;
    int i, start, last;
    int neg = 0;

    if ((str == NULL) || (cpuset == NULL) || (maxcpu <= 0) ||
        (maxcpu > 100000))
        return (-1);

    cur = *str;
    virSkipSpaces(&cur);
    if (*cur == 0)
        goto parse_error;

    /* initialize cpumap to all 0s */
    for (i = 0; i < maxcpu; i++)
        cpuset[i] = 0;
    ret = 0;

    while ((*cur != 0) && (*cur != sep)) {
        /*
         * 3 constructs are allowed:
         *     - N   : a single CPU number
         *     - N-M : a range of CPU numbers with N < M
         *     - ^N  : remove a single CPU number from the current set
         */
        if (*cur == '^') {
            cur++;
            neg = 1;
        }

        if (!c_isdigit(*cur))
            goto parse_error;
        start = virDomainCpuNumberParse(&cur, maxcpu);
        if (start < 0)
            goto parse_error;
        virSkipSpaces(&cur);
        if ((*cur == ',') || (*cur == 0) || (*cur == sep)) {
            if (neg) {
                if (cpuset[start] == 1) {
                    cpuset[start] = 0;
                    ret--;
                }
            } else {
                if (cpuset[start] == 0) {
                    cpuset[start] = 1;
                    ret++;
                }
            }
        } else if (*cur == '-') {
            if (neg)
                goto parse_error;
            cur++;
            virSkipSpaces(&cur);
            last = virDomainCpuNumberParse(&cur, maxcpu);
            if (last < start)
                goto parse_error;
            for (i = start; i <= last; i++) {
                if (cpuset[i] == 0) {
                    cpuset[i] = 1;
                    ret++;
                }
            }
            virSkipSpaces(&cur);
        }
        if (*cur == ',') {
            cur++;
            virSkipSpaces(&cur);
            neg = 0;
        } else if ((*cur == 0) || (*cur == sep)) {
            break;
        } else
            goto parse_error;
    }
    *str = cur;
    return (ret);

  parse_error:
    virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                         "%s", _("topology cpuset syntax error"));
    return (-1);
}


static int
virDomainLifecycleDefFormat(virBufferPtr buf,
                            int type,
                            const char *name,
                            virLifecycleToStringFunc convFunc)
{
    const char *typeStr = convFunc(type);
    if (!typeStr) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                             _("unexpected lifecycle type %d"), type);
        return -1;
    }

    virBufferVSprintf(buf, "  <%s>%s</%s>\n", name, typeStr, name);

    return 0;
}


static int
virDomainDiskDefFormat(virBufferPtr buf,
                       virDomainDiskDefPtr def,
                       int flags)
{
    const char *type = virDomainDiskTypeToString(def->type);
    const char *device = virDomainDiskDeviceTypeToString(def->device);
    const char *bus = virDomainDiskBusTypeToString(def->bus);
    const char *cachemode = virDomainDiskCacheTypeToString(def->cachemode);
    const char *error_policy = virDomainDiskErrorPolicyTypeToString(def->error_policy);
    const char *iomode = virDomainDiskIoTypeToString(def->iomode);

    if (!type) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                             _("unexpected disk type %d"), def->type);
        return -1;
    }
    if (!device) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                             _("unexpected disk device %d"), def->device);
        return -1;
    }
    if (!bus) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                             _("unexpected disk bus %d"), def->bus);
        return -1;
    }
    if (!cachemode) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                             _("unexpected disk cache mode %d"), def->cachemode);
        return -1;
    }
    if (!iomode) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                             _("unexpected disk io mode %d"), def->iomode);
        return -1;
    }

    virBufferVSprintf(buf,
                      "    <disk type='%s' device='%s'>\n",
                      type, device);

    if (def->driverName || def->driverType || def->cachemode) {
        virBufferVSprintf(buf, "      <driver");
        if (def->driverName)
            virBufferVSprintf(buf, " name='%s'", def->driverName);
        if (def->driverType)
            virBufferVSprintf(buf, " type='%s'", def->driverType);
        if (def->cachemode)
            virBufferVSprintf(buf, " cache='%s'", cachemode);
        if (def->error_policy)
            virBufferVSprintf(buf, " error_policy='%s'", error_policy);
        if (def->iomode)
            virBufferVSprintf(buf, " io='%s'", iomode);
        virBufferVSprintf(buf, "/>\n");
    }

    if (def->src || def->nhosts > 0) {
        switch (def->type) {
        case VIR_DOMAIN_DISK_TYPE_FILE:
            virBufferEscapeString(buf, "      <source file='%s'/>\n",
                                  def->src);
            break;
        case VIR_DOMAIN_DISK_TYPE_BLOCK:
            virBufferEscapeString(buf, "      <source dev='%s'/>\n",
                                  def->src);
            break;
        case VIR_DOMAIN_DISK_TYPE_DIR:
            virBufferEscapeString(buf, "      <source dir='%s'/>\n",
                                  def->src);
            break;
        case VIR_DOMAIN_DISK_TYPE_NETWORK:
            virBufferVSprintf(buf, "      <source protocol='%s'",
                              virDomainDiskProtocolTypeToString(def->protocol));
            if (def->src) {
                virBufferEscapeString(buf, " name='%s'", def->src);
            }
            if (def->nhosts == 0) {
                virBufferVSprintf(buf, "/>\n");
            } else {
                int i;

                virBufferVSprintf(buf, ">\n");
                for (i = 0; i < def->nhosts; i++) {
                    virBufferEscapeString(buf, "        <host name='%s'",
                                          def->hosts[i].name);
                    virBufferEscapeString(buf, " port='%s'/>\n",
                                          def->hosts[i].port);
                }
                virBufferVSprintf(buf, "      </source>\n");
            }
            break;
        default:
            virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                                 _("unexpected disk type %s"),
                                 virDomainDiskTypeToString(def->type));
            return -1;
        }
    }

    virBufferVSprintf(buf, "      <target dev='%s' bus='%s'/>\n",
                      def->dst, bus);

    if (def->bootIndex)
        virBufferVSprintf(buf, "      <boot order='%d'/>\n", def->bootIndex);
    if (def->readonly)
        virBufferAddLit(buf, "      <readonly/>\n");
    if (def->shared)
        virBufferAddLit(buf, "      <shareable/>\n");
    if (def->serial)
        virBufferEscapeString(buf, "      <serial>%s</serial>\n",
                              def->serial);
    if (def->encryption != NULL &&
        virStorageEncryptionFormat(buf, def->encryption, 6) < 0)
        return -1;

    if (virDomainDeviceInfoFormat(buf, &def->info, flags) < 0)
        return -1;

    virBufferAddLit(buf, "    </disk>\n");

    return 0;
}

static int
virDomainControllerDefFormat(virBufferPtr buf,
                             virDomainControllerDefPtr def,
                             int flags)
{
    const char *type = virDomainControllerTypeToString(def->type);
    const char *model = NULL;

    if (!type) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                             _("unexpected controller type %d"), def->type);
        return -1;
    }

    if (def->model != -1) {
        model = virDomainControllerModelTypeToString(def->model);

        if (!model) {
            virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                                 _("unexpected model type %d"), def->model);
            return -1;
        }
    }

    virBufferVSprintf(buf,
                      "    <controller type='%s' index='%d'",
                      type, def->idx);

    if (model) {
        virBufferEscapeString(buf, " model='%s'", model);
    }

    switch (def->type) {
    case VIR_DOMAIN_CONTROLLER_TYPE_VIRTIO_SERIAL:
        if (def->opts.vioserial.ports != -1) {
            virBufferVSprintf(buf, " ports='%d'",
                              def->opts.vioserial.ports);
        }
        if (def->opts.vioserial.vectors != -1) {
            virBufferVSprintf(buf, " vectors='%d'",
                              def->opts.vioserial.vectors);
        }
        break;

    default:
        break;
    }

    if (virDomainDeviceInfoIsSet(&def->info)) {
        virBufferAddLit(buf, ">\n");
        if (virDomainDeviceInfoFormat(buf, &def->info, flags) < 0)
            return -1;
        virBufferAddLit(buf, "    </controller>\n");
    } else {
        virBufferAddLit(buf, "/>\n");
    }

    return 0;
}

static int
virDomainFSDefFormat(virBufferPtr buf,
                     virDomainFSDefPtr def,
                     int flags)
{
    const char *type = virDomainFSTypeToString(def->type);
    const char *accessmode = virDomainFSAccessModeTypeToString(def->accessmode);

    if (!type) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                             _("unexpected filesystem type %d"), def->type);
        return -1;
    }

   if (!accessmode) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                             _("unexpected accessmode %d"), def->accessmode);
        return -1;
    }


    virBufferVSprintf(buf,
                      "    <filesystem type='%s' accessmode='%s'>\n",
                      type, accessmode);

    if (def->src) {
        switch (def->type) {
        case VIR_DOMAIN_FS_TYPE_MOUNT:
            virBufferEscapeString(buf, "      <source dir='%s'/>\n",
                                  def->src);
            break;

        case VIR_DOMAIN_FS_TYPE_BLOCK:
            virBufferEscapeString(buf, "      <source dev='%s'/>\n",
                                  def->src);
            break;

        case VIR_DOMAIN_FS_TYPE_FILE:
            virBufferEscapeString(buf, "      <source file='%s'/>\n",
                                  def->src);
            break;

        case VIR_DOMAIN_FS_TYPE_TEMPLATE:
            virBufferEscapeString(buf, "      <source name='%s'/>\n",
                                  def->src);
        }
    }

    virBufferVSprintf(buf, "      <target dir='%s'/>\n",
                      def->dst);

    if (def->readonly)
        virBufferAddLit(buf, "      <readonly/>\n");

    if (virDomainDeviceInfoFormat(buf, &def->info, flags) < 0)
        return -1;

    virBufferAddLit(buf, "    </filesystem>\n");

    return 0;
}

static int
virDomainNetDefFormat(virBufferPtr buf,
                      virDomainNetDefPtr def,
                      int flags)
{
    const char *type = virDomainNetTypeToString(def->type);
    char *attrs;

    if (!type) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                             _("unexpected net type %d"), def->type);
        return -1;
    }

    virBufferVSprintf(buf, "    <interface type='%s'>\n", type);

    virBufferVSprintf(buf,
                      "      <mac address='%02x:%02x:%02x:%02x:%02x:%02x'/>\n",
                      def->mac[0], def->mac[1], def->mac[2],
                      def->mac[3], def->mac[4], def->mac[5]);

    switch (def->type) {
    case VIR_DOMAIN_NET_TYPE_NETWORK:
        virBufferEscapeString(buf, "      <source network='%s'/>\n",
                              def->data.network.name);
        break;

    case VIR_DOMAIN_NET_TYPE_ETHERNET:
        if (def->data.ethernet.dev)
            virBufferEscapeString(buf, "      <source dev='%s'/>\n",
                                  def->data.ethernet.dev);
        if (def->data.ethernet.ipaddr)
            virBufferVSprintf(buf, "      <ip address='%s'/>\n",
                              def->data.ethernet.ipaddr);
        if (def->data.ethernet.script)
            virBufferEscapeString(buf, "      <script path='%s'/>\n",
                                  def->data.ethernet.script);
        break;

    case VIR_DOMAIN_NET_TYPE_BRIDGE:
        virBufferEscapeString(buf, "      <source bridge='%s'/>\n",
                              def->data.bridge.brname);
        if (def->data.bridge.ipaddr)
            virBufferVSprintf(buf, "      <ip address='%s'/>\n",
                              def->data.bridge.ipaddr);
        if (def->data.bridge.script)
            virBufferEscapeString(buf, "      <script path='%s'/>\n",
                                  def->data.bridge.script);
        break;

    case VIR_DOMAIN_NET_TYPE_SERVER:
    case VIR_DOMAIN_NET_TYPE_CLIENT:
    case VIR_DOMAIN_NET_TYPE_MCAST:
        if (def->data.socket.address)
            virBufferVSprintf(buf, "      <source address='%s' port='%d'/>\n",
                              def->data.socket.address, def->data.socket.port);
        else
            virBufferVSprintf(buf, "      <source port='%d'/>\n",
                              def->data.socket.port);
        break;

    case VIR_DOMAIN_NET_TYPE_INTERNAL:
        virBufferEscapeString(buf, "      <source name='%s'/>\n",
                              def->data.internal.name);
        break;

    case VIR_DOMAIN_NET_TYPE_DIRECT:
        virBufferEscapeString(buf, "      <source dev='%s'",
                              def->data.direct.linkdev);
        virBufferVSprintf(buf, " mode='%s'",
                   virDomainNetdevMacvtapTypeToString(def->data.direct.mode));
        virBufferAddLit(buf, "/>\n");
        virVirtualPortProfileFormat(buf, &def->data.direct.virtPortProfile,
                                    "      ");
        break;

    case VIR_DOMAIN_NET_TYPE_USER:
    case VIR_DOMAIN_NET_TYPE_LAST:
        break;
    }

    if (def->ifname)
        virBufferEscapeString(buf, "      <target dev='%s'/>\n",
                              def->ifname);
    if (def->model) {
        virBufferEscapeString(buf, "      <model type='%s'/>\n",
                              def->model);
        if (STREQ(def->model, "virtio") && def->backend) {
            virBufferVSprintf(buf, "      <driver name='%s'/>\n",
                              virDomainNetBackendTypeToString(def->backend));
        }
    }
    if (def->filter) {
        virBufferEscapeString(buf, "      <filterref filter='%s'",
                              def->filter);
        attrs = virNWFilterFormatParamAttributes(def->filterparams,
                                                 "        ");
        if (!attrs || strlen(attrs) <= 1)
            virBufferAddLit(buf, "/>\n");
        else
            virBufferVSprintf(buf, ">\n%s      </filterref>\n", attrs);
        VIR_FREE(attrs);
    }
    if (def->bootIndex)
        virBufferVSprintf(buf, "      <boot order='%d'/>\n", def->bootIndex);

    if (def->tune.sndbuf_specified) {
        virBufferAddLit(buf,   "      <tune>\n");
        virBufferVSprintf(buf, "        <sndbuf>%lu</sndbuf>\n", def->tune.sndbuf);
        virBufferAddLit(buf,   "      </tune>\n");
    }

    if (virDomainDeviceInfoFormat(buf, &def->info, flags) < 0)
        return -1;

    virBufferAddLit(buf, "    </interface>\n");

    return 0;
}


/* Assumes that "<device" has already been generated, and starts
 * output at " type='type'>". */
static int
virDomainChrSourceDefFormat(virBufferPtr buf,
                            virDomainChrSourceDefPtr def,
                            bool tty_compat,
                            int flags)
{
    const char *type = virDomainChrTypeToString(def->type);

    if (!type) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                             _("unexpected char type %d"), def->type);
        return -1;
    }

    /* Compat with legacy  <console tty='/dev/pts/5'/> syntax */
    virBufferVSprintf(buf, " type='%s'", type);
    if (tty_compat) {
        virBufferEscapeString(buf, " tty='%s'",
                              def->data.file.path);
    }
    virBufferAddLit(buf, ">\n");

    switch (def->type) {
    case VIR_DOMAIN_CHR_TYPE_NULL:
    case VIR_DOMAIN_CHR_TYPE_VC:
    case VIR_DOMAIN_CHR_TYPE_STDIO:
    case VIR_DOMAIN_CHR_TYPE_SPICEVMC:
        /* nada */
        break;

    case VIR_DOMAIN_CHR_TYPE_PTY:
    case VIR_DOMAIN_CHR_TYPE_DEV:
    case VIR_DOMAIN_CHR_TYPE_FILE:
    case VIR_DOMAIN_CHR_TYPE_PIPE:
        if (def->type != VIR_DOMAIN_CHR_TYPE_PTY ||
            (def->data.file.path &&
             !(flags & VIR_DOMAIN_XML_INACTIVE))) {
            virBufferEscapeString(buf, "      <source path='%s'/>\n",
                                  def->data.file.path);
        }
        break;

    case VIR_DOMAIN_CHR_TYPE_UDP:
        if (def->data.udp.bindService &&
            def->data.udp.bindHost) {
            virBufferVSprintf(buf,
                              "      <source mode='bind' host='%s' "
                              "service='%s'/>\n",
                              def->data.udp.bindHost,
                              def->data.udp.bindService);
        } else if (def->data.udp.bindHost) {
            virBufferVSprintf(buf, "      <source mode='bind' host='%s'/>\n",
                              def->data.udp.bindHost);
        } else if (def->data.udp.bindService) {
            virBufferVSprintf(buf, "      <source mode='bind' service='%s'/>\n",
                              def->data.udp.bindService);
        }

        if (def->data.udp.connectService &&
            def->data.udp.connectHost) {
            virBufferVSprintf(buf,
                              "      <source mode='connect' host='%s' "
                              "service='%s'/>\n",
                              def->data.udp.connectHost,
                              def->data.udp.connectService);
        } else if (def->data.udp.connectHost) {
            virBufferVSprintf(buf, "      <source mode='connect' host='%s'/>\n",
                              def->data.udp.connectHost);
        } else if (def->data.udp.connectService) {
            virBufferVSprintf(buf,
                              "      <source mode='connect' service='%s'/>\n",
                              def->data.udp.connectService);
        }
        break;

    case VIR_DOMAIN_CHR_TYPE_TCP:
        virBufferVSprintf(buf,
                          "      <source mode='%s' host='%s' service='%s'/>\n",
                          def->data.tcp.listen ? "bind" : "connect",
                          def->data.tcp.host,
                          def->data.tcp.service);
        virBufferVSprintf(buf, "      <protocol type='%s'/>\n",
                          virDomainChrTcpProtocolTypeToString(
                              def->data.tcp.protocol));
        break;

    case VIR_DOMAIN_CHR_TYPE_UNIX:
        virBufferVSprintf(buf, "      <source mode='%s'",
                          def->data.nix.listen ? "bind" : "connect");
        virBufferEscapeString(buf, " path='%s'/>\n",
                              def->data.nix.path);
        break;
    }

    return 0;
}

static int
virDomainChrDefFormat(virBufferPtr buf,
                      virDomainChrDefPtr def,
                      int flags)
{
    const char *elementName = virDomainChrDeviceTypeToString(def->deviceType);
    const char *targetType = virDomainChrTargetTypeToString(def->deviceType,
                                                            def->targetType);
    bool tty_compat;

    int ret = 0;

    if (!elementName) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                             _("unexpected char device type %d"),
                             def->deviceType);
        return -1;
    }

    virBufferVSprintf(buf, "    <%s", elementName);
    tty_compat = (def->deviceType == VIR_DOMAIN_CHR_DEVICE_TYPE_CONSOLE &&
                  def->target.port == 0 &&
                  def->source.type == VIR_DOMAIN_CHR_TYPE_PTY &&
                  !(flags & VIR_DOMAIN_XML_INACTIVE) &&
                  def->source.data.file.path);
    if (virDomainChrSourceDefFormat(buf, &def->source, tty_compat, flags) < 0)
        return -1;

    /* Format <target> block */
    switch (def->deviceType) {
    case VIR_DOMAIN_CHR_DEVICE_TYPE_CHANNEL: {
        if (!targetType) {
            virDomainReportError(VIR_ERR_INTERNAL_ERROR, "%s",
                                 _("Could not format channel target type"));
            return -1;
        }
        virBufferVSprintf(buf, "      <target type='%s'", targetType);

        switch (def->targetType) {
        case VIR_DOMAIN_CHR_CHANNEL_TARGET_TYPE_GUESTFWD: {
            int port = virSocketGetPort(def->target.addr);
            if (port < 0) {
                virDomainReportError(VIR_ERR_INTERNAL_ERROR, "%s",
                                     _("Unable to format guestfwd port"));
                return -1;
            }

            const char *addr = virSocketFormatAddr(def->target.addr);
            if (addr == NULL)
                return -1;

            virBufferVSprintf(buf, " address='%s' port='%d'",
                              addr, port);
            VIR_FREE(addr);
            break;
        }

        case VIR_DOMAIN_CHR_CHANNEL_TARGET_TYPE_VIRTIO: {
            if (def->target.name) {
                virBufferEscapeString(buf, " name='%s'", def->target.name);
            }
            break;
        }

        }
        virBufferAddLit(buf, "/>\n");
        break;
    }

    case VIR_DOMAIN_CHR_DEVICE_TYPE_CONSOLE:
        virBufferVSprintf(buf,
                          "      <target type='%s' port='%d'/>\n",
                          virDomainChrTargetTypeToString(def->deviceType,
                                                         def->targetType),
                          def->target.port);
        break;

    default:
        virBufferVSprintf(buf, "      <target port='%d'/>\n",
                          def->target.port);
        break;
    }

    if (virDomainDeviceInfoIsSet(&def->info)) {
        if (virDomainDeviceInfoFormat(buf, &def->info, flags) < 0)
            return -1;
    }

    virBufferVSprintf(buf, "    </%s>\n",
                      elementName);

    return ret;
}

static int
virDomainSmartcardDefFormat(virBufferPtr buf,
                            virDomainSmartcardDefPtr def,
                            int flags)
{
    const char *mode = virDomainSmartcardTypeToString(def->type);
    size_t i;

    if (!mode) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                             _("unexpected smartcard type %d"), def->type);
        return -1;
    }

    virBufferVSprintf(buf, "    <smartcard mode='%s'", mode);
    switch (def->type) {
    case VIR_DOMAIN_SMARTCARD_TYPE_HOST:
        if (!virDomainDeviceInfoIsSet(&def->info)) {
            virBufferAddLit(buf, "/>\n");
            return 0;
        }
        virBufferAddLit(buf, ">\n");
        break;

    case VIR_DOMAIN_SMARTCARD_TYPE_HOST_CERTIFICATES:
        virBufferAddLit(buf, ">\n");
        for (i = 0; i < VIR_DOMAIN_SMARTCARD_NUM_CERTIFICATES; i++)
            virBufferEscapeString(buf, "      <certificate>%s</certificate>\n",
                                  def->data.cert.file[i]);
        if (def->data.cert.database)
            virBufferEscapeString(buf, "      <database>%s</database>\n",
                                  def->data.cert.database);
        break;

    case VIR_DOMAIN_SMARTCARD_TYPE_PASSTHROUGH:
        if (virDomainChrSourceDefFormat(buf, &def->data.passthru, false,
                                        flags) < 0)
            return -1;
        break;

    default:
        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                             _("unexpected smartcard type %d"), def->type);
        return -1;
    }
    if (virDomainDeviceInfoFormat(buf, &def->info, flags) < 0)
        return -1;
    virBufferAddLit(buf, "    </smartcard>\n");
    return 0;
}

static int
virDomainSoundDefFormat(virBufferPtr buf,
                        virDomainSoundDefPtr def,
                        int flags)
{
    const char *model = virDomainSoundModelTypeToString(def->model);

    if (!model) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                             _("unexpected sound model %d"), def->model);
        return -1;
    }

    virBufferVSprintf(buf, "    <sound model='%s'",
                      model);

    if (virDomainDeviceInfoIsSet(&def->info)) {
        virBufferAddLit(buf, ">\n");
        if (virDomainDeviceInfoFormat(buf, &def->info, flags) < 0)
            return -1;
        virBufferAddLit(buf, "    </sound>\n");
    } else {
        virBufferAddLit(buf, "/>\n");
    }

    return 0;
}


static int
virDomainMemballoonDefFormat(virBufferPtr buf,
                             virDomainMemballoonDefPtr def,
                             int flags)
{
    const char *model = virDomainMemballoonModelTypeToString(def->model);

    if (!model) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                             _("unexpected memballoon model %d"), def->model);
        return -1;
    }

    virBufferVSprintf(buf, "    <memballoon model='%s'",
                      model);

    if (virDomainDeviceInfoIsSet(&def->info)) {
        virBufferAddLit(buf, ">\n");
        if (virDomainDeviceInfoFormat(buf, &def->info, flags) < 0)
            return -1;
        virBufferAddLit(buf, "    </memballoon>\n");
    } else {
        virBufferAddLit(buf, "/>\n");
    }

    return 0;
}

static int
virDomainSysinfoDefFormat(virBufferPtr buf,
                          virSysinfoDefPtr def)
{
    char *format = virSysinfoFormat(def, "  ");

    if (!format)
        return -1;
    virBufferAdd(buf, format, strlen(format));
    VIR_FREE(format);
    return 0;
}


static int
virDomainWatchdogDefFormat(virBufferPtr buf,
                           virDomainWatchdogDefPtr def,
                           int flags)
{
    const char *model = virDomainWatchdogModelTypeToString (def->model);
    const char *action = virDomainWatchdogActionTypeToString (def->action);

    if (!model) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                             _("unexpected watchdog model %d"), def->model);
        return -1;
    }

    if (!action) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                             _("unexpected watchdog action %d"), def->action);
        return -1;
    }

    virBufferVSprintf(buf, "    <watchdog model='%s' action='%s'",
                      model, action);

    if (virDomainDeviceInfoIsSet(&def->info)) {
        virBufferAddLit(buf, ">\n");
        if (virDomainDeviceInfoFormat(buf, &def->info, flags) < 0)
            return -1;
        virBufferAddLit(buf, "    </watchdog>\n");
    } else {
        virBufferAddLit(buf, "/>\n");
    }

    return 0;
}


static void
virDomainVideoAccelDefFormat(virBufferPtr buf,
                             virDomainVideoAccelDefPtr def)
{
    virBufferVSprintf(buf, "        <acceleration accel3d='%s'",
                      def->support3d ? "yes" : "no");
    virBufferVSprintf(buf, " accel2d='%s'",
                      def->support2d ? "yes" : "no");
    virBufferAddLit(buf, "/>\n");
}


static int
virDomainVideoDefFormat(virBufferPtr buf,
                        virDomainVideoDefPtr def,
                        int flags)
{
    const char *model = virDomainVideoTypeToString(def->type);

    if (!model) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                             _("unexpected video model %d"), def->type);
        return -1;
    }

    virBufferAddLit(buf, "    <video>\n");
    virBufferVSprintf(buf, "      <model type='%s'",
                      model);
    if (def->vram)
        virBufferVSprintf(buf, " vram='%u'", def->vram);
    if (def->heads)
        virBufferVSprintf(buf, " heads='%u'", def->heads);
    if (def->accel) {
        virBufferAddLit(buf, ">\n");
        virDomainVideoAccelDefFormat(buf, def->accel);
        virBufferAddLit(buf, "      </model>\n");
    } else {
        virBufferAddLit(buf, "/>\n");
    }

    if (virDomainDeviceInfoFormat(buf, &def->info, flags) < 0)
        return -1;

    virBufferAddLit(buf, "    </video>\n");

    return 0;
}

static int
virDomainInputDefFormat(virBufferPtr buf,
                        virDomainInputDefPtr def,
                        int flags)
{
    const char *type = virDomainInputTypeToString(def->type);
    const char *bus = virDomainInputBusTypeToString(def->bus);

    if (!type) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                             _("unexpected input type %d"), def->type);
        return -1;
    }
    if (!bus) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                             _("unexpected input bus type %d"), def->bus);
        return -1;
    }

    virBufferVSprintf(buf, "    <input type='%s' bus='%s'",
                      type, bus);

    if (virDomainDeviceInfoIsSet(&def->info)) {
        virBufferAddLit(buf, ">\n");
        if (virDomainDeviceInfoFormat(buf, &def->info, flags) < 0)
            return -1;
        virBufferAddLit(buf, "    </input>\n");
    } else {
        virBufferAddLit(buf, "/>\n");
    }

    return 0;
}


static int
virDomainTimerDefFormat(virBufferPtr buf,
                        virDomainTimerDefPtr def)
{
    const char *name = virDomainTimerNameTypeToString(def->name);

    if (!name) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                             _("unexpected timer name %d"), def->name);
        return -1;
    }
    virBufferVSprintf(buf, "    <timer name='%s'", name);

    if (def->present == 0) {
        virBufferAddLit(buf, " present='no'");
    } else if (def->present == 1) {
        virBufferAddLit(buf, " present='yes'");
    }

    if (def->tickpolicy != -1) {
        const char *tickpolicy
            = virDomainTimerTickpolicyTypeToString(def->tickpolicy);
        if (!tickpolicy) {
            virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                                 _("unexpected timer tickpolicy %d"),
                                 def->tickpolicy);
            return -1;
        }
        virBufferVSprintf(buf, " tickpolicy='%s'", tickpolicy);
    }

    if ((def->name == VIR_DOMAIN_TIMER_NAME_PLATFORM)
        || (def->name == VIR_DOMAIN_TIMER_NAME_RTC)) {
        if (def->track != -1) {
            const char *track
                = virDomainTimerTrackTypeToString(def->track);
            if (!track) {
                virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                                     _("unexpected timer track %d"),
                                     def->track);
                return -1;
            }
            virBufferVSprintf(buf, " track='%s'", track);
        }
    }

    if (def->name == VIR_DOMAIN_TIMER_NAME_TSC) {
        if (def->frequency > 0) {
            virBufferVSprintf(buf, " frequency='%lu'", def->frequency);
        }

        if (def->mode != -1) {
            const char *mode
                = virDomainTimerModeTypeToString(def->mode);
            if (!mode) {
                virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                                     _("unexpected timer mode %d"),
                                     def->mode);
                return -1;
            }
            virBufferVSprintf(buf, " mode='%s'", mode);
        }
    }

    if ((def->catchup.threshold == 0)
        && (def->catchup.slew == 0)
        && (def->catchup.limit == 0)) {
        virBufferAddLit(buf, "/>\n");
    } else {
        virBufferAddLit(buf, ">\n      <catchup ");
        if (def->catchup.threshold > 0) {
            virBufferVSprintf(buf, " threshold='%lu'", def->catchup.threshold);
        }
        if (def->catchup.slew > 0) {
            virBufferVSprintf(buf, " slew='%lu'", def->catchup.slew);
        }
        if (def->catchup.limit > 0) {
            virBufferVSprintf(buf, " limit='%lu'", def->catchup.limit);
        }
        virBufferAddLit(buf, "/>\n    </timer>\n");
    }

    return 0;
}

static void
virDomainGraphicsAuthDefFormatAttr(virBufferPtr buf,
                                   virDomainGraphicsAuthDefPtr def,
                                   unsigned int flags)
{
    if (!def->passwd)
        return;

    if (flags & VIR_DOMAIN_XML_SECURE)
        virBufferEscapeString(buf, " passwd='%s'",
                              def->passwd);

    if (def->expires) {
        char strbuf[100];
        struct tm tmbuf, *tm;
        tm = gmtime_r(&def->validTo, &tmbuf);
        strftime(strbuf, sizeof(strbuf), "%Y-%m-%dT%H:%M:%S", tm);
        virBufferVSprintf(buf, " passwdValidTo='%s'", strbuf);
    }
}

static int
virDomainGraphicsDefFormat(virBufferPtr buf,
                           virDomainGraphicsDefPtr def,
                           int flags)
{
    const char *type = virDomainGraphicsTypeToString(def->type);
    int children = 0;
    int i;

    if (!type) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                             _("unexpected net type %d"), def->type);
        return -1;
    }

    virBufferVSprintf(buf, "    <graphics type='%s'", type);

    switch (def->type) {
    case VIR_DOMAIN_GRAPHICS_TYPE_VNC:
        if (def->data.vnc.socket) {
            if (def->data.vnc.socket)
                virBufferVSprintf(buf, " socket='%s'",
                                  def->data.vnc.socket);
        } else {
            if (def->data.vnc.port &&
                (!def->data.vnc.autoport || !(flags & VIR_DOMAIN_XML_INACTIVE)))
                virBufferVSprintf(buf, " port='%d'",
                                  def->data.vnc.port);
            else if (def->data.vnc.autoport)
                virBufferAddLit(buf, " port='-1'");

            virBufferVSprintf(buf, " autoport='%s'",
                              def->data.vnc.autoport ? "yes" : "no");

            if (def->data.vnc.listenAddr)
                virBufferVSprintf(buf, " listen='%s'",
                                  def->data.vnc.listenAddr);
        }

        if (def->data.vnc.keymap)
            virBufferEscapeString(buf, " keymap='%s'",
                                  def->data.vnc.keymap);

        virDomainGraphicsAuthDefFormatAttr(buf, &def->data.vnc.auth, flags);
        break;

    case VIR_DOMAIN_GRAPHICS_TYPE_SDL:
        if (def->data.sdl.display)
            virBufferEscapeString(buf, " display='%s'",
                                  def->data.sdl.display);

        if (def->data.sdl.xauth)
            virBufferEscapeString(buf, " xauth='%s'",
                                  def->data.sdl.xauth);
        if (def->data.sdl.fullscreen)
            virBufferAddLit(buf, " fullscreen='yes'");

        break;

    case VIR_DOMAIN_GRAPHICS_TYPE_RDP:
        if (def->data.rdp.port)
            virBufferVSprintf(buf, " port='%d'",
                              def->data.rdp.port);
        else if (def->data.rdp.autoport)
            virBufferAddLit(buf, " port='0'");

        if (def->data.rdp.autoport)
            virBufferVSprintf(buf, " autoport='yes'");

        if (def->data.rdp.replaceUser)
            virBufferVSprintf(buf, " replaceUser='yes'");

        if (def->data.rdp.multiUser)
            virBufferVSprintf(buf, " multiUser='yes'");

        if (def->data.rdp.listenAddr)
            virBufferVSprintf(buf, " listen='%s'", def->data.rdp.listenAddr);

        break;

    case VIR_DOMAIN_GRAPHICS_TYPE_DESKTOP:
        if (def->data.desktop.display)
            virBufferEscapeString(buf, " display='%s'",
                                  def->data.desktop.display);

        if (def->data.desktop.fullscreen)
            virBufferAddLit(buf, " fullscreen='yes'");

        break;

    case VIR_DOMAIN_GRAPHICS_TYPE_SPICE:
        if (def->data.spice.port)
            virBufferVSprintf(buf, " port='%d'",
                              def->data.spice.port);

        if (def->data.spice.tlsPort)
            virBufferVSprintf(buf, " tlsPort='%d'",
                              def->data.spice.tlsPort);

        virBufferVSprintf(buf, " autoport='%s'",
                          def->data.spice.autoport ? "yes" : "no");

        if (def->data.spice.listenAddr)
            virBufferVSprintf(buf, " listen='%s'",
                              def->data.spice.listenAddr);

        if (def->data.spice.keymap)
            virBufferEscapeString(buf, " keymap='%s'",
                                  def->data.spice.keymap);

        virDomainGraphicsAuthDefFormatAttr(buf, &def->data.spice.auth, flags);
        break;

    }

    if (def->type == VIR_DOMAIN_GRAPHICS_TYPE_SPICE) {
        for (i = 0 ; i < VIR_DOMAIN_GRAPHICS_SPICE_CHANNEL_LAST ; i++) {
            int mode = def->data.spice.channels[i];
            if (mode == VIR_DOMAIN_GRAPHICS_SPICE_CHANNEL_MODE_ANY)
                continue;

            if (!children) {
                virBufferAddLit(buf, ">\n");
                children = 1;
            }

            virBufferVSprintf(buf, "      <channel name='%s' mode='%s'/>\n",
                              virDomainGraphicsSpiceChannelNameTypeToString(i),
                              virDomainGraphicsSpiceChannelModeTypeToString(mode));
        }
    }

    if (children) {
        virBufferAddLit(buf, "    </graphics>\n");
    } else {
        virBufferAddLit(buf, "/>\n");
    }

    return 0;
}


static int
virDomainHostdevDefFormat(virBufferPtr buf,
                          virDomainHostdevDefPtr def,
                          int flags)
{
    const char *mode = virDomainHostdevModeTypeToString(def->mode);
    const char *type;

    if (!mode || def->mode != VIR_DOMAIN_HOSTDEV_MODE_SUBSYS) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                             _("unexpected hostdev mode %d"), def->mode);
        return -1;
    }

    type = virDomainHostdevSubsysTypeToString(def->source.subsys.type);
    if (!type || (def->source.subsys.type != VIR_DOMAIN_HOSTDEV_SUBSYS_TYPE_USB && def->source.subsys.type != VIR_DOMAIN_HOSTDEV_SUBSYS_TYPE_PCI) ) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                             _("unexpected hostdev type %d"),
                             def->source.subsys.type);
        return -1;
    }

    virBufferVSprintf(buf, "    <hostdev mode='%s' type='%s' managed='%s'>\n",
                      mode, type, def->managed ? "yes" : "no");
    virBufferAddLit(buf, "      <source>\n");

    if (def->source.subsys.type == VIR_DOMAIN_HOSTDEV_SUBSYS_TYPE_USB) {
        if (def->source.subsys.u.usb.vendor) {
            virBufferVSprintf(buf, "        <vendor id='0x%.4x'/>\n",
                              def->source.subsys.u.usb.vendor);
            virBufferVSprintf(buf, "        <product id='0x%.4x'/>\n",
                              def->source.subsys.u.usb.product);
        }
        if (def->source.subsys.u.usb.bus ||
            def->source.subsys.u.usb.device)
            virBufferVSprintf(buf, "        <address bus='%d' device='%d'/>\n",
                              def->source.subsys.u.usb.bus,
                              def->source.subsys.u.usb.device);
    } else if (def->source.subsys.type == VIR_DOMAIN_HOSTDEV_SUBSYS_TYPE_PCI) {
        virBufferVSprintf(buf, "        <address domain='0x%.4x' bus='0x%.2x' slot='0x%.2x' function='0x%.1x'/>\n",
                          def->source.subsys.u.pci.domain,
                          def->source.subsys.u.pci.bus,
                          def->source.subsys.u.pci.slot,
                          def->source.subsys.u.pci.function);
    }

    virBufferAddLit(buf, "      </source>\n");

    if (def->bootIndex)
        virBufferVSprintf(buf, "      <boot order='%d'/>\n", def->bootIndex);

    if (virDomainDeviceInfoFormat(buf, &def->info, flags) < 0)
        return -1;

    virBufferAddLit(buf, "    </hostdev>\n");

    return 0;
}


char *virDomainDefFormat(virDomainDefPtr def,
                         int flags)
{
    virBuffer buf = VIR_BUFFER_INITIALIZER;
    unsigned char *uuid;
    char uuidstr[VIR_UUID_STRING_BUFLEN];
    const char *type = NULL;
    int n, allones = 1;

    if (!(type = virDomainVirtTypeToString(def->virtType))) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                         _("unexpected domain type %d"), def->virtType);
        goto cleanup;
    }

    if (def->id == -1)
        flags |= VIR_DOMAIN_XML_INACTIVE;

    virBufferVSprintf(&buf, "<domain type='%s'", type);
    if (!(flags & VIR_DOMAIN_XML_INACTIVE))
        virBufferVSprintf(&buf, " id='%d'", def->id);
    if (def->namespaceData && def->ns.href)
        virBufferVSprintf(&buf, " %s", (def->ns.href)());
    virBufferAddLit(&buf, ">\n");

    virBufferEscapeString(&buf, "  <name>%s</name>\n", def->name);

    uuid = def->uuid;
    virUUIDFormat(uuid, uuidstr);
    virBufferVSprintf(&buf, "  <uuid>%s</uuid>\n", uuidstr);

    if (def->description)
        virBufferEscapeString(&buf, "  <description>%s</description>\n",
                              def->description);

    virBufferVSprintf(&buf, "  <memory>%lu</memory>\n", def->mem.max_balloon);
    virBufferVSprintf(&buf, "  <currentMemory>%lu</currentMemory>\n",
                      def->mem.cur_balloon);

    /* add blkiotune only if there are any */
    if (def->blkio.weight) {
        virBufferVSprintf(&buf, "  <blkiotune>\n");
        virBufferVSprintf(&buf, "    <weight>%u</weight>\n",
                          def->blkio.weight);
        virBufferVSprintf(&buf, "  </blkiotune>\n");
    }

    /* add memtune only if there are any */
    if (def->mem.hard_limit || def->mem.soft_limit || def->mem.min_guarantee ||
        def->mem.swap_hard_limit)
        virBufferVSprintf(&buf, "  <memtune>\n");
    if (def->mem.hard_limit) {
        virBufferVSprintf(&buf, "    <hard_limit>%lu</hard_limit>\n",
                          def->mem.hard_limit);
    }
    if (def->mem.soft_limit) {
        virBufferVSprintf(&buf, "    <soft_limit>%lu</soft_limit>\n",
                          def->mem.soft_limit);
    }
    if (def->mem.min_guarantee) {
        virBufferVSprintf(&buf, "    <min_guarantee>%lu</min_guarantee>\n",
                          def->mem.min_guarantee);
    }
    if (def->mem.swap_hard_limit) {
        virBufferVSprintf(&buf, "    <swap_hard_limit>%lu</swap_hard_limit>\n",
                          def->mem.swap_hard_limit);
    }
    if (def->mem.hard_limit || def->mem.soft_limit || def->mem.min_guarantee ||
        def->mem.swap_hard_limit)
        virBufferVSprintf(&buf, "  </memtune>\n");

    if (def->mem.hugepage_backed) {
        virBufferAddLit(&buf, "  <memoryBacking>\n");
        virBufferAddLit(&buf, "    <hugepages/>\n");
        virBufferAddLit(&buf, "  </memoryBacking>\n");
    }

    for (n = 0 ; n < def->cpumasklen ; n++)
        if (def->cpumask[n] != 1)
            allones = 0;

    virBufferAddLit(&buf, "  <vcpu");
    if (!allones) {
        char *cpumask = NULL;
        if ((cpumask =
             virDomainCpuSetFormat(def->cpumask, def->cpumasklen)) == NULL)
            goto cleanup;
        virBufferVSprintf(&buf, " cpuset='%s'", cpumask);
        VIR_FREE(cpumask);
    }
    if (def->vcpus != def->maxvcpus)
        virBufferVSprintf(&buf, " current='%u'", def->vcpus);
    virBufferVSprintf(&buf, ">%u</vcpu>\n", def->maxvcpus);

    if (def->sysinfo)
        virDomainSysinfoDefFormat(&buf, def->sysinfo);

    if (def->os.bootloader) {
        virBufferEscapeString(&buf, "  <bootloader>%s</bootloader>\n",
                              def->os.bootloader);
        if (def->os.bootloaderArgs)
            virBufferEscapeString(&buf, "  <bootloader_args>%s</bootloader_args>\n",
                                  def->os.bootloaderArgs);
    }
    virBufferAddLit(&buf, "  <os>\n");

    virBufferAddLit(&buf, "    <type");
    if (def->os.arch)
        virBufferVSprintf(&buf, " arch='%s'", def->os.arch);
    if (def->os.machine)
        virBufferVSprintf(&buf, " machine='%s'", def->os.machine);
    /*
     * HACK: For xen driver we previously used bogus 'linux' as the
     * os type for paravirt, whereas capabilities declare it to
     * be 'xen'. So we convert to the former for backcompat
     */
    if (def->virtType == VIR_DOMAIN_VIRT_XEN &&
        STREQ(def->os.type, "xen"))
        virBufferVSprintf(&buf, ">%s</type>\n", "linux");
    else
        virBufferVSprintf(&buf, ">%s</type>\n", def->os.type);

    if (def->os.init)
        virBufferEscapeString(&buf, "    <init>%s</init>\n",
                              def->os.init);
    if (def->os.loader)
        virBufferEscapeString(&buf, "    <loader>%s</loader>\n",
                              def->os.loader);
    if (def->os.kernel)
        virBufferEscapeString(&buf, "    <kernel>%s</kernel>\n",
                              def->os.kernel);
    if (def->os.initrd)
        virBufferEscapeString(&buf, "    <initrd>%s</initrd>\n",
                              def->os.initrd);
    if (def->os.cmdline)
        virBufferEscapeString(&buf, "    <cmdline>%s</cmdline>\n",
                              def->os.cmdline);
    if (def->os.root)
        virBufferEscapeString(&buf, "    <root>%s</root>\n",
                              def->os.root);

    if (!def->os.bootloader) {
        for (n = 0 ; n < def->os.nBootDevs ; n++) {
            const char *boottype =
                virDomainBootTypeToString(def->os.bootDevs[n]);
            if (!boottype) {
                virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                                     _("unexpected boot device type %d"),
                                     def->os.bootDevs[n]);
                goto cleanup;
            }
            virBufferVSprintf(&buf, "    <boot dev='%s'/>\n", boottype);
        }

        if (def->os.bootmenu != VIR_DOMAIN_BOOT_MENU_DEFAULT) {
            const char *enabled = (def->os.bootmenu ==
                                   VIR_DOMAIN_BOOT_MENU_ENABLED ? "yes"
                                                                : "no");
            virBufferVSprintf(&buf, "    <bootmenu enable='%s'/>\n", enabled);
        }
    }

    if (def->os.smbios_mode) {
        const char *mode;

        mode = virDomainSmbiosModeTypeToString(def->os.smbios_mode);
        if (mode == NULL) {
            virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                         _("unexpected smbios mode %d"), def->os.smbios_mode);
            goto cleanup;
        }
        virBufferVSprintf(&buf, "    <smbios mode='%s'/>\n", mode);
    }

    virBufferAddLit(&buf, "  </os>\n");

    if (def->features) {
        int i;
        virBufferAddLit(&buf, "  <features>\n");
        for (i = 0 ; i < VIR_DOMAIN_FEATURE_LAST ; i++) {
            if (def->features & (1 << i)) {
                const char *name = virDomainFeatureTypeToString(i);
                if (!name) {
                    virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                                         _("unexpected feature %d"), i);
                    goto cleanup;
                }
                virBufferVSprintf(&buf, "    <%s/>\n", name);
            }
        }
        virBufferAddLit(&buf, "  </features>\n");
    }

    if (virCPUDefFormatBuf(&buf, def->cpu, "  ", 0) < 0)
        goto cleanup;

    virBufferVSprintf(&buf, "  <clock offset='%s'",
                      virDomainClockOffsetTypeToString(def->clock.offset));
    switch (def->clock.offset) {
    case VIR_DOMAIN_CLOCK_OFFSET_VARIABLE:
        virBufferVSprintf(&buf, " adjustment='%lld'", def->clock.data.adjustment);
        break;
    case VIR_DOMAIN_CLOCK_OFFSET_TIMEZONE:
        virBufferEscapeString(&buf, " timezone='%s'", def->clock.data.timezone);
        break;
    }
    if (def->clock.ntimers == 0) {
        virBufferAddLit(&buf, "/>\n");
    } else {
        virBufferAddLit(&buf, ">\n");
        for (n = 0; n < def->clock.ntimers; n++) {
            if (virDomainTimerDefFormat(&buf, def->clock.timers[n]) < 0)
                goto cleanup;
        }
        virBufferAddLit(&buf, "  </clock>\n");
    }

    if (virDomainLifecycleDefFormat(&buf, def->onPoweroff,
                                    "on_poweroff",
                                    virDomainLifecycleTypeToString) < 0)
        goto cleanup;
    if (virDomainLifecycleDefFormat(&buf, def->onReboot,
                                    "on_reboot",
                                    virDomainLifecycleTypeToString) < 0)
        goto cleanup;
    if (virDomainLifecycleDefFormat(&buf, def->onCrash,
                                    "on_crash",
                                    virDomainLifecycleCrashTypeToString) < 0)
        goto cleanup;

    virBufferAddLit(&buf, "  <devices>\n");

    if (def->emulator)
        virBufferEscapeString(&buf, "    <emulator>%s</emulator>\n",
                              def->emulator);

    for (n = 0 ; n < def->ndisks ; n++)
        if (virDomainDiskDefFormat(&buf, def->disks[n], flags) < 0)
            goto cleanup;

    for (n = 0 ; n < def->ncontrollers ; n++)
        if (virDomainControllerDefFormat(&buf, def->controllers[n], flags) < 0)
            goto cleanup;

    for (n = 0 ; n < def->nfss ; n++)
        if (virDomainFSDefFormat(&buf, def->fss[n], flags) < 0)
            goto cleanup;


    for (n = 0 ; n < def->nnets ; n++)
        if (virDomainNetDefFormat(&buf, def->nets[n], flags) < 0)
            goto cleanup;

    for (n = 0 ; n < def->nsmartcards ; n++)
        if (virDomainSmartcardDefFormat(&buf, def->smartcards[n], flags) < 0)
            goto cleanup;

    for (n = 0 ; n < def->nserials ; n++)
        if (virDomainChrDefFormat(&buf, def->serials[n], flags) < 0)
            goto cleanup;

    for (n = 0 ; n < def->nparallels ; n++)
        if (virDomainChrDefFormat(&buf, def->parallels[n], flags) < 0)
            goto cleanup;

    /* If there's a PV console that's preferred.. */
    if (def->console) {
        if (virDomainChrDefFormat(&buf, def->console, flags) < 0)
            goto cleanup;
    } else if (def->nserials != 0) {
        /* ..else for legacy compat duplicate the first serial device as a
         * console */
        virDomainChrDef console;
        memcpy(&console, def->serials[0], sizeof(console));
        console.deviceType = VIR_DOMAIN_CHR_DEVICE_TYPE_CONSOLE;
        if (virDomainChrDefFormat(&buf, &console, flags) < 0)
            goto cleanup;
    }

    for (n = 0 ; n < def->nchannels ; n++)
        if (virDomainChrDefFormat(&buf, def->channels[n], flags) < 0)
            goto cleanup;

    for (n = 0 ; n < def->ninputs ; n++)
        if (def->inputs[n]->bus == VIR_DOMAIN_INPUT_BUS_USB &&
            virDomainInputDefFormat(&buf, def->inputs[n], flags) < 0)
            goto cleanup;

    if (def->ngraphics > 0) {
        /* If graphics is enabled, add the implicit mouse */
        virDomainInputDef autoInput = {
            VIR_DOMAIN_INPUT_TYPE_MOUSE,
            STREQ(def->os.type, "hvm") ?
            VIR_DOMAIN_INPUT_BUS_PS2 : VIR_DOMAIN_INPUT_BUS_XEN,
            { .alias = NULL },
        };

        if (virDomainInputDefFormat(&buf, &autoInput, flags) < 0)
            goto cleanup;

        for (n = 0 ; n < def->ngraphics ; n++)
            if (virDomainGraphicsDefFormat(&buf, def->graphics[n], flags) < 0)
                goto cleanup;
    }

    for (n = 0 ; n < def->nsounds ; n++)
        if (virDomainSoundDefFormat(&buf, def->sounds[n], flags) < 0)
            goto cleanup;

    for (n = 0 ; n < def->nvideos ; n++)
        if (virDomainVideoDefFormat(&buf, def->videos[n], flags) < 0)
            goto cleanup;

    for (n = 0 ; n < def->nhostdevs ; n++)
        if (virDomainHostdevDefFormat(&buf, def->hostdevs[n], flags) < 0)
            goto cleanup;

    if (def->watchdog)
        virDomainWatchdogDefFormat (&buf, def->watchdog, flags);

    if (def->memballoon)
        virDomainMemballoonDefFormat (&buf, def->memballoon, flags);

    virBufferAddLit(&buf, "  </devices>\n");

    if (def->seclabel.model) {
        const char *sectype = virDomainSeclabelTypeToString(def->seclabel.type);
        if (!sectype)
            goto cleanup;
        if (!def->seclabel.label ||
            (def->seclabel.type == VIR_DOMAIN_SECLABEL_DYNAMIC &&
             (flags & VIR_DOMAIN_XML_INACTIVE))) {
            virBufferVSprintf(&buf, "  <seclabel type='%s' model='%s'/>\n",
                              sectype, def->seclabel.model);
        } else {
            virBufferVSprintf(&buf, "  <seclabel type='%s' model='%s'>\n",
                                  sectype, def->seclabel.model);
            virBufferEscapeString(&buf, "    <label>%s</label>\n",
                                  def->seclabel.label);
            if (def->seclabel.imagelabel &&
                def->seclabel.type == VIR_DOMAIN_SECLABEL_DYNAMIC)
                virBufferEscapeString(&buf, "    <imagelabel>%s</imagelabel>\n",
                                      def->seclabel.imagelabel);
            virBufferAddLit(&buf, "  </seclabel>\n");
        }
    }

    if (def->namespaceData && def->ns.format) {
        if ((def->ns.format)(&buf, def->namespaceData) < 0)
            goto cleanup;
    }

    virBufferAddLit(&buf, "</domain>\n");

    if (virBufferError(&buf))
        goto no_memory;

    return virBufferContentAndReset(&buf);

 no_memory:
    virReportOOMError();
 cleanup:
    virBufferFreeAndReset(&buf);
    return NULL;
}


static char *virDomainObjFormat(virCapsPtr caps,
                                virDomainObjPtr obj,
                                int flags)
{
    char *config_xml = NULL;
    virBuffer buf = VIR_BUFFER_INITIALIZER;

    virBufferVSprintf(&buf, "<domstatus state='%s' pid='%d'>\n",
                      virDomainStateTypeToString(obj->state),
                      obj->pid);

    if (caps->privateDataXMLFormat &&
        ((caps->privateDataXMLFormat)(&buf, obj->privateData)) < 0)
        goto error;

    if (!(config_xml = virDomainDefFormat(obj->def,
                                          flags)))
        goto error;

    virBufferAdd(&buf, config_xml, strlen(config_xml));
    VIR_FREE(config_xml);
    virBufferAddLit(&buf, "</domstatus>\n");

    if (virBufferError(&buf))
        goto no_memory;

    return virBufferContentAndReset(&buf);

no_memory:
    virReportOOMError();
error:
    virBufferFreeAndReset(&buf);
    return NULL;
}

int virDomainSaveXML(const char *configDir,
                     virDomainDefPtr def,
                     const char *xml)
{
    char *configFile = NULL;
    int fd = -1, ret = -1;
    size_t towrite;

    if ((configFile = virDomainConfigFile(configDir, def->name)) == NULL)
        goto cleanup;

    if (virFileMakePath(configDir)) {
        virReportSystemError(errno,
                             _("cannot create config directory '%s'"),
                             configDir);
        goto cleanup;
    }

    if ((fd = open(configFile,
                   O_WRONLY | O_CREAT | O_TRUNC,
                   S_IRUSR | S_IWUSR )) < 0) {
        virReportSystemError(errno,
                             _("cannot create config file '%s'"),
                             configFile);
        goto cleanup;
    }

    towrite = strlen(xml);
    if (safewrite(fd, xml, towrite) < 0) {
        virReportSystemError(errno,
                             _("cannot write config file '%s'"),
                             configFile);
        goto cleanup;
    }

    if (VIR_CLOSE(fd) < 0) {
        virReportSystemError(errno,
                             _("cannot save config file '%s'"),
                             configFile);
        goto cleanup;
    }

    ret = 0;
 cleanup:
    VIR_FORCE_CLOSE(fd);

    VIR_FREE(configFile);
    return ret;
}

int virDomainSaveConfig(const char *configDir,
                        virDomainDefPtr def)
{
    int ret = -1;
    char *xml;

    if (!(xml = virDomainDefFormat(def,
                                   VIR_DOMAIN_XML_WRITE_FLAGS)))
        goto cleanup;

    if (virDomainSaveXML(configDir, def, xml))
        goto cleanup;

    ret = 0;
cleanup:
    VIR_FREE(xml);
    return ret;
}

int virDomainSaveStatus(virCapsPtr caps,
                        const char *statusDir,
                        virDomainObjPtr obj)
{
    int flags = VIR_DOMAIN_XML_SECURE|VIR_DOMAIN_XML_INTERNAL_STATUS;
    int ret = -1;
    char *xml;

    if (!(xml = virDomainObjFormat(caps, obj, flags)))
        goto cleanup;

    if (virDomainSaveXML(statusDir, obj->def, xml))
        goto cleanup;

    ret = 0;
cleanup:
    VIR_FREE(xml);
    return ret;
}


static virDomainObjPtr virDomainLoadConfig(virCapsPtr caps,
                                           virDomainObjListPtr doms,
                                           const char *configDir,
                                           const char *autostartDir,
                                           const char *name,
                                           virDomainLoadConfigNotify notify,
                                           void *opaque)
{
    char *configFile = NULL, *autostartLink = NULL;
    virDomainDefPtr def = NULL;
    virDomainObjPtr dom;
    int autostart;
    int newVM = 1;

    if ((configFile = virDomainConfigFile(configDir, name)) == NULL)
        goto error;
    if (!(def = virDomainDefParseFile(caps, configFile,
                                      VIR_DOMAIN_XML_INACTIVE)))
        goto error;

    /* if the domain is already in our hashtable, we don't need to do
     * anything further
     */
    if ((dom = virDomainFindByUUID(doms, def->uuid))) {
        VIR_FREE(configFile);
        virDomainDefFree(def);
        return dom;
    }

    if ((autostartLink = virDomainConfigFile(autostartDir, name)) == NULL)
        goto error;

    if ((autostart = virFileLinkPointsTo(autostartLink, configFile)) < 0)
        goto error;

    if (!(dom = virDomainAssignDef(caps, doms, def, false)))
        goto error;

    dom->autostart = autostart;

    if (notify)
        (*notify)(dom, newVM, opaque);

    VIR_FREE(configFile);
    VIR_FREE(autostartLink);
    return dom;

error:
    VIR_FREE(configFile);
    VIR_FREE(autostartLink);
    virDomainDefFree(def);
    return NULL;
}

static virDomainObjPtr virDomainLoadStatus(virCapsPtr caps,
                                           virDomainObjListPtr doms,
                                           const char *statusDir,
                                           const char *name,
                                           virDomainLoadConfigNotify notify,
                                           void *opaque)
{
    char *statusFile = NULL;
    virDomainObjPtr obj = NULL;
    char uuidstr[VIR_UUID_STRING_BUFLEN];

    if ((statusFile = virDomainConfigFile(statusDir, name)) == NULL)
        goto error;

    if (!(obj = virDomainObjParseFile(caps, statusFile)))
        goto error;

    virUUIDFormat(obj->def->uuid, uuidstr);

    if (virHashLookup(doms->objs, uuidstr) != NULL) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                             _("unexpected domain %s already exists"),
                             obj->def->name);
        goto error;
    }

    if (virHashAddEntry(doms->objs, uuidstr, obj) < 0) {
        virReportOOMError();
        goto error;
    }

    if (notify)
        (*notify)(obj, 1, opaque);

    VIR_FREE(statusFile);
    return obj;

error:
    if (obj)
        virDomainObjUnref(obj);
    VIR_FREE(statusFile);
    return NULL;
}

int virDomainLoadAllConfigs(virCapsPtr caps,
                            virDomainObjListPtr doms,
                            const char *configDir,
                            const char *autostartDir,
                            int liveStatus,
                            virDomainLoadConfigNotify notify,
                            void *opaque)
{
    DIR *dir;
    struct dirent *entry;

    VIR_INFO("Scanning for configs in %s", configDir);

    if (!(dir = opendir(configDir))) {
        if (errno == ENOENT)
            return 0;
        virReportSystemError(errno,
                             _("Failed to open dir '%s'"),
                             configDir);
        return -1;
    }

    while ((entry = readdir(dir))) {
        virDomainObjPtr dom;

        if (entry->d_name[0] == '.')
            continue;

        if (!virFileStripSuffix(entry->d_name, ".xml"))
            continue;

        /* NB: ignoring errors, so one malformed config doesn't
           kill the whole process */
        VIR_INFO("Loading config file '%s.xml'", entry->d_name);
        if (liveStatus)
            dom = virDomainLoadStatus(caps,
                                      doms,
                                      configDir,
                                      entry->d_name,
                                      notify,
                                      opaque);
        else
            dom = virDomainLoadConfig(caps,
                                      doms,
                                      configDir,
                                      autostartDir,
                                      entry->d_name,
                                      notify,
                                      opaque);
        if (dom) {
            virDomainObjUnlock(dom);
            if (!liveStatus)
                dom->persistent = 1;
        }
    }

    closedir(dir);

    return 0;
}

int virDomainDeleteConfig(const char *configDir,
                          const char *autostartDir,
                          virDomainObjPtr dom)
{
    char *configFile = NULL, *autostartLink = NULL;
    int ret = -1;

    if ((configFile = virDomainConfigFile(configDir, dom->def->name)) == NULL)
        goto cleanup;
    if ((autostartLink = virDomainConfigFile(autostartDir, dom->def->name)) == NULL)
        goto cleanup;

    /* Not fatal if this doesn't work */
    unlink(autostartLink);

    if (unlink(configFile) < 0 &&
        errno != ENOENT) {
        virReportSystemError(errno,
                             _("cannot remove config %s"),
                             configFile);
        goto cleanup;
    }

    ret = 0;

cleanup:
    VIR_FREE(configFile);
    VIR_FREE(autostartLink);
    return ret;
}

char *virDomainConfigFile(const char *dir,
                          const char *name)
{
    char *ret = NULL;

    if (virAsprintf(&ret, "%s/%s.xml", dir, name) < 0) {
        virReportOOMError();
        return NULL;
    }

    return ret;
}

/* Translates a device name of the form (regex) "[fhv]d[a-z]+" into
 * the corresponding bus,index combination (e.g. sda => (0,0), sdi (1,1),
 *                                               hdd => (1,1), vdaa => (0,26))
 * @param disk The disk device
 * @param busIdx parsed bus number
 * @param devIdx parsed device number
 * @return 0 on success, -1 on failure
 */
int virDiskNameToBusDeviceIndex(const virDomainDiskDefPtr disk,
                                int *busIdx,
                                int *devIdx) {

    int idx = virDiskNameToIndex(disk->dst);
    if (idx < 0)
        return -1;

    switch (disk->bus) {
        case VIR_DOMAIN_DISK_BUS_IDE:
            *busIdx = idx / 2;
            *devIdx = idx % 2;
            break;
        case VIR_DOMAIN_DISK_BUS_SCSI:
            *busIdx = idx / 7;
            *devIdx = idx % 7;
            break;
        case VIR_DOMAIN_DISK_BUS_FDC:
        case VIR_DOMAIN_DISK_BUS_USB:
        case VIR_DOMAIN_DISK_BUS_VIRTIO:
        case VIR_DOMAIN_DISK_BUS_XEN:
        default:
            *busIdx = 0;
            *devIdx = idx;
            break;
    }

    return 0;
}

virDomainFSDefPtr virDomainGetRootFilesystem(virDomainDefPtr def)
{
    int i;

    for (i = 0 ; i < def->nfss ; i++) {
        if (def->fss[i]->type != VIR_DOMAIN_FS_TYPE_MOUNT)
            continue;

        if (STREQ(def->fss[i]->dst, "/"))
            return def->fss[i];
    }

    return NULL;
}

/*
 * virDomainObjIsDuplicate:
 * @doms : virDomainObjListPtr to search
 * @def  : virDomainDefPtr definition of domain to lookup
 * @check_active: If true, ensure that domain is not active
 *
 * Returns: -1 on error
 *          0 if domain is new
 *          1 if domain is a duplicate
 */
int
virDomainObjIsDuplicate(virDomainObjListPtr doms,
                        virDomainDefPtr def,
                        unsigned int check_active)
{
    int ret = -1;
    int dupVM = 0;
    virDomainObjPtr vm = NULL;

    /* See if a VM with matching UUID already exists */
    vm = virDomainFindByUUID(doms, def->uuid);
    if (vm) {
        /* UUID matches, but if names don't match, refuse it */
        if (STRNEQ(vm->def->name, def->name)) {
            char uuidstr[VIR_UUID_STRING_BUFLEN];
            virUUIDFormat(vm->def->uuid, uuidstr);
            virDomainReportError(VIR_ERR_OPERATION_FAILED,
                                 _("domain '%s' is already defined with uuid %s"),
                                 vm->def->name, uuidstr);
            goto cleanup;
        }

        if (check_active) {
            /* UUID & name match, but if VM is already active, refuse it */
            if (virDomainObjIsActive(vm)) {
                virDomainReportError(VIR_ERR_OPERATION_INVALID,
                                     _("domain is already active as '%s'"),
                                     vm->def->name);
                goto cleanup;
            }
        }

        dupVM = 1;
    } else {
        /* UUID does not match, but if a name matches, refuse it */
        vm = virDomainFindByName(doms, def->name);
        if (vm) {
            char uuidstr[VIR_UUID_STRING_BUFLEN];
            virUUIDFormat(vm->def->uuid, uuidstr);
            virDomainReportError(VIR_ERR_OPERATION_FAILED,
                                 _("domain '%s' already exists with uuid %s"),
                                 def->name, uuidstr);
            goto cleanup;
        }
    }

    ret = dupVM;
cleanup:
    if (vm)
        virDomainObjUnlock(vm);
    return ret;
}


void virDomainObjLock(virDomainObjPtr obj)
{
    virMutexLock(&obj->lock);
}

void virDomainObjUnlock(virDomainObjPtr obj)
{
    virMutexUnlock(&obj->lock);
}


static void virDomainObjListCountActive(void *payload, const char *name ATTRIBUTE_UNUSED, void *data)
{
    virDomainObjPtr obj = payload;
    int *count = data;
    virDomainObjLock(obj);
    if (virDomainObjIsActive(obj))
        (*count)++;
    virDomainObjUnlock(obj);
}

static void virDomainObjListCountInactive(void *payload, const char *name ATTRIBUTE_UNUSED, void *data)
{
    virDomainObjPtr obj = payload;
    int *count = data;
    virDomainObjLock(obj);
    if (!virDomainObjIsActive(obj))
        (*count)++;
    virDomainObjUnlock(obj);
}

int virDomainObjListNumOfDomains(virDomainObjListPtr doms, int active)
{
    int count = 0;
    if (active)
        virHashForEach(doms->objs, virDomainObjListCountActive, &count);
    else
        virHashForEach(doms->objs, virDomainObjListCountInactive, &count);
    return count;
}

struct virDomainIDData {
    int numids;
    int maxids;
    int *ids;
};

static void virDomainObjListCopyActiveIDs(void *payload, const char *name ATTRIBUTE_UNUSED, void *opaque)
{
    virDomainObjPtr obj = payload;
    struct virDomainIDData *data = opaque;
    virDomainObjLock(obj);
    if (virDomainObjIsActive(obj) && data->numids < data->maxids)
        data->ids[data->numids++] = obj->def->id;
    virDomainObjUnlock(obj);
}

int virDomainObjListGetActiveIDs(virDomainObjListPtr doms,
                                 int *ids,
                                 int maxids)
{
    struct virDomainIDData data = { 0, maxids, ids };
    virHashForEach(doms->objs, virDomainObjListCopyActiveIDs, &data);
    return data.numids;
}

struct virDomainNameData {
    int oom;
    int numnames;
    int maxnames;
    char **const names;
};

static void virDomainObjListCopyInactiveNames(void *payload, const char *name ATTRIBUTE_UNUSED, void *opaque)
{
    virDomainObjPtr obj = payload;
    struct virDomainNameData *data = opaque;

    if (data->oom)
        return;

    virDomainObjLock(obj);
    if (!virDomainObjIsActive(obj) && data->numnames < data->maxnames) {
        if (!(data->names[data->numnames] = strdup(obj->def->name)))
            data->oom = 1;
        else
            data->numnames++;
    }
    virDomainObjUnlock(obj);
}


int virDomainObjListGetInactiveNames(virDomainObjListPtr doms,
                                     char **const names,
                                     int maxnames)
{
    struct virDomainNameData data = { 0, 0, maxnames, names };
    int i;
    virHashForEach(doms->objs, virDomainObjListCopyInactiveNames, &data);
    if (data.oom) {
        virReportOOMError();
        goto cleanup;
    }

    return data.numnames;

cleanup:
    for (i = 0 ; i < data.numnames ; i++)
        VIR_FREE(data.names[i]);
    return -1;
}

/* Snapshot Def functions */
void virDomainSnapshotDefFree(virDomainSnapshotDefPtr def)
{
    if (!def)
        return;

    VIR_FREE(def->name);
    VIR_FREE(def->description);
    VIR_FREE(def->parent);
    VIR_FREE(def);
}

virDomainSnapshotDefPtr virDomainSnapshotDefParseString(const char *xmlStr,
                                                        int newSnapshot)
{
    xmlXPathContextPtr ctxt = NULL;
    xmlDocPtr xml = NULL;
    xmlNodePtr root;
    virDomainSnapshotDefPtr def = NULL;
    virDomainSnapshotDefPtr ret = NULL;
    char *creation = NULL, *state = NULL;
    struct timeval tv;

    xml = virXMLParse(NULL, xmlStr, "domainsnapshot.xml");
    if (!xml) {
        virDomainReportError(VIR_ERR_XML_ERROR,
                             "%s",_("failed to parse snapshot xml document"));
        return NULL;
    }

    if ((root = xmlDocGetRootElement(xml)) == NULL) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                              "%s", _("missing root element"));
        goto cleanup;
    }

    if (!xmlStrEqual(root->name, BAD_CAST "domainsnapshot")) {
        virDomainReportError(VIR_ERR_XML_ERROR, "%s", _("domainsnapshot"));
        goto cleanup;
    }

    ctxt = xmlXPathNewContext(xml);
    if (ctxt == NULL) {
        virReportOOMError();
        goto cleanup;
    }

    if (VIR_ALLOC(def) < 0) {
        virReportOOMError();
        goto cleanup;
    }

    ctxt->node = root;

    gettimeofday(&tv, NULL);

    def->name = virXPathString("string(./name)", ctxt);
    if (def->name == NULL)
        ignore_value(virAsprintf(&def->name, "%ld", tv.tv_sec));

    if (def->name == NULL) {
        virReportOOMError();
        goto cleanup;
    }

    def->description = virXPathString("string(./description)", ctxt);

    if (!newSnapshot) {
        if (virXPathLong("string(./creationTime)", ctxt,
                         &def->creationTime) < 0) {
            virDomainReportError(VIR_ERR_INTERNAL_ERROR, "%s",
                                 _("missing creationTime from existing snapshot"));
            goto cleanup;
        }

        def->parent = virXPathString("string(./parent/name)", ctxt);

        state = virXPathString("string(./state)", ctxt);
        if (state == NULL) {
            /* there was no state in an existing snapshot; this
             * should never happen
             */
            virDomainReportError(VIR_ERR_INTERNAL_ERROR, "%s",
                                 _("missing state from existing snapshot"));
            goto cleanup;
        }
        def->state = virDomainStateTypeFromString(state);
        if (def->state < 0) {
            virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                                 _("Invalid state '%s' in domain snapshot XML"),
                                 state);
            goto cleanup;
        }

        if (virXPathLong("string(./active)", ctxt, &def->active) < 0) {
            virDomainReportError(VIR_ERR_INTERNAL_ERROR, "%s",
                                 _("Could not find 'active' element"));
            goto cleanup;
        }
    }
    else
        def->creationTime = tv.tv_sec;

    ret = def;

cleanup:
    VIR_FREE(creation);
    VIR_FREE(state);
    xmlXPathFreeContext(ctxt);
    if (ret == NULL)
        virDomainSnapshotDefFree(def);
    xmlFreeDoc(xml);

    return ret;
}

char *virDomainSnapshotDefFormat(char *domain_uuid,
                                 virDomainSnapshotDefPtr def,
                                 int internal)
{
    virBuffer buf = VIR_BUFFER_INITIALIZER;

    virBufferAddLit(&buf, "<domainsnapshot>\n");
    virBufferVSprintf(&buf, "  <name>%s</name>\n", def->name);
    if (def->description)
        virBufferVSprintf(&buf, "  <description>%s</description>\n",
                          def->description);
    virBufferVSprintf(&buf, "  <state>%s</state>\n",
                      virDomainStateTypeToString(def->state));
    if (def->parent) {
        virBufferAddLit(&buf, "  <parent>\n");
        virBufferVSprintf(&buf, "    <name>%s</name>\n", def->parent);
        virBufferAddLit(&buf, "  </parent>\n");
    }
    virBufferVSprintf(&buf, "  <creationTime>%ld</creationTime>\n",
                      def->creationTime);
    virBufferAddLit(&buf, "  <domain>\n");
    virBufferVSprintf(&buf, "    <uuid>%s</uuid>\n", domain_uuid);
    virBufferAddLit(&buf, "  </domain>\n");
    if (internal)
        virBufferVSprintf(&buf, "  <active>%ld</active>\n", def->active);
    virBufferAddLit(&buf, "</domainsnapshot>\n");

    if (virBufferError(&buf)) {
        virBufferFreeAndReset(&buf);
        virReportOOMError();
        return NULL;
    }

    return virBufferContentAndReset(&buf);
}

/* Snapshot Obj functions */
static virDomainSnapshotObjPtr virDomainSnapshotObjNew(void)
{
    virDomainSnapshotObjPtr snapshot;

    if (VIR_ALLOC(snapshot) < 0) {
        virReportOOMError();
        return NULL;
    }

    snapshot->refs = 1;

    VIR_DEBUG("obj=%p", snapshot);

    return snapshot;
}

static void virDomainSnapshotObjFree(virDomainSnapshotObjPtr snapshot)
{
    if (!snapshot)
        return;

    VIR_DEBUG("obj=%p", snapshot);

    virDomainSnapshotDefFree(snapshot->def);
    VIR_FREE(snapshot);
}

int virDomainSnapshotObjUnref(virDomainSnapshotObjPtr snapshot)
{
    snapshot->refs--;
    VIR_DEBUG("obj=%p refs=%d", snapshot, snapshot->refs);
    if (snapshot->refs == 0) {
        virDomainSnapshotObjFree(snapshot);
        return 0;
    }
    return snapshot->refs;
}

virDomainSnapshotObjPtr virDomainSnapshotAssignDef(virDomainSnapshotObjListPtr snapshots,
                                                   const virDomainSnapshotDefPtr def)
{
    virDomainSnapshotObjPtr snap;

    if (virHashLookup(snapshots->objs, def->name) != NULL) {
        virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                             _("unexpected domain snapshot %s already exists"),
                             def->name);
        return NULL;
    }

    if (!(snap = virDomainSnapshotObjNew()))
        return NULL;
    snap->def = def;

    if (virHashAddEntry(snapshots->objs, snap->def->name, snap) < 0) {
        VIR_FREE(snap);
        virReportOOMError();
        return NULL;
    }

    return snap;
}

/* Snapshot Obj List functions */
int virDomainSnapshotObjListInit(virDomainSnapshotObjListPtr snapshots)
{
    snapshots->objs = virHashCreate(50);
    if (!snapshots->objs) {
        virReportOOMError();
        return -1;
    }
    return 0;
}

static void virDomainSnapshotObjListDeallocator(void *payload,
                                                const char *name ATTRIBUTE_UNUSED)
{
    virDomainSnapshotObjPtr obj = payload;

    virDomainSnapshotObjUnref(obj);
}

static void virDomainSnapshotObjListDeinit(virDomainSnapshotObjListPtr snapshots)
{
    if (snapshots->objs)
        virHashFree(snapshots->objs, virDomainSnapshotObjListDeallocator);
}

struct virDomainSnapshotNameData {
    int oom;
    int numnames;
    int maxnames;
    char **const names;
};

static void virDomainSnapshotObjListCopyNames(void *payload,
                                              const char *name ATTRIBUTE_UNUSED,
                                              void *opaque)
{
    virDomainSnapshotObjPtr obj = payload;
    struct virDomainSnapshotNameData *data = opaque;

    if (data->oom)
        return;

    if (data->numnames < data->maxnames) {
        if (!(data->names[data->numnames] = strdup(obj->def->name)))
            data->oom = 1;
        else
            data->numnames++;
    }
}

int virDomainSnapshotObjListGetNames(virDomainSnapshotObjListPtr snapshots,
                                     char **const names, int maxnames)
{
    struct virDomainSnapshotNameData data = { 0, 0, maxnames, names };
    int i;

    virHashForEach(snapshots->objs, virDomainSnapshotObjListCopyNames, &data);
    if (data.oom) {
        virReportOOMError();
        goto cleanup;
    }

    return data.numnames;

cleanup:
    for (i = 0; i < data.numnames; i++)
        VIR_FREE(data.names[i]);
    return -1;
}

static void virDomainSnapshotObjListCount(void *payload ATTRIBUTE_UNUSED,
                                          const char *name ATTRIBUTE_UNUSED,
                                          void *data)
{
    int *count = data;

    (*count)++;
}

int virDomainSnapshotObjListNum(virDomainSnapshotObjListPtr snapshots)
{
    int count = 0;

    virHashForEach(snapshots->objs, virDomainSnapshotObjListCount, &count);

    return count;
}

static int virDomainSnapshotObjListSearchName(const void *payload,
                                              const char *name ATTRIBUTE_UNUSED,
                                              const void *data)
{
    virDomainSnapshotObjPtr obj = (virDomainSnapshotObjPtr)payload;
    int want = 0;

    if (STREQ(obj->def->name, (const char *)data))
        want = 1;

    return want;
}

virDomainSnapshotObjPtr virDomainSnapshotFindByName(const virDomainSnapshotObjListPtr snapshots,
                                                    const char *name)
{
    return virHashSearch(snapshots->objs, virDomainSnapshotObjListSearchName, name);
}

void virDomainSnapshotObjListRemove(virDomainSnapshotObjListPtr snapshots,
                                    virDomainSnapshotObjPtr snapshot)
{
    virHashRemoveEntry(snapshots->objs, snapshot->def->name,
                       virDomainSnapshotObjListDeallocator);
}

struct snapshot_has_children {
    char *name;
    int number;
};

static void virDomainSnapshotCountChildren(void *payload,
                                           const char *name ATTRIBUTE_UNUSED,
                                           void *data)
{
    virDomainSnapshotObjPtr obj = payload;
    struct snapshot_has_children *curr = data;

    if (obj->def->parent && STREQ(obj->def->parent, curr->name))
        curr->number++;
}

int virDomainSnapshotHasChildren(virDomainSnapshotObjPtr snap,
                                virDomainSnapshotObjListPtr snapshots)
{
    struct snapshot_has_children children;

    children.name = snap->def->name;
    children.number = 0;
    virHashForEach(snapshots->objs, virDomainSnapshotCountChildren, &children);

    return children.number;
}


int virDomainChrDefForeach(virDomainDefPtr def,
                           bool abortOnError,
                           virDomainChrDefIterator iter,
                           void *opaque)
{
    int i;
    int rc = 0;

    for (i = 0 ; i < def->nserials ; i++) {
        if ((iter)(def,
                   def->serials[i],
                   opaque) < 0)
            rc = -1;

        if (abortOnError && rc != 0)
            goto done;
    }

    for (i = 0 ; i < def->nparallels ; i++) {
        if ((iter)(def,
                   def->parallels[i],
                   opaque) < 0)
            rc = -1;

        if (abortOnError && rc != 0)
            goto done;
    }

    for (i = 0 ; i < def->nchannels ; i++) {
        if ((iter)(def,
                   def->channels[i],
                   opaque) < 0)
            rc = -1;

        if (abortOnError && rc != 0)
            goto done;
    }
    if (def->console) {
        if ((iter)(def,
                   def->console,
                   opaque) < 0)
            rc = -1;

        if (abortOnError && rc != 0)
            goto done;
    }

done:
    return rc;
}


int virDomainSmartcardDefForeach(virDomainDefPtr def,
                                 bool abortOnError,
                                 virDomainSmartcardDefIterator iter,
                                 void *opaque)
{
    int i;
    int rc = 0;

    for (i = 0 ; i < def->nsmartcards ; i++) {
        if ((iter)(def,
                   def->smartcards[i],
                   opaque) < 0)
            rc = -1;

        if (abortOnError && rc != 0)
            goto done;
    }

done:
    return rc;
}


int virDomainDiskDefForeachPath(virDomainDiskDefPtr disk,
                                bool allowProbing,
                                bool ignoreOpenFailure,
                                virDomainDiskDefPathIterator iter,
                                void *opaque)
{
    virHashTablePtr paths;
    int format;
    int ret = -1;
    size_t depth = 0;
    char *nextpath = NULL;

    if (!disk->src || disk->type == VIR_DOMAIN_DISK_TYPE_NETWORK)
        return 0;

    if (disk->driverType) {
        const char *formatStr = disk->driverType;
        if (STREQ(formatStr, "aio"))
            formatStr = "raw"; /* Xen compat */

        if ((format = virStorageFileFormatTypeFromString(formatStr)) < 0) {
            virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                                 _("unknown disk format '%s' for %s"),
                                 disk->driverType, disk->src);
            return -1;
        }
    } else {
        if (allowProbing) {
            format = VIR_STORAGE_FILE_AUTO;
        } else {
            virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                                 _("no disk format for %s and probing is disabled"),
                                 disk->src);
            return -1;
        }
    }

    paths = virHashCreate(5);

    do {
        virStorageFileMetadata meta;
        const char *path = nextpath ? nextpath : disk->src;
        int fd;

        if (iter(disk, path, depth, opaque) < 0)
            goto cleanup;

        if (virHashLookup(paths, path)) {
            virDomainReportError(VIR_ERR_INTERNAL_ERROR,
                                 _("backing store for %s is self-referential"),
                                 disk->src);
            goto cleanup;
        }

        if ((fd = open(path, O_RDONLY)) < 0) {
            if (ignoreOpenFailure) {
                char ebuf[1024];
                VIR_WARN("Ignoring open failure on %s: %s", path,
                         virStrerror(errno, ebuf, sizeof(ebuf)));
                break;
            } else {
                virReportSystemError(errno,
                                     _("unable to open disk path %s"),
                                     path);
                goto cleanup;
            }
        }

        if (virStorageFileGetMetadataFromFD(path, fd, format, &meta) < 0) {
            VIR_FORCE_CLOSE(fd);
            goto cleanup;
        }

        if (VIR_CLOSE(fd) < 0)
            virReportSystemError(errno,
                                 _("could not close file %s"),
                                 path);

        if (virHashAddEntry(paths, path, (void*)0x1) < 0) {
            virReportOOMError();
            goto cleanup;
        }

        depth++;
        nextpath = meta.backingStore;

        /* Stop iterating if we reach a non-file backing store */
        if (nextpath && !meta.backingStoreIsFile) {
            VIR_DEBUG("Stopping iteration on non-file backing store: %s",
                      nextpath);
            break;
        }

        format = meta.backingStoreFormat;

        if (format == VIR_STORAGE_FILE_AUTO &&
            !allowProbing)
            format = VIR_STORAGE_FILE_RAW; /* Stops further recursion */

        /* Allow probing for image formats that are safe */
        if (format == VIR_STORAGE_FILE_AUTO_SAFE)
            format = VIR_STORAGE_FILE_AUTO;
    } while (nextpath);

    ret = 0;

cleanup:
    virHashFree(paths, NULL);
    VIR_FREE(nextpath);

    return ret;
}
