/*
 * domain_conf.h: domain XML processing
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

#ifndef __DOMAIN_CONF_H
# define __DOMAIN_CONF_H

# include <libxml/parser.h>
# include <libxml/tree.h>
# include <libxml/xpath.h>

# include "internal.h"
# include "capabilities.h"
# include "storage_encryption_conf.h"
# include "cpu_conf.h"
# include "util.h"
# include "threads.h"
# include "hash.h"
# include "network.h"
# include "nwfilter_params.h"
# include "nwfilter_conf.h"
# include "macvtap.h"
# include "sysinfo.h"

/* Private component of virDomainXMLFlags */
typedef enum {
   VIR_DOMAIN_XML_INTERNAL_STATUS = (1<<16), /* dump internal domain status information */
} virDomainXMLInternalFlags;

/* Different types of hypervisor */
/* NB: Keep in sync with virDomainVirtTypeToString impl */
enum virDomainVirtType {
    VIR_DOMAIN_VIRT_QEMU,
    VIR_DOMAIN_VIRT_KQEMU,
    VIR_DOMAIN_VIRT_KVM,
    VIR_DOMAIN_VIRT_XEN,
    VIR_DOMAIN_VIRT_LXC,
    VIR_DOMAIN_VIRT_UML,
    VIR_DOMAIN_VIRT_OPENVZ,
    VIR_DOMAIN_VIRT_VSERVER,
    VIR_DOMAIN_VIRT_LDOM,
    VIR_DOMAIN_VIRT_TEST,
    VIR_DOMAIN_VIRT_VMWARE,
    VIR_DOMAIN_VIRT_HYPERV,
    VIR_DOMAIN_VIRT_VBOX,
    VIR_DOMAIN_VIRT_ONE,
    VIR_DOMAIN_VIRT_PHYP,

    VIR_DOMAIN_VIRT_LAST,
};

enum virDomainDeviceAddressType {
    VIR_DOMAIN_DEVICE_ADDRESS_TYPE_NONE,
    VIR_DOMAIN_DEVICE_ADDRESS_TYPE_PCI,
    VIR_DOMAIN_DEVICE_ADDRESS_TYPE_DRIVE,
    VIR_DOMAIN_DEVICE_ADDRESS_TYPE_VIRTIO_SERIAL,
    VIR_DOMAIN_DEVICE_ADDRESS_TYPE_CCID,

    VIR_DOMAIN_DEVICE_ADDRESS_TYPE_LAST
};

typedef struct _virDomainDevicePCIAddress virDomainDevicePCIAddress;
typedef virDomainDevicePCIAddress *virDomainDevicePCIAddressPtr;
struct _virDomainDevicePCIAddress {
    unsigned int domain;
    unsigned int bus;
    unsigned int slot;
    unsigned int function;
};

typedef struct _virDomainDeviceDriveAddress virDomainDeviceDriveAddress;
typedef virDomainDeviceDriveAddress *virDomainDeviceDriveAddressPtr;
struct _virDomainDeviceDriveAddress {
    unsigned int controller;
    unsigned int bus;
    unsigned int unit;
};

typedef struct _virDomainDeviceVirtioSerialAddress virDomainDeviceVirtioSerialAddress;
typedef virDomainDeviceVirtioSerialAddress *virDomainDeviceVirtioSerialAddressPtr;
struct _virDomainDeviceVirtioSerialAddress {
    unsigned int controller;
    unsigned int bus;
    unsigned int port;
};

typedef struct _virDomainDeviceCcidAddress virDomainDeviceCcidAddress;
typedef virDomainDeviceCcidAddress *virDomainDeviceCcidAddressPtr;
struct _virDomainDeviceCcidAddress {
    unsigned int controller;
    unsigned int slot;
};

typedef struct _virDomainDeviceInfo virDomainDeviceInfo;
typedef virDomainDeviceInfo *virDomainDeviceInfoPtr;
struct _virDomainDeviceInfo {
    char *alias;
    int type;
    union {
        virDomainDevicePCIAddress pci;
        virDomainDeviceDriveAddress drive;
        virDomainDeviceVirtioSerialAddress vioserial;
        virDomainDeviceCcidAddress ccid;
    } addr;
};


/* Two types of disk backends */
enum virDomainDiskType {
    VIR_DOMAIN_DISK_TYPE_BLOCK,
    VIR_DOMAIN_DISK_TYPE_FILE,
    VIR_DOMAIN_DISK_TYPE_DIR,
    VIR_DOMAIN_DISK_TYPE_NETWORK,

    VIR_DOMAIN_DISK_TYPE_LAST
};

/* Three types of disk frontend */
enum virDomainDiskDevice {
    VIR_DOMAIN_DISK_DEVICE_DISK,
    VIR_DOMAIN_DISK_DEVICE_CDROM,
    VIR_DOMAIN_DISK_DEVICE_FLOPPY,

    VIR_DOMAIN_DISK_DEVICE_LAST
};

enum virDomainDiskBus {
    VIR_DOMAIN_DISK_BUS_IDE,
    VIR_DOMAIN_DISK_BUS_FDC,
    VIR_DOMAIN_DISK_BUS_SCSI,
    VIR_DOMAIN_DISK_BUS_VIRTIO,
    VIR_DOMAIN_DISK_BUS_XEN,
    VIR_DOMAIN_DISK_BUS_USB,
    VIR_DOMAIN_DISK_BUS_UML,
    VIR_DOMAIN_DISK_BUS_SATA,

    VIR_DOMAIN_DISK_BUS_LAST
};

enum  virDomainDiskCache {
    VIR_DOMAIN_DISK_CACHE_DEFAULT,
    VIR_DOMAIN_DISK_CACHE_DISABLE,
    VIR_DOMAIN_DISK_CACHE_WRITETHRU,
    VIR_DOMAIN_DISK_CACHE_WRITEBACK,

    VIR_DOMAIN_DISK_CACHE_LAST
};

enum  virDomainDiskErrorPolicy {
    VIR_DOMAIN_DISK_ERROR_POLICY_DEFAULT,
    VIR_DOMAIN_DISK_ERROR_POLICY_STOP,
    VIR_DOMAIN_DISK_ERROR_POLICY_IGNORE,
    VIR_DOMAIN_DISK_ERROR_POLICY_ENOSPACE,

    VIR_DOMAIN_DISK_ERROR_POLICY_LAST
};

enum virDomainDiskProtocol {
    VIR_DOMAIN_DISK_PROTOCOL_NBD,
    VIR_DOMAIN_DISK_PROTOCOL_RBD,
    VIR_DOMAIN_DISK_PROTOCOL_SHEEPDOG,

    VIR_DOMAIN_DISK_PROTOCOL_LAST
};

typedef struct _virDomainDiskHostDef virDomainDiskHostDef;
typedef virDomainDiskHostDef *virDomainDiskHostDefPtr;
struct _virDomainDiskHostDef {
    char *name;
    char *port;
};

enum  virDomainDiskIo {
    VIR_DOMAIN_DISK_IO_DEFAULT,
    VIR_DOMAIN_DISK_IO_NATIVE,
    VIR_DOMAIN_DISK_IO_THREADS,

    VIR_DOMAIN_DISK_IO_LAST
};

/* Stores the virtual disk configuration */
typedef struct _virDomainDiskDef virDomainDiskDef;
typedef virDomainDiskDef *virDomainDiskDefPtr;
struct _virDomainDiskDef {
    int type;
    int device;
    int bus;
    char *src;
    char *dst;
    int protocol;
    int nhosts;
    virDomainDiskHostDefPtr hosts;
    char *driverName;
    char *driverType;
    char *serial;
    int cachemode;
    int error_policy;
    int bootIndex;
    int iomode;
    unsigned int readonly : 1;
    unsigned int shared : 1;
    virDomainDeviceInfo info;
    virStorageEncryptionPtr encryption;
};


enum virDomainControllerType {
    VIR_DOMAIN_CONTROLLER_TYPE_IDE,
    VIR_DOMAIN_CONTROLLER_TYPE_FDC,
    VIR_DOMAIN_CONTROLLER_TYPE_SCSI,
    VIR_DOMAIN_CONTROLLER_TYPE_SATA,
    VIR_DOMAIN_CONTROLLER_TYPE_VIRTIO_SERIAL,
    VIR_DOMAIN_CONTROLLER_TYPE_CCID,

    VIR_DOMAIN_CONTROLLER_TYPE_LAST
};


enum virDomainControllerModel {
    VIR_DOMAIN_CONTROLLER_MODEL_AUTO,
    VIR_DOMAIN_CONTROLLER_MODEL_BUSLOGIC,
    VIR_DOMAIN_CONTROLLER_MODEL_LSILOGIC,
    VIR_DOMAIN_CONTROLLER_MODEL_LSISAS1068,
    VIR_DOMAIN_CONTROLLER_MODEL_VMPVSCSI,

    VIR_DOMAIN_CONTROLLER_MODEL_LAST
};

typedef struct _virDomainVirtioSerialOpts virDomainVirtioSerialOpts;
typedef virDomainVirtioSerialOpts *virDomainVirtioSerialOptsPtr;
struct _virDomainVirtioSerialOpts {
    int ports;   /* -1 == undef */
    int vectors; /* -1 == undef */
};

/* Stores the virtual disk controller configuration */
typedef struct _virDomainControllerDef virDomainControllerDef;
typedef virDomainControllerDef *virDomainControllerDefPtr;
struct _virDomainControllerDef {
    int type;
    int idx;
    int model; /* -1 == undef */
    union {
        virDomainVirtioSerialOpts vioserial;
    } opts;
    virDomainDeviceInfo info;
};


/* Two types of disk backends */
enum virDomainFSType {
    VIR_DOMAIN_FS_TYPE_MOUNT,   /* Better named 'bind' */
    VIR_DOMAIN_FS_TYPE_BLOCK,
    VIR_DOMAIN_FS_TYPE_FILE,
    VIR_DOMAIN_FS_TYPE_TEMPLATE,

    VIR_DOMAIN_FS_TYPE_LAST
};

/* Filesystem mount access mode  */
enum virDomainFSAccessMode {
    VIR_DOMAIN_FS_ACCESSMODE_PASSTHROUGH,
    VIR_DOMAIN_FS_ACCESSMODE_MAPPED,
    VIR_DOMAIN_FS_ACCESSMODE_SQUASH,

    VIR_DOMAIN_FS_ACCESSMODE_LAST
};

typedef struct _virDomainFSDef virDomainFSDef;
typedef virDomainFSDef *virDomainFSDefPtr;
struct _virDomainFSDef {
    int type;
    int accessmode;
    char *src;
    char *dst;
    unsigned int readonly : 1;
    virDomainDeviceInfo info;
};


/* 5 different types of networking config */
enum virDomainNetType {
    VIR_DOMAIN_NET_TYPE_USER,
    VIR_DOMAIN_NET_TYPE_ETHERNET,
    VIR_DOMAIN_NET_TYPE_SERVER,
    VIR_DOMAIN_NET_TYPE_CLIENT,
    VIR_DOMAIN_NET_TYPE_MCAST,
    VIR_DOMAIN_NET_TYPE_NETWORK,
    VIR_DOMAIN_NET_TYPE_BRIDGE,
    VIR_DOMAIN_NET_TYPE_INTERNAL,
    VIR_DOMAIN_NET_TYPE_DIRECT,

    VIR_DOMAIN_NET_TYPE_LAST,
};

/* the backend driver used for virtio interfaces */
enum virDomainNetBackendType {
    VIR_DOMAIN_NET_BACKEND_TYPE_DEFAULT, /* prefer kernel, fall back to user */
    VIR_DOMAIN_NET_BACKEND_TYPE_QEMU,    /* userland */
    VIR_DOMAIN_NET_BACKEND_TYPE_VHOST,   /* kernel */

    VIR_DOMAIN_NET_BACKEND_TYPE_LAST,
};

/* the mode type for macvtap devices */
enum virDomainNetdevMacvtapType {
    VIR_DOMAIN_NETDEV_MACVTAP_MODE_VEPA,
    VIR_DOMAIN_NETDEV_MACVTAP_MODE_PRIVATE,
    VIR_DOMAIN_NETDEV_MACVTAP_MODE_BRIDGE,

    VIR_DOMAIN_NETDEV_MACVTAP_MODE_LAST,
};


/* Stores the virtual network interface configuration */
typedef struct _virDomainNetDef virDomainNetDef;
typedef virDomainNetDef *virDomainNetDefPtr;
struct _virDomainNetDef {
    enum virDomainNetType type;
    unsigned char mac[VIR_MAC_BUFLEN];
    char *model;
    enum virDomainNetBackendType backend;
    union {
        struct {
            char *dev;
            char *script;
            char *ipaddr;
        } ethernet;
        struct {
            char *address;
            int port;
        } socket; /* any of NET_CLIENT or NET_SERVER or NET_MCAST */
        struct {
            char *name;
        } network;
        struct {
            char *brname;
            char *script;
            char *ipaddr;
        } bridge;
        struct {
            char *name;
        } internal;
        struct {
            char *linkdev;
            int mode;
            virVirtualPortProfileParams virtPortProfile;
        } direct;
    } data;
    struct {
        bool sndbuf_specified;
        unsigned long sndbuf;
    } tune;
    char *ifname;
    int bootIndex;
    virDomainDeviceInfo info;
    char *filter;
    virNWFilterHashTablePtr filterparams;
};

enum virDomainChrDeviceType {
    VIR_DOMAIN_CHR_DEVICE_TYPE_PARALLEL = 0,
    VIR_DOMAIN_CHR_DEVICE_TYPE_SERIAL,
    VIR_DOMAIN_CHR_DEVICE_TYPE_CONSOLE,
    VIR_DOMAIN_CHR_DEVICE_TYPE_CHANNEL,

    VIR_DOMAIN_CHR_DEVICE_TYPE_LAST,
};

enum virDomainChrChannelTargetType {
    VIR_DOMAIN_CHR_CHANNEL_TARGET_TYPE_GUESTFWD = 0,
    VIR_DOMAIN_CHR_CHANNEL_TARGET_TYPE_VIRTIO,

    VIR_DOMAIN_CHR_CHANNEL_TARGET_TYPE_LAST,
};

enum virDomainChrConsoleTargetType {
    VIR_DOMAIN_CHR_CONSOLE_TARGET_TYPE_SERIAL = 0,
    VIR_DOMAIN_CHR_CONSOLE_TARGET_TYPE_XEN,
    VIR_DOMAIN_CHR_CONSOLE_TARGET_TYPE_UML,
    VIR_DOMAIN_CHR_CONSOLE_TARGET_TYPE_VIRTIO,

    VIR_DOMAIN_CHR_CONSOLE_TARGET_TYPE_LAST,
};

enum virDomainChrType {
    VIR_DOMAIN_CHR_TYPE_NULL,
    VIR_DOMAIN_CHR_TYPE_VC,
    VIR_DOMAIN_CHR_TYPE_PTY,
    VIR_DOMAIN_CHR_TYPE_DEV,
    VIR_DOMAIN_CHR_TYPE_FILE,
    VIR_DOMAIN_CHR_TYPE_PIPE,
    VIR_DOMAIN_CHR_TYPE_STDIO,
    VIR_DOMAIN_CHR_TYPE_UDP,
    VIR_DOMAIN_CHR_TYPE_TCP,
    VIR_DOMAIN_CHR_TYPE_UNIX,
    VIR_DOMAIN_CHR_TYPE_SPICEVMC,

    VIR_DOMAIN_CHR_TYPE_LAST,
};

enum virDomainChrTcpProtocol {
    VIR_DOMAIN_CHR_TCP_PROTOCOL_RAW,
    VIR_DOMAIN_CHR_TCP_PROTOCOL_TELNET,
    VIR_DOMAIN_CHR_TCP_PROTOCOL_TELNETS, /* secure telnet */
    VIR_DOMAIN_CHR_TCP_PROTOCOL_TLS,

    VIR_DOMAIN_CHR_TCP_PROTOCOL_LAST,
};

enum virDomainChrSpicevmcName {
    VIR_DOMAIN_CHR_SPICEVMC_VDAGENT,
    VIR_DOMAIN_CHR_SPICEVMC_SMARTCARD,

    VIR_DOMAIN_CHR_SPICEVMC_LAST,
};

/* The host side information for a character device.  */
typedef struct _virDomainChrSourceDef virDomainChrSourceDef;
typedef virDomainChrSourceDef *virDomainChrSourceDefPtr;
struct _virDomainChrSourceDef {
    int type; /* virDomainChrType */
    union {
        /* no <source> for null, vc, stdio */
        struct {
            char *path;
        } file; /* pty, file, pipe, or device */
        struct {
            char *host;
            char *service;
            bool listen;
            int protocol;
        } tcp;
        struct {
            char *bindHost;
            char *bindService;
            char *connectHost;
            char *connectService;
        } udp;
        struct {
            char *path;
            bool listen;
        } nix;
        int spicevmc;
    } data;
};

/* A complete character device, both host and domain views.  */
typedef struct _virDomainChrDef virDomainChrDef;
typedef virDomainChrDef *virDomainChrDefPtr;
struct _virDomainChrDef {
    int deviceType;
    int targetType;
    union {
        int port; /* parallel, serial, console */
        virSocketAddrPtr addr; /* guestfwd */
        char *name; /* virtio */
    } target;

    virDomainChrSourceDef source;

    virDomainDeviceInfo info;
};

enum virDomainSmartcardType {
    VIR_DOMAIN_SMARTCARD_TYPE_HOST,
    VIR_DOMAIN_SMARTCARD_TYPE_HOST_CERTIFICATES,
    VIR_DOMAIN_SMARTCARD_TYPE_PASSTHROUGH,

    VIR_DOMAIN_SMARTCARD_TYPE_LAST,
};

# define VIR_DOMAIN_SMARTCARD_NUM_CERTIFICATES 3
# define VIR_DOMAIN_SMARTCARD_DEFAULT_DATABASE "/etc/pki/nssdb"

typedef struct _virDomainSmartcardDef virDomainSmartcardDef;
typedef virDomainSmartcardDef *virDomainSmartcardDefPtr;
struct _virDomainSmartcardDef {
    int type; /* virDomainSmartcardType */
    union {
        /* no extra data for 'host' */
        struct {
            char *file[VIR_DOMAIN_SMARTCARD_NUM_CERTIFICATES];
            char *database;
        } cert; /* 'host-certificates' */
        virDomainChrSourceDef passthru; /* 'passthrough' */
    } data;

    virDomainDeviceInfo info;
};

enum virDomainInputType {
    VIR_DOMAIN_INPUT_TYPE_MOUSE,
    VIR_DOMAIN_INPUT_TYPE_TABLET,

    VIR_DOMAIN_INPUT_TYPE_LAST,
};

enum virDomainInputBus {
    VIR_DOMAIN_INPUT_BUS_PS2,
    VIR_DOMAIN_INPUT_BUS_USB,
    VIR_DOMAIN_INPUT_BUS_XEN,

    VIR_DOMAIN_INPUT_BUS_LAST
};

typedef struct _virDomainInputDef virDomainInputDef;
typedef virDomainInputDef *virDomainInputDefPtr;
struct _virDomainInputDef {
    int type;
    int bus;
    virDomainDeviceInfo info;
};

enum virDomainSoundModel {
    VIR_DOMAIN_SOUND_MODEL_SB16,
    VIR_DOMAIN_SOUND_MODEL_ES1370,
    VIR_DOMAIN_SOUND_MODEL_PCSPK,
    VIR_DOMAIN_SOUND_MODEL_AC97,
    VIR_DOMAIN_SOUND_MODEL_ICH6,

    VIR_DOMAIN_SOUND_MODEL_LAST
};

typedef struct _virDomainSoundDef virDomainSoundDef;
typedef virDomainSoundDef *virDomainSoundDefPtr;
struct _virDomainSoundDef {
    int model;
    virDomainDeviceInfo info;
};

enum virDomainWatchdogModel {
    VIR_DOMAIN_WATCHDOG_MODEL_I6300ESB,
    VIR_DOMAIN_WATCHDOG_MODEL_IB700,

    VIR_DOMAIN_WATCHDOG_MODEL_LAST
};

enum virDomainWatchdogAction {
    VIR_DOMAIN_WATCHDOG_ACTION_RESET,
    VIR_DOMAIN_WATCHDOG_ACTION_SHUTDOWN,
    VIR_DOMAIN_WATCHDOG_ACTION_POWEROFF,
    VIR_DOMAIN_WATCHDOG_ACTION_PAUSE,
    VIR_DOMAIN_WATCHDOG_ACTION_DUMP,
    VIR_DOMAIN_WATCHDOG_ACTION_NONE,

    VIR_DOMAIN_WATCHDOG_ACTION_LAST
};

typedef struct _virDomainWatchdogDef virDomainWatchdogDef;
typedef virDomainWatchdogDef *virDomainWatchdogDefPtr;
struct _virDomainWatchdogDef {
    int model;
    int action;
    virDomainDeviceInfo info;
};


enum virDomainVideoType {
    VIR_DOMAIN_VIDEO_TYPE_VGA,
    VIR_DOMAIN_VIDEO_TYPE_CIRRUS,
    VIR_DOMAIN_VIDEO_TYPE_VMVGA,
    VIR_DOMAIN_VIDEO_TYPE_XEN,
    VIR_DOMAIN_VIDEO_TYPE_VBOX,
    VIR_DOMAIN_VIDEO_TYPE_QXL,

    VIR_DOMAIN_VIDEO_TYPE_LAST
};


typedef struct _virDomainVideoAccelDef virDomainVideoAccelDef;
typedef virDomainVideoAccelDef *virDomainVideoAccelDefPtr;
struct _virDomainVideoAccelDef {
    unsigned int support3d :1;
    unsigned int support2d :1;
};


typedef struct _virDomainVideoDef virDomainVideoDef;
typedef virDomainVideoDef *virDomainVideoDefPtr;
struct _virDomainVideoDef {
    int type;
    unsigned int vram;
    unsigned int heads;
    virDomainVideoAccelDefPtr accel;
    virDomainDeviceInfo info;
};

/* 3 possible graphics console modes */
enum virDomainGraphicsType {
    VIR_DOMAIN_GRAPHICS_TYPE_SDL,
    VIR_DOMAIN_GRAPHICS_TYPE_VNC,
    VIR_DOMAIN_GRAPHICS_TYPE_RDP,
    VIR_DOMAIN_GRAPHICS_TYPE_DESKTOP,
    VIR_DOMAIN_GRAPHICS_TYPE_SPICE,

    VIR_DOMAIN_GRAPHICS_TYPE_LAST,
};

typedef struct _virDomainGraphicsAuthDef virDomainGraphicsAuthDef;
typedef virDomainGraphicsAuthDef *virDomainGraphicsAuthDefPtr;
struct _virDomainGraphicsAuthDef {
    char *passwd;
    unsigned int expires: 1; /* Whether there is an expiry time set */
    time_t validTo;  /* seconds since epoch */
};

enum virDomainGraphicsSpiceChannelName {
    VIR_DOMAIN_GRAPHICS_SPICE_CHANNEL_MAIN,
    VIR_DOMAIN_GRAPHICS_SPICE_CHANNEL_DISPLAY,
    VIR_DOMAIN_GRAPHICS_SPICE_CHANNEL_INPUT,
    VIR_DOMAIN_GRAPHICS_SPICE_CHANNEL_CURSOR,
    VIR_DOMAIN_GRAPHICS_SPICE_CHANNEL_PLAYBACK,
    VIR_DOMAIN_GRAPHICS_SPICE_CHANNEL_RECORD,
    VIR_DOMAIN_GRAPHICS_SPICE_CHANNEL_SMARTCARD,

    VIR_DOMAIN_GRAPHICS_SPICE_CHANNEL_LAST
};

enum virDomainGraphicsSpiceChannelMode {
    VIR_DOMAIN_GRAPHICS_SPICE_CHANNEL_MODE_ANY,
    VIR_DOMAIN_GRAPHICS_SPICE_CHANNEL_MODE_SECURE,
    VIR_DOMAIN_GRAPHICS_SPICE_CHANNEL_MODE_INSECURE,

    VIR_DOMAIN_GRAPHICS_SPICE_CHANNEL_MODE_LAST
};

typedef struct _virDomainGraphicsDef virDomainGraphicsDef;
typedef virDomainGraphicsDef *virDomainGraphicsDefPtr;
struct _virDomainGraphicsDef {
    int type;
    union {
        struct {
            int port;
            unsigned int autoport :1;
            char *listenAddr;
            char *keymap;
            char *socket;
            virDomainGraphicsAuthDef auth;
        } vnc;
        struct {
            char *display;
            char *xauth;
            int fullscreen;
        } sdl;
        struct {
            int port;
            char *listenAddr;
            unsigned int autoport :1;
            unsigned int replaceUser :1;
            unsigned int multiUser :1;
        } rdp;
        struct {
            char *display;
            unsigned int fullscreen :1;
        } desktop;
        struct {
            int port;
            int tlsPort;
            char *listenAddr;
            char *keymap;
            virDomainGraphicsAuthDef auth;
            unsigned int autoport :1;
            int channels[VIR_DOMAIN_GRAPHICS_SPICE_CHANNEL_LAST];
        } spice;
    } data;
};

enum virDomainHostdevMode {
    VIR_DOMAIN_HOSTDEV_MODE_SUBSYS,
    VIR_DOMAIN_HOSTDEV_MODE_CAPABILITIES,

    VIR_DOMAIN_HOSTDEV_MODE_LAST,
};

enum virDomainHostdevSubsysType {
    VIR_DOMAIN_HOSTDEV_SUBSYS_TYPE_USB,
    VIR_DOMAIN_HOSTDEV_SUBSYS_TYPE_PCI,

    VIR_DOMAIN_HOSTDEV_SUBSYS_TYPE_LAST
};

typedef struct _virDomainHostdevDef virDomainHostdevDef;
typedef virDomainHostdevDef *virDomainHostdevDefPtr;
struct _virDomainHostdevDef {
    int mode; /* enum virDomainHostdevMode */
    unsigned int managed : 1;
    union {
        struct {
            int type; /* enum virDomainHostdevBusType */
            union {
                struct {
                    unsigned bus;
                    unsigned device;

                    unsigned vendor;
                    unsigned product;
                } usb;
                virDomainDevicePCIAddress pci; /* host address */
            } u;
        } subsys;
        struct {
            /* TBD: struct capabilities see:
             * https://www.redhat.com/archives/libvir-list/2008-July/msg00429.html
             */
            int dummy;
        } caps;
    } source;
    char* target;
    int bootIndex;
    virDomainDeviceInfo info; /* Guest address */
};


enum {
    VIR_DOMAIN_MEMBALLOON_MODEL_VIRTIO,
    VIR_DOMAIN_MEMBALLOON_MODEL_XEN,
    VIR_DOMAIN_MEMBALLOON_MODEL_NONE,

    VIR_DOMAIN_MEMBALLOON_MODEL_LAST
};

typedef struct _virDomainMemballoonDef virDomainMemballoonDef;
typedef virDomainMemballoonDef *virDomainMemballoonDefPtr;
struct _virDomainMemballoonDef {
    int model;
    virDomainDeviceInfo info;
};


enum virDomainSmbiosMode {
    VIR_DOMAIN_SMBIOS_NONE,
    VIR_DOMAIN_SMBIOS_EMULATE,
    VIR_DOMAIN_SMBIOS_HOST,
    VIR_DOMAIN_SMBIOS_SYSINFO,

    VIR_DOMAIN_SMBIOS_LAST
};

/* Flags for the 'type' field in next struct */
enum virDomainDeviceType {
    VIR_DOMAIN_DEVICE_DISK,
    VIR_DOMAIN_DEVICE_FS,
    VIR_DOMAIN_DEVICE_NET,
    VIR_DOMAIN_DEVICE_INPUT,
    VIR_DOMAIN_DEVICE_SOUND,
    VIR_DOMAIN_DEVICE_VIDEO,
    VIR_DOMAIN_DEVICE_HOSTDEV,
    VIR_DOMAIN_DEVICE_WATCHDOG,
    VIR_DOMAIN_DEVICE_CONTROLLER,
    VIR_DOMAIN_DEVICE_GRAPHICS,

    VIR_DOMAIN_DEVICE_LAST,
};

typedef struct _virDomainDeviceDef virDomainDeviceDef;
typedef virDomainDeviceDef *virDomainDeviceDefPtr;
struct _virDomainDeviceDef {
    int type;
    union {
        virDomainDiskDefPtr disk;
        virDomainControllerDefPtr controller;
        virDomainFSDefPtr fs;
        virDomainNetDefPtr net;
        virDomainInputDefPtr input;
        virDomainSoundDefPtr sound;
        virDomainVideoDefPtr video;
        virDomainHostdevDefPtr hostdev;
        virDomainWatchdogDefPtr watchdog;
        virDomainGraphicsDefPtr graphics;
    } data;
};


# define VIR_DOMAIN_MAX_BOOT_DEVS 4

enum virDomainBootOrder {
    VIR_DOMAIN_BOOT_FLOPPY,
    VIR_DOMAIN_BOOT_CDROM,
    VIR_DOMAIN_BOOT_DISK,
    VIR_DOMAIN_BOOT_NET,

    VIR_DOMAIN_BOOT_LAST,
};

enum virDomainBootMenu {
    VIR_DOMAIN_BOOT_MENU_DEFAULT = 0,
    VIR_DOMAIN_BOOT_MENU_ENABLED,
    VIR_DOMAIN_BOOT_MENU_DISABLED,
};

enum virDomainFeature {
    VIR_DOMAIN_FEATURE_ACPI,
    VIR_DOMAIN_FEATURE_APIC,
    VIR_DOMAIN_FEATURE_PAE,
    VIR_DOMAIN_FEATURE_HAP,

    VIR_DOMAIN_FEATURE_LAST
};

enum virDomainLifecycleAction {
    VIR_DOMAIN_LIFECYCLE_DESTROY,
    VIR_DOMAIN_LIFECYCLE_RESTART,
    VIR_DOMAIN_LIFECYCLE_RESTART_RENAME,
    VIR_DOMAIN_LIFECYCLE_PRESERVE,

    VIR_DOMAIN_LIFECYCLE_LAST
};

enum virDomainLifecycleCrashAction {
    VIR_DOMAIN_LIFECYCLE_CRASH_DESTROY,
    VIR_DOMAIN_LIFECYCLE_CRASH_RESTART,
    VIR_DOMAIN_LIFECYCLE_CRASH_RESTART_RENAME,
    VIR_DOMAIN_LIFECYCLE_CRASH_PRESERVE,
    VIR_DOMAIN_LIFECYCLE_CRASH_COREDUMP_DESTROY,
    VIR_DOMAIN_LIFECYCLE_CRASH_COREDUMP_RESTART,

    VIR_DOMAIN_LIFECYCLE_CRASH_LAST
};

/* Operating system configuration data & machine / arch */
typedef struct _virDomainOSDef virDomainOSDef;
typedef virDomainOSDef *virDomainOSDefPtr;
struct _virDomainOSDef {
    char *type;
    char *arch;
    char *machine;
    int nBootDevs;
    int bootDevs[VIR_DOMAIN_BOOT_LAST];
    int bootmenu;
    char *init;
    char *kernel;
    char *initrd;
    char *cmdline;
    char *root;
    char *loader;
    char *bootloader;
    char *bootloaderArgs;
    int smbios_mode;
};

enum virDomainSeclabelType {
    VIR_DOMAIN_SECLABEL_DYNAMIC,
    VIR_DOMAIN_SECLABEL_STATIC,

    VIR_DOMAIN_SECLABEL_LAST,
};

/* Security configuration for domain */
typedef struct _virSecurityLabelDef virSecurityLabelDef;
typedef virSecurityLabelDef *virSecurityLabelDefPtr;
struct _virSecurityLabelDef {
    char *model;        /* name of security model */
    char *label;        /* security label string */
    char *imagelabel;   /* security image label string */
    int type;
};

enum virDomainTimerNameType {
    VIR_DOMAIN_TIMER_NAME_PLATFORM = 0,
    VIR_DOMAIN_TIMER_NAME_PIT,
    VIR_DOMAIN_TIMER_NAME_RTC,
    VIR_DOMAIN_TIMER_NAME_HPET,
    VIR_DOMAIN_TIMER_NAME_TSC,

    VIR_DOMAIN_TIMER_NAME_LAST,
};

enum virDomainTimerTrackType {
    VIR_DOMAIN_TIMER_TRACK_BOOT = 0,
    VIR_DOMAIN_TIMER_TRACK_GUEST,
    VIR_DOMAIN_TIMER_TRACK_WALL,

    VIR_DOMAIN_TIMER_TRACK_LAST,
};

enum virDomainTimerTickpolicyType {
    VIR_DOMAIN_TIMER_TICKPOLICY_DELAY = 0,
    VIR_DOMAIN_TIMER_TICKPOLICY_CATCHUP,
    VIR_DOMAIN_TIMER_TICKPOLICY_MERGE,
    VIR_DOMAIN_TIMER_TICKPOLICY_DISCARD,

    VIR_DOMAIN_TIMER_TICKPOLICY_LAST,
};

enum virDomainTimerModeType {
    VIR_DOMAIN_TIMER_MODE_AUTO = 0,
    VIR_DOMAIN_TIMER_MODE_NATIVE,
    VIR_DOMAIN_TIMER_MODE_EMULATE,
    VIR_DOMAIN_TIMER_MODE_PARAVIRT,
    VIR_DOMAIN_TIMER_MODE_SMPSAFE,

    VIR_DOMAIN_TIMER_MODE_LAST,
};

typedef struct _virDomainTimerCatchupDef virDomainTimerCatchupDef;
typedef virDomainTimerCatchupDef *virDomainTimerCatchupDefPtr;
struct _virDomainTimerCatchupDef {
    unsigned long threshold;
    unsigned long slew;
    unsigned long limit;
};

typedef struct _virDomainTimerDef virDomainTimerDef;
typedef virDomainTimerDef *virDomainTimerDefPtr;
struct _virDomainTimerDef {
    int name;
    int present;    /* unspecified = -1, no = 0, yes = 1 */
    int tickpolicy; /* none|catchup|merge|discard */

    virDomainTimerCatchupDef catchup;

    /* track is only valid for name='platform|rtc' */
    int track;  /* host|guest */

    /* frequency & mode are only valid for name='tsc' */
    unsigned long frequency; /* in Hz, unspecified = 0 */
    int mode;       /* auto|native|emulate|paravirt */
};

enum virDomainClockOffsetType {
    VIR_DOMAIN_CLOCK_OFFSET_UTC = 0,
    VIR_DOMAIN_CLOCK_OFFSET_LOCALTIME = 1,
    VIR_DOMAIN_CLOCK_OFFSET_VARIABLE = 2,
    VIR_DOMAIN_CLOCK_OFFSET_TIMEZONE = 3,

    VIR_DOMAIN_CLOCK_OFFSET_LAST,
};

typedef struct _virDomainClockDef virDomainClockDef;
typedef virDomainClockDef *virDomainClockDefPtr;
struct _virDomainClockDef {
    int offset;

    union {
        /* Adjustment in seconds, relative to UTC, when
         * offset == VIR_DOMAIN_CLOCK_OFFSET_VARIABLE */
        long long adjustment;

        /* Timezone name, when
         * offset == VIR_DOMAIN_CLOCK_OFFSET_LOCALTIME */
        char *timezone;
    } data;

    int ntimers;
    virDomainTimerDefPtr *timers;
};

# define VIR_DOMAIN_CPUMASK_LEN 1024


/* Snapshot state */
typedef struct _virDomainSnapshotDef virDomainSnapshotDef;
typedef virDomainSnapshotDef *virDomainSnapshotDefPtr;
struct _virDomainSnapshotDef {
    char *name;
    char *description;
    char *parent;
    time_t creationTime;
    int state;

    long active;
};

typedef struct _virDomainSnapshotObj virDomainSnapshotObj;
typedef virDomainSnapshotObj *virDomainSnapshotObjPtr;
struct _virDomainSnapshotObj {
    int refs;

    virDomainSnapshotDefPtr def;
};

typedef struct _virDomainSnapshotObjList virDomainSnapshotObjList;
typedef virDomainSnapshotObjList *virDomainSnapshotObjListPtr;
struct _virDomainSnapshotObjList {
    /* name string -> virDomainSnapshotObj  mapping
     * for O(1), lockless lookup-by-name */
    virHashTable *objs;
};

virDomainSnapshotDefPtr virDomainSnapshotDefParseString(const char *xmlStr,
                                                        int newSnapshot);
void virDomainSnapshotDefFree(virDomainSnapshotDefPtr def);
char *virDomainSnapshotDefFormat(char *domain_uuid,
                                 virDomainSnapshotDefPtr def,
                                 int internal);
virDomainSnapshotObjPtr virDomainSnapshotAssignDef(virDomainSnapshotObjListPtr snapshots,
                                                   const virDomainSnapshotDefPtr def);

int virDomainSnapshotObjListInit(virDomainSnapshotObjListPtr objs);
int virDomainSnapshotObjListGetNames(virDomainSnapshotObjListPtr snapshots,
                                     char **const names, int maxnames);
int virDomainSnapshotObjListNum(virDomainSnapshotObjListPtr snapshots);
virDomainSnapshotObjPtr virDomainSnapshotFindByName(const virDomainSnapshotObjListPtr snapshots,
                                                    const char *name);
void virDomainSnapshotObjListRemove(virDomainSnapshotObjListPtr snapshots,
                                    virDomainSnapshotObjPtr snapshot);
int virDomainSnapshotObjUnref(virDomainSnapshotObjPtr snapshot);
int virDomainSnapshotHasChildren(virDomainSnapshotObjPtr snap,
                                virDomainSnapshotObjListPtr snapshots);

/* Guest VM main configuration */
typedef struct _virDomainDef virDomainDef;
typedef virDomainDef *virDomainDefPtr;
struct _virDomainDef {
    int virtType;
    int id;
    unsigned char uuid[VIR_UUID_BUFLEN];
    char *name;
    char *description;

    struct {
        unsigned int weight;
    } blkio;

    struct {
        unsigned long max_balloon;
        unsigned long cur_balloon;
        unsigned long hugepage_backed;
        unsigned long hard_limit;
        unsigned long soft_limit;
        unsigned long min_guarantee;
        unsigned long swap_hard_limit;
    } mem;
    unsigned short vcpus;
    unsigned short maxvcpus;
    int cpumasklen;
    char *cpumask;

    /* These 3 are based on virDomainLifeCycleAction enum flags */
    int onReboot;
    int onPoweroff;
    int onCrash;

    virDomainOSDef os;
    char *emulator;
    int features;

    virDomainClockDef clock;

    int ngraphics;
    virDomainGraphicsDefPtr *graphics;

    int ndisks;
    virDomainDiskDefPtr *disks;

    int ncontrollers;
    virDomainControllerDefPtr *controllers;

    int nfss;
    virDomainFSDefPtr *fss;

    int nnets;
    virDomainNetDefPtr *nets;

    int ninputs;
    virDomainInputDefPtr *inputs;

    int nsounds;
    virDomainSoundDefPtr *sounds;

    int nvideos;
    virDomainVideoDefPtr *videos;

    int nhostdevs;
    virDomainHostdevDefPtr *hostdevs;

    int nsmartcards;
    virDomainSmartcardDefPtr *smartcards;

    int nserials;
    virDomainChrDefPtr *serials;

    int nparallels;
    virDomainChrDefPtr *parallels;

    int nchannels;
    virDomainChrDefPtr *channels;

    /* Only 1 */
    virDomainChrDefPtr console;
    virSecurityLabelDef seclabel;
    virDomainWatchdogDefPtr watchdog;
    virDomainMemballoonDefPtr memballoon;
    virCPUDefPtr cpu;
    virSysinfoDefPtr sysinfo;

    void *namespaceData;
    virDomainXMLNamespace ns;
};

/* Guest VM runtime state */
typedef struct _virDomainObj virDomainObj;
typedef virDomainObj *virDomainObjPtr;
struct _virDomainObj {
    virMutex lock;
    int refs;

    int pid;
    int state;

    unsigned int autostart : 1;
    unsigned int persistent : 1;
    unsigned int updated : 1;

    virDomainDefPtr def; /* The current definition */
    virDomainDefPtr newDef; /* New definition to activate at shutdown */

    virDomainSnapshotObjList snapshots;
    virDomainSnapshotObjPtr current_snapshot;

    void *privateData;
    void (*privateDataFreeFunc)(void *);
};

typedef struct _virDomainObjList virDomainObjList;
typedef virDomainObjList *virDomainObjListPtr;
struct _virDomainObjList {
    /* uuid string -> virDomainObj  mapping
     * for O(1), lockless lookup-by-uuid */
    virHashTable *objs;
};

static inline int
virDomainObjIsActive(virDomainObjPtr dom)
{
    return dom->def->id != -1;
}


int virDomainObjListInit(virDomainObjListPtr objs);
void virDomainObjListDeinit(virDomainObjListPtr objs);

virDomainObjPtr virDomainFindByID(const virDomainObjListPtr doms,
                                  int id);
virDomainObjPtr virDomainFindByUUID(const virDomainObjListPtr doms,
                                    const unsigned char *uuid);
virDomainObjPtr virDomainFindByName(const virDomainObjListPtr doms,
                                    const char *name);


void virDomainGraphicsDefFree(virDomainGraphicsDefPtr def);
void virDomainInputDefFree(virDomainInputDefPtr def);
void virDomainDiskDefFree(virDomainDiskDefPtr def);
void virDomainDiskHostDefFree(virDomainDiskHostDefPtr def);
void virDomainControllerDefFree(virDomainControllerDefPtr def);
void virDomainFSDefFree(virDomainFSDefPtr def);
void virDomainNetDefFree(virDomainNetDefPtr def);
void virDomainSmartcardDefFree(virDomainSmartcardDefPtr def);
void virDomainChrDefFree(virDomainChrDefPtr def);
void virDomainChrSourceDefFree(virDomainChrSourceDefPtr def);
void virDomainSoundDefFree(virDomainSoundDefPtr def);
void virDomainMemballoonDefFree(virDomainMemballoonDefPtr def);
void virDomainWatchdogDefFree(virDomainWatchdogDefPtr def);
void virDomainVideoDefFree(virDomainVideoDefPtr def);
void virDomainHostdevDefFree(virDomainHostdevDefPtr def);
void virDomainDeviceDefFree(virDomainDeviceDefPtr def);
int virDomainDeviceAddressIsValid(virDomainDeviceInfoPtr info,
                                  int type);
int virDomainDevicePCIAddressIsValid(virDomainDevicePCIAddressPtr addr);
int virDomainDeviceDriveAddressIsValid(virDomainDeviceDriveAddressPtr addr);
int virDomainDeviceVirtioSerialAddressIsValid(virDomainDeviceVirtioSerialAddressPtr addr);
int virDomainDeviceInfoIsSet(virDomainDeviceInfoPtr info);
void virDomainDeviceInfoClear(virDomainDeviceInfoPtr info);
void virDomainDefClearPCIAddresses(virDomainDefPtr def);
void virDomainDefClearDeviceAliases(virDomainDefPtr def);

typedef int (*virDomainDeviceInfoCallback)(virDomainDefPtr def,
                                           virDomainDeviceInfoPtr dev,
                                           void *opaque);

int virDomainDeviceInfoIterate(virDomainDefPtr def,
                               virDomainDeviceInfoCallback cb,
                               void *opaque);

void virDomainDefFree(virDomainDefPtr vm);
void virDomainObjRef(virDomainObjPtr vm);
/* Returns 1 if the object was freed, 0 if more refs exist */
int virDomainObjUnref(virDomainObjPtr vm);

/* live == true means def describes an active domain (being migrated or
 * restored) as opposed to a new persistent configuration of the domain */
virDomainObjPtr virDomainAssignDef(virCapsPtr caps,
                                   virDomainObjListPtr doms,
                                   const virDomainDefPtr def,
                                   bool live);
void virDomainObjAssignDef(virDomainObjPtr domain,
                           const virDomainDefPtr def,
                           bool live);
int virDomainObjSetDefTransient(virCapsPtr caps,
                                virDomainObjPtr domain,
                                bool live);
virDomainDefPtr
virDomainObjGetPersistentDef(virCapsPtr caps,
                             virDomainObjPtr domain);
void virDomainRemoveInactive(virDomainObjListPtr doms,
                             virDomainObjPtr dom);

virDomainDeviceDefPtr virDomainDeviceDefParse(virCapsPtr caps,
                                              const virDomainDefPtr def,
                                              const char *xmlStr,
                                              int flags);
virDomainDefPtr virDomainDefParseString(virCapsPtr caps,
                                        const char *xmlStr,
                                        int flags);
virDomainDefPtr virDomainDefParseFile(virCapsPtr caps,
                                      const char *filename,
                                      int flags);
virDomainDefPtr virDomainDefParseNode(virCapsPtr caps,
                                      xmlDocPtr doc,
                                      xmlNodePtr root,
                                      int flags);

virDomainObjPtr virDomainObjParseFile(virCapsPtr caps,
                                      const char *filename);
virDomainObjPtr virDomainObjParseNode(virCapsPtr caps,
                                      xmlDocPtr xml,
                                      xmlNodePtr root);

int virDomainDefAddImplicitControllers(virDomainDefPtr def);

char *virDomainDefFormat(virDomainDefPtr def,
                         int flags);

int virDomainCpuSetParse(const char **str,
                         char sep,
                         char *cpuset,
                         int maxcpu);
char *virDomainCpuSetFormat(char *cpuset,
                            int maxcpu);

int virDomainDiskInsert(virDomainDefPtr def,
                        virDomainDiskDefPtr disk);
void virDomainDiskInsertPreAlloced(virDomainDefPtr def,
                                   virDomainDiskDefPtr disk);
int virDomainDiskDefAssignAddress(virCapsPtr caps, virDomainDiskDefPtr def);

void virDomainDiskRemove(virDomainDefPtr def, size_t i);

int virDomainControllerInsert(virDomainDefPtr def,
                              virDomainControllerDefPtr controller);
void virDomainControllerInsertPreAlloced(virDomainDefPtr def,
                                         virDomainControllerDefPtr controller);

int virDomainSaveXML(const char *configDir,
                     virDomainDefPtr def,
                     const char *xml);

int virDomainSaveConfig(const char *configDir,
                        virDomainDefPtr def);
int virDomainSaveStatus(virCapsPtr caps,
                        const char *statusDir,
                        virDomainObjPtr obj) ATTRIBUTE_RETURN_CHECK;

typedef void (*virDomainLoadConfigNotify)(virDomainObjPtr dom,
                                          int newDomain,
                                          void *opaque);

int virDomainLoadAllConfigs(virCapsPtr caps,
                            virDomainObjListPtr doms,
                            const char *configDir,
                            const char *autostartDir,
                            int liveStatus,
                            virDomainLoadConfigNotify notify,
                            void *opaque);

int virDomainDeleteConfig(const char *configDir,
                          const char *autostartDir,
                          virDomainObjPtr dom);

char *virDomainConfigFile(const char *dir,
                          const char *name);

int virDiskNameToBusDeviceIndex(virDomainDiskDefPtr disk,
                                int *busIdx,
                                int *devIdx);

virDomainFSDefPtr virDomainGetRootFilesystem(virDomainDefPtr def);
int virDomainVideoDefaultType(virDomainDefPtr def);
int virDomainVideoDefaultRAM(virDomainDefPtr def, int type);

int virDomainObjIsDuplicate(virDomainObjListPtr doms,
                            virDomainDefPtr def,
                            unsigned int check_active);

void virDomainObjLock(virDomainObjPtr obj);
void virDomainObjUnlock(virDomainObjPtr obj);

int virDomainObjListNumOfDomains(virDomainObjListPtr doms, int active);

int virDomainObjListGetActiveIDs(virDomainObjListPtr doms,
                                 int *ids,
                                 int maxids);
int virDomainObjListGetInactiveNames(virDomainObjListPtr doms,
                                     char **const names,
                                     int maxnames);

typedef int (*virDomainSmartcardDefIterator)(virDomainDefPtr def,
                                             virDomainSmartcardDefPtr dev,
                                             void *opaque);

int virDomainSmartcardDefForeach(virDomainDefPtr def,
                                 bool abortOnError,
                                 virDomainSmartcardDefIterator iter,
                                 void *opaque);

typedef int (*virDomainChrDefIterator)(virDomainDefPtr def,
                                       virDomainChrDefPtr dev,
                                       void *opaque);

int virDomainChrDefForeach(virDomainDefPtr def,
                           bool abortOnError,
                           virDomainChrDefIterator iter,
                           void *opaque);

typedef int (*virDomainDiskDefPathIterator)(virDomainDiskDefPtr disk,
                                            const char *path,
                                            size_t depth,
                                            void *opaque);

int virDomainDiskDefForeachPath(virDomainDiskDefPtr disk,
                                bool allowProbing,
                                bool ignoreOpenFailure,
                                virDomainDiskDefPathIterator iter,
                                void *opaque);

typedef const char* (*virLifecycleToStringFunc)(int type);
typedef int (*virLifecycleFromStringFunc)(const char *type);

VIR_ENUM_DECL(virDomainVirt)
VIR_ENUM_DECL(virDomainBoot)
VIR_ENUM_DECL(virDomainFeature)
VIR_ENUM_DECL(virDomainLifecycle)
VIR_ENUM_DECL(virDomainLifecycleCrash)
VIR_ENUM_DECL(virDomainDevice)
VIR_ENUM_DECL(virDomainDeviceAddress)
VIR_ENUM_DECL(virDomainDeviceAddressMode)
VIR_ENUM_DECL(virDomainDisk)
VIR_ENUM_DECL(virDomainDiskDevice)
VIR_ENUM_DECL(virDomainDiskBus)
VIR_ENUM_DECL(virDomainDiskCache)
VIR_ENUM_DECL(virDomainDiskErrorPolicy)
VIR_ENUM_DECL(virDomainDiskProtocol)
VIR_ENUM_DECL(virDomainDiskIo)
VIR_ENUM_DECL(virDomainController)
VIR_ENUM_DECL(virDomainControllerModel)
VIR_ENUM_DECL(virDomainFS)
VIR_ENUM_DECL(virDomainFSAccessMode)
VIR_ENUM_DECL(virDomainNet)
VIR_ENUM_DECL(virDomainNetBackend)
VIR_ENUM_DECL(virDomainChrDevice)
VIR_ENUM_DECL(virDomainChrChannelTarget)
VIR_ENUM_DECL(virDomainChrConsoleTarget)
VIR_ENUM_DECL(virDomainSmartcard)
VIR_ENUM_DECL(virDomainChr)
VIR_ENUM_DECL(virDomainChrTcpProtocol)
VIR_ENUM_DECL(virDomainChrSpicevmc)
VIR_ENUM_DECL(virDomainSoundModel)
VIR_ENUM_DECL(virDomainMemballoonModel)
VIR_ENUM_DECL(virDomainSysinfo)
VIR_ENUM_DECL(virDomainSmbiosMode)
VIR_ENUM_DECL(virDomainWatchdogModel)
VIR_ENUM_DECL(virDomainWatchdogAction)
VIR_ENUM_DECL(virDomainVideo)
VIR_ENUM_DECL(virDomainHostdevMode)
VIR_ENUM_DECL(virDomainHostdevSubsys)
VIR_ENUM_DECL(virDomainInput)
VIR_ENUM_DECL(virDomainInputBus)
VIR_ENUM_DECL(virDomainGraphics)
VIR_ENUM_DECL(virDomainGraphicsSpiceChannelName)
VIR_ENUM_DECL(virDomainGraphicsSpiceChannelMode)
/* from libvirt.h */
VIR_ENUM_DECL(virDomainState)
VIR_ENUM_DECL(virDomainSeclabel)
VIR_ENUM_DECL(virDomainClockOffset)

VIR_ENUM_DECL(virDomainNetdevMacvtap)

VIR_ENUM_DECL(virDomainTimerName)
VIR_ENUM_DECL(virDomainTimerTrack)
VIR_ENUM_DECL(virDomainTimerTickpolicy)
VIR_ENUM_DECL(virDomainTimerMode)

#endif /* __DOMAIN_CONF_H */
