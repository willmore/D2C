
/*
 * vmx.c: VMware VMX parsing/formatting functions
 *
 * Copyright (C) 2010-2011 Red Hat, Inc.
 * Copyright (C) 2009-2010 Matthias Bolte <matthias.bolte@googlemail.com>
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
 */

#include <config.h>

#include <c-ctype.h>
#include <libxml/uri.h>

#include "internal.h"
#include "virterror_internal.h"
#include "conf.h"
#include "memory.h"
#include "logging.h"
#include "uuid.h"
#include "vmx.h"

/*

mapping:

domain-xml                        <=>   vmx


                                        config.version = "8"                    # essential
                                        virtualHW.version = "4"                 # essential for ESX 3.5
                                        virtualHW.version = "7"                 # essential for ESX 4.0


???                               <=>   guestOS = "<value>"                     # essential, FIXME: not representable
def->id = <value>                 <=>   ???                                     # not representable
def->uuid = <value>               <=>   uuid.bios = "<value>"
def->name = <value>               <=>   displayName = "<value>"
def->mem.max_balloon = <value kilobyte>    <=>   memsize = "<value megabyte>"            # must be a multiple of 4, defaults to 32
def->mem.cur_balloon = <value kilobyte>    <=>   sched.mem.max = "<value megabyte>"      # defaults to "unlimited" -> def->mem.cur_balloon = def->mem.max_balloon
def->mem.min_guarantee = <value kilobyte>  <=>   sched.mem.minsize = "<value megabyte>"  # defaults to 0
def->maxvcpus = <value>           <=>   numvcpus = "<value>"                    # must be 1 or a multiple of 2, defaults to 1
def->cpumask = <uint list>        <=>   sched.cpu.affinity = "<uint list>"



################################################################################
## os ##########################################################################

def->os

->type = "hvm"
->arch = <arch>                   <=>   guestOS = "<value>"                     # if <value> ends with -64 than <arch> is x86_64, otherwise <arch> is i686
->machine
->nBootDevs
->bootDevs
->init
->kernel
->initrd
->cmdline
->root
->loader
->bootloader
->bootloaderArgs
->smbios_mode                     <=>   smbios.reflecthost = "<value>"          # <value> == true means SMBIOS_HOST, otherwise it's SMBIOS_EMULATE, defaults to "false"



################################################################################
## disks #######################################################################

                                        scsi[0..3]:[0..6,8..15] -> <controller>:<unit> with 1 bus per controller
                                        ide[0..1]:[0..1]        -> <bus>:<unit> with 1 controller
                                        floppy[0..1]            -> <unit> with 1 controller and 1 bus per controller

def->disks[0]...

## disks: scsi hard drive from .vmdk image #####################################

                                        scsi0.present = "true"                  # defaults to "false"
                                        scsi0:0.present = "true"                # defaults to "false"
                                        scsi0:0.startConnected = "true"         # defaults to "true"

???                               <=>   scsi0:0.mode = "persistent"             # defaults to "persistent"
                                        scsi0:0.mode = "undoable"
                                        scsi0:0.mode = "independent-persistent"
                                        scsi0:0.mode = "independent-nonpersistent"

...
->type = _DISK_TYPE_FILE          <=>   scsi0:0.deviceType = "scsi-hardDisk"    # defaults to ?
->device = _DISK_DEVICE_DISK      <=>   scsi0:0.deviceType = "scsi-hardDisk"    # defaults to ?
->bus = _DISK_BUS_SCSI
->src = <value>.vmdk              <=>   scsi0:0.fileName = "<value>.vmdk"
->dst = sd[<controller> * 15 + <unit> mapped to [a-z]+]
->driverName = <driver>           <=>   scsi0.virtualDev = "<driver>"           # default depends on guestOS value
->driverType
->cachemode                       <=>   scsi0:0.writeThrough = "<value>"        # defaults to false, true -> _DISK_CACHE_WRITETHRU, false _DISK_CACHE_DEFAULT
->readonly
->shared
->slotnum


## disks: ide hard drive from .vmdk image ######################################

                                        ide0:0.present = "true"                 # defaults to "false"
                                        ide0:0.startConnected = "true"          # defaults to "true"

???                               <=>   ide0:0.mode = "persistent"              # defaults to "persistent"
                                        ide0:0.mode = "undoable"
                                        ide0:0.mode = "independent-persistent"
                                        ide0:0.mode = "independent-nonpersistent"

...
->type = _DISK_TYPE_FILE          <=>   ide0:0.deviceType = "ata-hardDisk"      # defaults to ?
->device = _DISK_DEVICE_DISK      <=>   ide0:0.deviceType = "ata-hardDisk"      # defaults to ?
->bus = _DISK_BUS_IDE
->src = <value>.vmdk              <=>   ide0:0.fileName = "<value>.vmdk"
->dst = hd[<bus> * 2 + <unit> mapped to [a-z]+]
->driverName
->driverType
->cachemode                       <=>   ide0:0.writeThrough = "<value>"         # defaults to false, true -> _DISK_CACHE_WRITETHRU, false _DISK_CACHE_DEFAULT
->readonly
->shared
->slotnum


## disks: scsi cdrom from .iso image ###########################################

                                        scsi0.present = "true"                  # defaults to "false"
                                        scsi0:0.present = "true"                # defaults to "false"
                                        scsi0:0.startConnected = "true"         # defaults to "true"

...
->type = _DISK_TYPE_FILE          <=>   scsi0:0.deviceType = "cdrom-image"      # defaults to ?
->device = _DISK_DEVICE_CDROM     <=>   scsi0:0.deviceType = "cdrom-image"      # defaults to ?
->bus = _DISK_BUS_SCSI
->src = <value>.iso               <=>   scsi0:0.fileName = "<value>.iso"
->dst = sd[<controller> * 15 + <unit> mapped to [a-z]+]
->driverName = <driver>           <=>   scsi0.virtualDev = "<driver>"           # default depends on guestOS value
->driverType
->cachemode
->readonly
->shared
->slotnum


## disks: ide cdrom from .iso image ############################################

                                        ide0:0.present = "true"                 # defaults to "false"
                                        ide0:0.startConnected = "true"          # defaults to "true"

...
->type = _DISK_TYPE_FILE          <=>   ide0:0.deviceType = "cdrom-image"       # defaults to ?
->device = _DISK_DEVICE_CDROM     <=>   ide0:0.deviceType = "cdrom-image"       # defaults to ?
->bus = _DISK_BUS_IDE
->src = <value>.iso               <=>   ide0:0.fileName = "<value>.iso"
->dst = hd[<bus> * 2 + <unit> mapped to [a-z]+]
->driverName
->driverType
->cachemode
->readonly
->shared
->slotnum


## disks: scsi cdrom from host device ##########################################

                                        scsi0.present = "true"                  # defaults to "false"
                                        scsi0:0.present = "true"                # defaults to "false"
                                        scsi0:0.startConnected = "true"         # defaults to "true"

...
->type = _DISK_TYPE_BLOCK         <=>   scsi0:0.deviceType = "atapi-cdrom"      # defaults to ?
->device = _DISK_DEVICE_CDROM     <=>   scsi0:0.deviceType = "atapi-cdrom"      # defaults to ?
->bus = _DISK_BUS_SCSI
->src = <value>                   <=>   scsi0:0.fileName = "<value>"            # e.g. "/dev/scd0" ?
->dst = sd[<controller> * 15 + <unit> mapped to [a-z]+]
->driverName = <driver>           <=>   scsi0.virtualDev = "<driver>"           # default depends on guestOS value
->driverType
->cachemode
->readonly
->shared
->slotnum


## disks: ide cdrom from host device ###########################################

                                        ide0:0.present = "true"                 # defaults to "false"
                                        ide0:0.startConnected = "true"          # defaults to "true"
                                        ide0:0.clientDevice = "false"           # defaults to "false"

...
->type = _DISK_TYPE_BLOCK         <=>   ide0:0.deviceType = "atapi-cdrom"       # defaults to ?
->device = _DISK_DEVICE_CDROM     <=>   ide0:0.deviceType = "atapi-cdrom"       # defaults to ?
->bus = _DISK_BUS_IDE
->src = <value>                   <=>   ide0:0.fileName = "<value>"             # e.g. "/dev/scd0"
->dst = hd[<bus> * 2 + <unit> mapped to [a-z]+]
->driverName
->driverType
->cachemode
->readonly
->shared
->slotnum


## disks: floppy from .flp image ###############################################

                                        floppy0.present = "true"                # defaults to "true"
                                        floppy0.startConnected = "true"         # defaults to "true"
                                        floppy0.clientDevice = "false"          # defaults to "false"

...
->type = _DISK_TYPE_FILE          <=>   floppy0.fileType = "file"               # defaults to ?
->device = _DISK_DEVICE_FLOPPY
->bus = _DISK_BUS_FDC
->src = <value>.flp               <=>   floppy0.fileName = "<value>.flp"
->dst = fd[<unit> mapped to [a-z]+]
->driverName
->driverType
->cachemode
->readonly
->shared
->slotnum


## disks: floppy from host device ##############################################

                                        floppy0.present = "true"                # defaults to "true"
                                        floppy0.startConnected = "true"         # defaults to "true"
                                        floppy0.clientDevice = "false"          # defaults to "false"

...
->type = _DISK_TYPE_BLOCK         <=>   floppy0.fileType = "device"             # defaults to ?
->device = _DISK_DEVICE_FLOPPY
->bus = _DISK_BUS_FDC
->src = <value>                   <=>   floppy0.fileName = "<value>"            # e.g. "/dev/fd0"
->dst = fd[<unit> mapped to [a-z]+]
->driverName
->driverType
->cachemode
->readonly
->shared
->slotnum



################################################################################
## nets ########################################################################

                                        ethernet[0..3] -> <controller>

                                        ethernet0.present = "true"              # defaults to "false"
                                        ethernet0.startConnected = "true"       # defaults to "true"

                                        ethernet0.networkName = "VM Network"    # FIXME

def->nets[0]...
->model = <model>                 <=>   ethernet0.virtualDev = "<model>"        # default depends on guestOS value
                                        ethernet0.features = "15"               # if present and virtualDev is "vmxnet" => vmxnet2 (enhanced)


                                        ethernet0.addressType = "generated"     # default to "generated"
->mac = <value>                   <=>   ethernet0.generatedAddress = "<value>"
                                        ethernet0.generatedAddressOffset = "0"  # ?


                                        ethernet0.addressType = "static"        # default to "generated"
->mac = <value>                   <=>   ethernet0.address = "<value>"


                                        ethernet0.addressType = "vpx"           # default to "generated"
->mac = <value>                   <=>   ethernet0.generatedAddress = "<value>"


                                        ethernet0.addressType = "static"        # default to "generated"
->mac = <value>                   <=>   ethernet0.address = "<value>"
                                        ethernet0.checkMACAddress = "false"     # mac address outside the VMware prefixes

                                                                                # 00:0c:29 prefix for autogenerated mac's -> ethernet0.addressType = "generated"
                                                                                # 00:50:56 prefix for manual configured mac's
                                                                                #          00:50:56:00:00:00 - 00:50:56:3f:ff:ff -> ethernet0.addressType = "static"
                                                                                #          00:50:56:80:00:00 - 00:50:56:bf:ff:ff -> ethernet0.addressType = "vpx"
                                                                                # 00:05:69 old prefix from esx 1.5


## nets: bridged ###############################################################

...
->type = _NET_TYPE_BRIDGE         <=>   ethernet0.connectionType = "bridged"    # defaults to "bridged"
->data.bridge.brname = <value>    <=>   ethernet0.networkName = "<value>"


## nets: hostonly ##############################################################

...                                                                             # FIXME: Investigate if ESX supports this
->type = _NET_TYPE_NETWORK        <=>   ethernet0.connectionType = "hostonly"   # defaults to "bridged"


## nets: nat ###################################################################

...                                                                             # FIXME: Investigate if ESX supports this
->type = _NET_TYPE_USER           <=>   ethernet0.connectionType = "nat"        # defaults to "bridged"


## nets: custom ################################################################

...
->type = _NET_TYPE_BRIDGE         <=>   ethernet0.connectionType = "custom"     # defaults to "bridged"
->data.bridge.brname = <value>    <=>   ethernet0.networkName = "<value>"
->ifname = <value>                <=>   ethernet0.vnet = "<value>"



################################################################################
## video #######################################################################

def->videos[0]...
->type = _VIDEO_TYPE_VMVGA
->vram = <value kilobyte>         <=>   svga.vramSize = "<value byte>"
->heads = 1



################################################################################
## serials #####################################################################

                                        serial[0..3] -> <port>

                                        serial0.present = "true"                # defaults to "false"
                                        serial0.startConnected = "true"         # defaults to "true"

def->serials[0]...
->target.port = <port>


## serials: device #############################################################

->type = _CHR_TYPE_DEV            <=>   serial0.fileType = "device"
->data.file.path = <value>        <=>   serial0.fileName = "<value>"            # e.g. "/dev/ttyS0"
???                               <=>   serial0.tryNoRxLoss = "false"           # defaults to "false", FIXME: not representable
???                               <=>   serial0.yieldOnMsrRead = "true"         # defaults to "false", FIXME: not representable


## serials: file ###############################################################

->type = _CHR_TYPE_FILE           <=>   serial0.fileType = "file"
->data.file.path = <value>        <=>   serial0.fileName = "<value>"            # e.g. "serial0.file"
???                               <=>   serial0.tryNoRxLoss = "false"           # defaults to "false", FIXME: not representable
???                               <=>   serial0.yieldOnMsrRead = "true"         # defaults to "false", FIXME: not representable


## serials: pipe, far end -> app ###############################################

->type = _CHR_TYPE_PIPE           <=>   serial0.fileType = "pipe"
->data.file.path = <value>        <=>   serial0.fileName = "<value>"            # e.g. "serial0.pipe"
???                               <=>   serial0.pipe.endPoint = "client"        # defaults to ?, FIXME: not representable
???                               <=>   serial0.tryNoRxLoss = "true"            # defaults to "false", FIXME: not representable
???                               <=>   serial0.yieldOnMsrRead = "true"         # defaults to "false", FIXME: not representable

->type = _CHR_TYPE_PIPE           <=>   serial0.fileType = "pipe"
->data.file.path = <value>        <=>   serial0.fileName = "<value>"            # e.g. "serial0.pipe"
???                               <=>   serial0.pipe.endPoint = "server"        # defaults to ?, FIXME: not representable
???                               <=>   serial0.tryNoRxLoss = "true"            # defaults to "false", FIXME: not representable
???                               <=>   serial0.yieldOnMsrRead = "true"         # defaults to "false", FIXME: not representable


## serials: pipe, far end -> vm ################################################

->type = _CHR_TYPE_PIPE           <=>   serial0.fileType = "pipe"
->data.file.path = <value>        <=>   serial0.fileName = "<value>"            # e.g. "serial0.pipe"
???                               <=>   serial0.pipe.endPoint = "client"        # defaults to ?, FIXME: not representable
???                               <=>   serial0.tryNoRxLoss = "false"           # defaults to "false", FIXME: not representable
???                               <=>   serial0.yieldOnMsrRead = "true"         # defaults to "false", FIXME: not representable

->type = _CHR_TYPE_PIPE           <=>   serial0.fileType = "pipe"
->data.file.path = <value>        <=>   serial0.fileName = "<value>"            # e.g. "serial0.pipe"
???                               <=>   serial0.pipe.endPoint = "server"        # defaults to ?, FIXME: not representable
???                               <=>   serial0.tryNoRxLoss = "false"           # defaults to "false", FIXME: not representable
???                               <=>   serial0.yieldOnMsrRead = "true"         # defaults to "false", FIXME: not representable


## serials: network, server (since vSphere 4.1) ################################

->type = _CHR_TYPE_TCP            <=>   serial0.fileType = "network"

->data.tcp.host = <host>          <=>   serial0.fileName = "<protocol>://<host>:<service>"
->data.tcp.service = <service>                                                  # e.g. "telnet://0.0.0.0:42001"
->data.tcp.protocol = <protocol>

->data.tcp.listen = true          <=>   serial0.network.endPoint = "server"     # defaults to "server"

???                               <=>   serial0.vspc = "foobar"                 # defaults to <not present>, FIXME: not representable
???                               <=>   serial0.tryNoRxLoss = "false"           # defaults to "false", FIXME: not representable
???                               <=>   serial0.yieldOnMsrRead = "true"         # defaults to "false", FIXME: not representable


## serials: network, client (since vSphere 4.1) ################################

->type = _CHR_TYPE_TCP            <=>   serial0.fileType = "network"

->data.tcp.host = <host>          <=>   serial0.fileName = "<protocol>://<host>:<service>"
->data.tcp.service = <service>                                                  # e.g. "telnet://192.168.0.17:42001"
->data.tcp.protocol = <protocol>

->data.tcp.listen = false         <=>   serial0.network.endPoint = "client"     # defaults to "server"

???                               <=>   serial0.vspc = "foobar"                 # defaults to <not present>, FIXME: not representable
???                               <=>   serial0.tryNoRxLoss = "false"           # defaults to "false", FIXME: not representable
???                               <=>   serial0.yieldOnMsrRead = "true"         # defaults to "false", FIXME: not representable



################################################################################
## parallels ###################################################################

                                        parallel[0..2] -> <port>

                                        parallel0.present = "true"              # defaults to "false"
                                        parallel0.startConnected = "true"       # defaults to "true"

def->parallels[0]...
->target.port = <port>


## parallels: device #############################################################

->type = _CHR_TYPE_DEV            <=>   parallel0.fileType = "device"
->data.file.path = <value>        <=>   parallel0.fileName = "<value>"          # e.g. "/dev/parport0"
???                               <=>   parallel0.bidirectional = "<value>"     # defaults to ?, FIXME: not representable


## parallels: file #############################################################

->type = _CHR_TYPE_FILE           <=>   parallel0.fileType = "file"
->data.file.path = <value>        <=>   parallel0.fileName = "<value>"          # e.g. "parallel0.file"
???                               <=>   parallel0.bidirectional = "<value>"     # must be "false" for fileType = "file", FIXME: not representable



################################################################################
## sound #######################################################################

                                        sound.present = "true"                  # defaults to "false"
                                        sound.startConnected = "true"           # defaults to "true"
                                        sound.autodetect = "true"
                                        sound.fileName = "-1"

                                        FIXME: Investigate if ESX supports this,
                                               at least the VI Client GUI has no
                                               options to add a sound device, but
                                               the VI API contains a VirtualSoundCard

*/

#define VIR_FROM_THIS VIR_FROM_NONE

#define VMX_ERROR(code, ...)                                                  \
    virReportErrorHelper(NULL, VIR_FROM_NONE, code, __FILE__, __FUNCTION__,   \
                         __LINE__, __VA_ARGS__)

#define VMX_BUILD_NAME_EXTRA(_suffix, _extra)                                 \
    snprintf(_suffix##_name, sizeof(_suffix##_name), "%s."_extra, prefix);

#define VMX_BUILD_NAME(_suffix)                                               \
    VMX_BUILD_NAME_EXTRA(_suffix, #_suffix)

/* directly map the virDomainControllerModel to virVMXSCSIControllerModel,
 * this is good enough for now because all virDomainControllerModel values
 * are actually SCSI controller models in the ESX case */
VIR_ENUM_DECL(virVMXSCSIControllerModel)
VIR_ENUM_IMPL(virVMXSCSIControllerModel, VIR_DOMAIN_CONTROLLER_MODEL_LAST,
              "auto", /* just to match virDomainControllerModel, will never be used */
              "buslogic",
              "lsilogic",
              "lsisas1068",
              "pvscsi");



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Helpers
 */

char *
virVMXEscapeHex(const char *string, char escape, const char *special)
{
    char *escaped = NULL;
    size_t length = 1; /* 1 byte for termination */
    const char *tmp1 = string;
    char *tmp2;

    /* Calculate length of escaped string */
    while (*tmp1 != '\0') {
        if (*tmp1 == escape || strspn(tmp1, special) > 0) {
            length += 2;
        }

        ++tmp1;
        ++length;
    }

    if (VIR_ALLOC_N(escaped, length) < 0) {
        virReportOOMError();
        return NULL;
    }

    tmp1 = string; /* reading from this one */
    tmp2 = escaped; /* writing to this one */

    /* Escape to 'cXX' where c is the escape char and X is a hex digit */
    while (*tmp1 != '\0') {
        if (*tmp1 == escape || strspn(tmp1, special) > 0) {
            *tmp2++ = escape;

            snprintf(tmp2, 3, "%02x", (unsigned int)*tmp1);

            tmp2 += 2;
        } else {
            *tmp2++ = *tmp1;
        }

        ++tmp1;
    }

    *tmp2 = '\0';

    return escaped;
}



int
virVMXUnescapeHex(char *string, char escape)
{
    char *tmp1 = string; /* reading from this one */
    char *tmp2 = string; /* writing to this one */

    /* Unescape from 'cXX' where c is the escape char and X is a hex digit */
    while (*tmp1 != '\0') {
        if (*tmp1 == escape) {
            if (!c_isxdigit(tmp1[1]) || !c_isxdigit(tmp1[2])) {
                return -1;
            }

            *tmp2++ = virHexToBin(tmp1[1]) * 16 + virHexToBin(tmp1[2]);
            tmp1 += 3;
        } else {
            *tmp2++ = *tmp1++;
        }
    }

    *tmp2 = '\0';

    return 0;
}



char *
virVMXConvertToUTF8(const char *encoding, const char *string)
{
    char *result = NULL;
    xmlCharEncodingHandlerPtr handler;
    xmlBufferPtr input;
    xmlBufferPtr utf8;

    handler = xmlFindCharEncodingHandler(encoding);

    if (handler == NULL) {
        VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                  _("libxml2 doesn't handle %s encoding"), encoding);
        return NULL;
    }

    input = xmlBufferCreateStatic((char *)string, strlen(string));
    utf8 = xmlBufferCreate();

    if (xmlCharEncInFunc(handler, utf8, input) < 0) {
        VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                  _("Could not convert from %s to UTF-8 encoding"), encoding);
        goto cleanup;
    }

    result = (char *)utf8->content;
    utf8->content = NULL;

  cleanup:
    xmlCharEncCloseFunc(handler);
    xmlBufferFree(input);
    xmlBufferFree(utf8);

    return result;
}



static int
virVMXGetConfigString(virConfPtr conf, const char *name, char **string,
                      bool optional)
{
    virConfValuePtr value;

    *string = NULL;
    value = virConfGetValue(conf, name);

    if (value == NULL) {
        if (optional) {
            return 0;
        }

        VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                  _("Missing essential config entry '%s'"), name);
        return -1;
    }

    if (value->type != VIR_CONF_STRING) {
        VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                  _("Config entry '%s' must be a string"), name);
        return -1;
    }

    if (value->str == NULL) {
        if (optional) {
            return 0;
        }

        VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                  _("Missing essential config entry '%s'"), name);
        return -1;
    }

    *string = strdup(value->str);

    if (*string == NULL) {
        virReportOOMError();
        return -1;
    }

    return 0;
}



static int
virVMXGetConfigUUID(virConfPtr conf, const char *name, unsigned char *uuid,
                    bool optional)
{
    virConfValuePtr value;

    value = virConfGetValue(conf, name);

    if (value == NULL) {
        if (optional) {
            return 0;
        } else {
            VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                      _("Missing essential config entry '%s'"), name);
            return -1;
        }
    }

    if (value->type != VIR_CONF_STRING) {
        VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                  _("Config entry '%s' must be a string"), name);
        return -1;
    }

    if (value->str == NULL) {
        if (optional) {
            return 0;
        } else {
            VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                      _("Missing essential config entry '%s'"), name);
            return -1;
        }
    }

    if (virUUIDParse(value->str, uuid) < 0) {
        VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                  _("Could not parse UUID from string '%s'"), value->str);
        return -1;
    }

    return 0;
}



static int
virVMXGetConfigLong(virConfPtr conf, const char *name, long long *number,
                    long long default_, bool optional)
{
    virConfValuePtr value;

    *number = default_;
    value = virConfGetValue(conf, name);

    if (value == NULL) {
        if (optional) {
            return 0;
        } else {
            VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                      _("Missing essential config entry '%s'"), name);
            return -1;
        }
    }

    if (value->type == VIR_CONF_STRING) {
        if (value->str == NULL) {
            if (optional) {
                return 0;
            } else {
                VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                          _("Missing essential config entry '%s'"), name);
                return -1;
            }
        }

        if (STREQ(value->str, "unlimited")) {
            *number = -1;
        } else if (virStrToLong_ll(value->str, NULL, 10, number) < 0) {
            VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                      _("Config entry '%s' must represent an integer value"),
                      name);
            return -1;
        }
    } else {
        VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                  _("Config entry '%s' must be a string"), name);
        return -1;
    }

    return 0;
}



static int
virVMXGetConfigBoolean(virConfPtr conf, const char *name, bool *boolean_,
                       bool default_, bool optional)
{
    virConfValuePtr value;

    *boolean_ = default_;
    value = virConfGetValue(conf, name);

    if (value == NULL) {
        if (optional) {
            return 0;
        } else {
            VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                      _("Missing essential config entry '%s'"), name);
            return -1;
        }
    }

    if (value->type == VIR_CONF_STRING) {
        if (value->str == NULL) {
            if (optional) {
                return 0;
            } else {
                VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                          _("Missing essential config entry '%s'"), name);
                return -1;
            }
        }

        if (STRCASEEQ(value->str, "true")) {
            *boolean_ = 1;
        } else if (STRCASEEQ(value->str, "false")) {
            *boolean_ = 0;
        } else {
            VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                      _("Config entry '%s' must represent a boolean value "
                        "(true|false)"), name);
            return -1;
        }
    } else {
        VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                  _("Config entry '%s' must be a string"), name);
        return -1;
    }

    return 0;
}



static int
virVMXSCSIDiskNameToControllerAndUnit(const char *name, int *controller, int *unit)
{
    int idx;

    if (! STRPREFIX(name, "sd")) {
        VMX_ERROR(VIR_ERR_INTERNAL_ERROR, "%s",
                  _("Expecting domain XML attribute 'dev' of entry "
                    "'devices/disk/target' to start with 'sd'"));
        return -1;
    }

    idx = virDiskNameToIndex(name);

    if (idx < 0) {
        VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                  _("Could not parse valid disk index from '%s'"), name);
        return -1;
    }

    /* Each of the 4 SCSI controllers has 1 bus with 15 units each for devices */
    if (idx >= (4 * 15)) {
        VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                  _("SCSI disk index (parsed from '%s') is too large"), name);
        return -1;
    }

    *controller = idx / 15;
    *unit = idx % 15;

    /* Skip the controller ifself at unit 7 */
    if (*unit >= 7) {
        ++(*unit);
    }

    return 0;
}



static int
virVMXIDEDiskNameToBusAndUnit(const char *name, int *bus, int *unit)
{
    int idx;

    if (! STRPREFIX(name, "hd")) {
        VMX_ERROR(VIR_ERR_INTERNAL_ERROR, "%s",
                  _("Expecting domain XML attribute 'dev' of entry "
                    "'devices/disk/target' to start with 'hd'"));
        return -1;
    }

    idx = virDiskNameToIndex(name);

    if (idx < 0) {
        VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                  _("Could not parse valid disk index from '%s'"), name);
        return -1;
    }

    /* The IDE controller has 2 buses with 2 units each for devices */
    if (idx >= (2 * 2)) {
        VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                  _("IDE disk index (parsed from '%s') is too large"), name);
        return -1;
    }

    *bus = idx / 2;
    *unit = idx % 2;

    return 0;
}



static int
virVMXFloppyDiskNameToUnit(const char *name, int *unit)
{
    int idx;

    if (! STRPREFIX(name, "fd")) {
        VMX_ERROR(VIR_ERR_INTERNAL_ERROR, "%s",
                  _("Expecting domain XML attribute 'dev' of entry "
                    "'devices/disk/target' to start with 'fd'"));
        return -1;
    }

    idx = virDiskNameToIndex(name);

    if (idx < 0) {
        VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                  _("Could not parse valid disk index from '%s'"), name);
        return -1;
    }

    /* The FDC controller has 1 bus with 2 units for devices */
    if (idx >= 2) {
        VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                  _("Floppy disk index (parsed from '%s') is too large"), name);
        return -1;
    }

    *unit = idx;

    return 0;
}



static int
virVMXVerifyDiskAddress(virCapsPtr caps, virDomainDiskDefPtr disk)
{
    virDomainDiskDef def;
    virDomainDeviceDriveAddressPtr drive;

    memset(&def, 0, sizeof(def));

    if (disk->info.type != VIR_DOMAIN_DEVICE_ADDRESS_TYPE_DRIVE) {
        VMX_ERROR(VIR_ERR_CONFIG_UNSUPPORTED,
                  _("Unsupported disk address type '%s'"),
                  virDomainDeviceAddressTypeToString(disk->info.type));
        return -1;
    }

    drive = &disk->info.addr.drive;

    def.dst = disk->dst;
    def.bus = disk->bus;

    if (virDomainDiskDefAssignAddress(caps, &def) < 0) {
        VMX_ERROR(VIR_ERR_INTERNAL_ERROR, "%s",
                  _("Could not verify disk address"));
        return -1;
    }

    if (def.info.addr.drive.controller != drive->controller ||
        def.info.addr.drive.bus != drive->bus ||
        def.info.addr.drive.unit != drive->unit) {
        VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                  _("Disk address %d:%d:%d doesn't match target device '%s'"),
                  drive->controller, drive->bus, drive->unit, disk->dst);
        return -1;
    }

    /* drive->{controller|bus|unit} is unsigned, no >= 0 checks are necessary */
    if (disk->bus == VIR_DOMAIN_DISK_BUS_SCSI) {
        if (drive->controller > 3) {
            VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                      _("SCSI controller index %d out of [0..3] range"),
                      drive->controller);
            return -1;
        }

        if (drive->bus != 0) {
            VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                      _("SCSI bus index %d out of [0] range"),
                      drive->bus);
            return -1;
        }

        if (drive->unit > 15 || drive->unit == 7) {
            VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                      _("SCSI unit index %d out of [0..6,8..15] range"),
                      drive->unit);
            return -1;
        }
    } else if (disk->bus == VIR_DOMAIN_DISK_BUS_IDE) {
        if (drive->controller != 0) {
            VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                      _("IDE controller index %d out of [0] range"),
                      drive->controller);
            return -1;
        }

        if (drive->bus > 1) {
            VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                      _("IDE bus index %d out of [0..1] range"),
                      drive->bus);
            return -1;
        }

        if (drive->unit > 1) {
            VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                      _("IDE unit index %d out of [0..1] range"),
                      drive->unit);
            return -1;
        }
    } else if (disk->bus == VIR_DOMAIN_DISK_BUS_FDC) {
        if (drive->controller != 0) {
            VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                      _("FDC controller index %d out of [0] range"),
                      drive->controller);
            return -1;
        }

        if (drive->bus != 0) {
            VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                      _("FDC bus index %d out of [0] range"),
                      drive->bus);
            return -1;
        }

        if (drive->unit > 1) {
            VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                      _("FDC unit index %d out of [0..1] range"),
                      drive->unit);
            return -1;
        }
    } else {
        VMX_ERROR(VIR_ERR_CONFIG_UNSUPPORTED,
                  _("Unsupported bus type '%s'"),
                  virDomainDiskBusTypeToString(disk->bus));
        return -1;
    }

    return 0;
}



static int
virVMXHandleLegacySCSIDiskDriverName(virDomainDefPtr def,
                                     virDomainDiskDefPtr disk)
{
    char *tmp;
    int model, i;
    virDomainControllerDefPtr controller = NULL;

    if (disk->bus != VIR_DOMAIN_DISK_BUS_SCSI || disk->driverName == NULL) {
        return 0;
    }

    tmp = disk->driverName;

    for (; *tmp != '\0'; ++tmp) {
        *tmp = c_tolower(*tmp);
    }

    model = virDomainControllerModelTypeFromString(disk->driverName);

    if (model < 0) {
        VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                  _("Unknown driver name '%s'"), disk->driverName);
        return -1;
    }

    for (i = 0; i < def->ncontrollers; ++i) {
        if (def->controllers[i]->idx == disk->info.addr.drive.controller) {
            controller = def->controllers[i];
            break;
        }
    }

    if (controller == NULL) {
        VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                  _("Missing SCSI controller for index %d"),
                  disk->info.addr.drive.controller);
        return -1;
    }

    if (controller->model == -1) {
        controller->model = model;
    } else if (controller->model != model) {
        VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                  _("Inconsistent SCSI controller model ('%s' is not '%s') "
                    "for SCSI controller index %d"), disk->driverName,
                  virDomainControllerModelTypeToString(controller->model),
                  controller->idx);
        return -1;
    }

    return 0;
}



static int
virVMXGatherSCSIControllers(virVMXContext *ctx, virDomainDefPtr def,
                            int virtualDev[4], bool present[4])
{
    int result = -1;
    int i, k;
    virDomainDiskDefPtr disk;
    virDomainControllerDefPtr controller;
    bool controllerHasDisksAttached;
    int count = 0;
    int *autodetectedModels;

    if (VIR_ALLOC_N(autodetectedModels, def->ndisks) < 0) {
        virReportOOMError();
        return -1;
    }

    for (i = 0; i < def->ncontrollers; ++i) {
        controller = def->controllers[i];

        if (controller->type != VIR_DOMAIN_CONTROLLER_TYPE_SCSI) {
            /* skip non-SCSI controllers */
            continue;
        }

        controllerHasDisksAttached = false;

        for (k = 0; k < def->ndisks; ++k) {
            disk = def->disks[k];

            if (disk->bus == VIR_DOMAIN_DISK_BUS_SCSI &&
                disk->info.addr.drive.controller == controller->idx) {
                controllerHasDisksAttached = true;
                break;
            }
        }

        if (! controllerHasDisksAttached) {
            /* skip SCSI controllers without attached disks */
            continue;
        }

        if (controller->model == VIR_DOMAIN_CONTROLLER_MODEL_AUTO &&
            ctx->autodetectSCSIControllerModel != NULL) {
            count = 0;

            /* try to autodetect the SCSI controller model by collecting
             * SCSI controller model of all disks attached to this controller */
            for (k = 0; k < def->ndisks; ++k) {
                disk = def->disks[k];

                if (disk->bus == VIR_DOMAIN_DISK_BUS_SCSI &&
                    disk->info.addr.drive.controller == controller->idx) {
                    if (ctx->autodetectSCSIControllerModel
                               (disk, &autodetectedModels[count],
                                ctx->opaque) < 0) {
                        goto cleanup;
                    }

                    ++count;
                }
            }

            /* autodetection fails when the disks attached to one controller
             * have inconsistent SCSI controller models */
            for (k = 0; k < count; ++k) {
                if (autodetectedModels[k] != autodetectedModels[0]) {
                    VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                              _("Disks on SCSI controller %d have inconsistent "
                                "controller models, cannot autodetect model"),
                              controller->idx);
                    goto cleanup;
                }
            }

            controller->model = autodetectedModels[0];
        }

        if (controller->model != -1 &&
            controller->model != VIR_DOMAIN_CONTROLLER_MODEL_BUSLOGIC &&
            controller->model != VIR_DOMAIN_CONTROLLER_MODEL_LSILOGIC &&
            controller->model != VIR_DOMAIN_CONTROLLER_MODEL_LSISAS1068 &&
            controller->model != VIR_DOMAIN_CONTROLLER_MODEL_VMPVSCSI) {
            VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                      _("Expecting domain XML attribute 'model' of entry "
                        "'controller' to be 'buslogic' or 'lsilogic' or "
                        "'lsisas1068' or 'vmpvscsi' but found '%s'"),
                      virDomainControllerModelTypeToString(controller->model));
            goto cleanup;
        }

        present[controller->idx] = true;
        virtualDev[controller->idx] = controller->model;
    }

    result = 0;

  cleanup:
    VIR_FREE(autodetectedModels);

    return result;
}



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * VMX -> Domain XML
 */

virDomainDefPtr
virVMXParseConfig(virVMXContext *ctx, virCapsPtr caps, const char *vmx)
{
    bool success = false;
    virConfPtr conf = NULL;
    char *encoding = NULL;
    char *utf8;
    virDomainDefPtr def = NULL;
    long long config_version = 0;
    long long virtualHW_version = 0;
    long long memsize = 0;
    long long sched_mem_max = 0;
    long long sched_mem_minsize = 0;
    long long numvcpus = 0;
    char *sched_cpu_affinity = NULL;
    char *guestOS = NULL;
    bool smbios_reflecthost = false;
    int controller;
    int bus;
    int port;
    bool present;
    int scsi_virtualDev[4] = { -1, -1, -1, -1 };
    int unit;

    if (ctx->parseFileName == NULL) {
        VMX_ERROR(VIR_ERR_INTERNAL_ERROR, "%s",
                  _("virVMXContext has no parseFileName function set"));
        return NULL;
    }

    conf = virConfReadMem(vmx, strlen(vmx), VIR_CONF_FLAG_VMX_FORMAT);

    if (conf == NULL) {
        return NULL;
    }

    /* vmx:.encoding */
    if (virVMXGetConfigString(conf, ".encoding", &encoding, true) < 0) {
        goto cleanup;
    }

    if (encoding == NULL || STRCASEEQ(encoding, "UTF-8")) {
        /* nothing */
    } else {
        virConfFree(conf);
        conf = NULL;

        utf8 = virVMXConvertToUTF8(encoding, vmx);

        if (utf8 == NULL) {
            goto cleanup;
        }

        conf = virConfReadMem(utf8, strlen(utf8), VIR_CONF_FLAG_VMX_FORMAT);

        VIR_FREE(utf8);

        if (conf == NULL) {
            goto cleanup;
        }
    }

    /* Allocate domain def */
    if (VIR_ALLOC(def) < 0) {
        virReportOOMError();
        return NULL;
    }

    def->virtType = VIR_DOMAIN_VIRT_VMWARE;
    def->id = -1;

    /* vmx:config.version */
    if (virVMXGetConfigLong(conf, "config.version", &config_version, 0,
                            false) < 0) {
        goto cleanup;
    }

    if (config_version != 8) {
        VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                  _("Expecting VMX entry 'config.version' to be 8 but found "
                    "%lld"), config_version);
        goto cleanup;
    }

    /* vmx:virtualHW.version */
    if (virVMXGetConfigLong(conf, "virtualHW.version", &virtualHW_version, 0,
                            false) < 0) {
        goto cleanup;
    }

    if (virtualHW_version != 4 && virtualHW_version != 7) {
        VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                  _("Expecting VMX entry 'virtualHW.version' to be 4 or 7 "
                    "but found %lld"),
                  virtualHW_version);
        goto cleanup;
    }

    /* vmx:uuid.bios -> def:uuid */
    /* FIXME: Need to handle 'uuid.action = "create"' */
    if (virVMXGetConfigUUID(conf, "uuid.bios", def->uuid, true) < 0) {
        goto cleanup;
    }

    /* vmx:displayName -> def:name */
    if (virVMXGetConfigString(conf, "displayName", &def->name, true) < 0) {
        goto cleanup;
    }

    if (def->name != NULL) {
        if (virVMXUnescapeHexPercent(def->name) < 0 ||
            virVMXUnescapeHexPipe(def->name) < 0) {
            VMX_ERROR(VIR_ERR_INTERNAL_ERROR, "%s",
                      _("VMX entry 'name' contains invalid escape sequence"));
            goto cleanup;
        }
    }

    /* vmx:annotation -> def:description */
    if (virVMXGetConfigString(conf, "annotation", &def->description,
                              true) < 0) {
        goto cleanup;
    }

    if (def->description != NULL) {
        if (virVMXUnescapeHexPipe(def->description) < 0) {
            VMX_ERROR(VIR_ERR_INTERNAL_ERROR, "%s",
                      _("VMX entry 'annotation' contains invalid escape "
                        "sequence"));
            goto cleanup;
        }
    }

    /* vmx:memsize -> def:mem.max_balloon */
    if (virVMXGetConfigLong(conf, "memsize", &memsize, 32, true) < 0) {
        goto cleanup;
    }

    if (memsize <= 0 || memsize % 4 != 0) {
        VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                  _("Expecting VMX entry 'memsize' to be an unsigned "
                    "integer (multiple of 4) but found %lld"), memsize);
        goto cleanup;
    }

    def->mem.max_balloon = memsize * 1024; /* Scale from megabytes to kilobytes */

    /* vmx:sched.mem.max -> def:mem.cur_balloon */
    if (virVMXGetConfigLong(conf, "sched.mem.max", &sched_mem_max, memsize,
                            true) < 0) {
        goto cleanup;
    }

    if (sched_mem_max < 0) {
        sched_mem_max = memsize;
    }

    def->mem.cur_balloon = sched_mem_max * 1024; /* Scale from megabytes to kilobytes */

    if (def->mem.cur_balloon > def->mem.max_balloon) {
        def->mem.cur_balloon = def->mem.max_balloon;
    }

    /* vmx:sched.mem.minsize -> def:mem.min_guarantee */
    if (virVMXGetConfigLong(conf, "sched.mem.minsize", &sched_mem_minsize, 0,
                            true) < 0) {
        goto cleanup;
    }

    if (sched_mem_minsize < 0) {
        sched_mem_minsize = 0;
    }

    def->mem.min_guarantee = sched_mem_minsize * 1024; /* Scale from megabytes to kilobytes */

    if (def->mem.min_guarantee > def->mem.max_balloon) {
        def->mem.min_guarantee = def->mem.max_balloon;
    }

    /* vmx:numvcpus -> def:vcpus */
    if (virVMXGetConfigLong(conf, "numvcpus", &numvcpus, 1, true) < 0) {
        goto cleanup;
    }

    if (numvcpus <= 0 || (numvcpus % 2 != 0 && numvcpus != 1)) {
        VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                  _("Expecting VMX entry 'numvcpus' to be an unsigned "
                    "integer (1 or a multiple of 2) but found %lld"), numvcpus);
        goto cleanup;
    }

    def->maxvcpus = def->vcpus = numvcpus;

    /* vmx:sched.cpu.affinity -> def:cpumask */
    /* NOTE: maps to VirtualMachine:config.cpuAffinity.affinitySet */
    if (virVMXGetConfigString(conf, "sched.cpu.affinity", &sched_cpu_affinity,
                              true) < 0) {
        goto cleanup;
    }

    if (sched_cpu_affinity != NULL && STRNEQ(sched_cpu_affinity, "all")) {
        const char *current = sched_cpu_affinity;
        int number, count = 0;

        def->cpumasklen = 0;

        if (VIR_ALLOC_N(def->cpumask, VIR_DOMAIN_CPUMASK_LEN) < 0) {
            virReportOOMError();
            goto cleanup;
        }

        while (*current != '\0') {
            virSkipSpaces(&current);

            number = virParseNumber(&current);

            if (number < 0) {
                VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                          _("Expecting VMX entry 'sched.cpu.affinity' to be "
                            "a comma separated list of unsigned integers but "
                            "found '%s'"), sched_cpu_affinity);
                goto cleanup;
            }

            if (number >= VIR_DOMAIN_CPUMASK_LEN) {
                VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                          _("VMX entry 'sched.cpu.affinity' contains a %d, "
                            "this value is too large"), number);
                goto cleanup;
            }

            if (number + 1 > def->cpumasklen) {
                def->cpumasklen = number + 1;
            }

            def->cpumask[number] = 1;
            ++count;

            virSkipSpaces(&current);

            if (*current == ',') {
                ++current;
            } else if (*current == '\0') {
                break;
            } else {
                VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                          _("Expecting VMX entry 'sched.cpu.affinity' to be "
                            "a comma separated list of unsigned integers but "
                            "found '%s'"), sched_cpu_affinity);
                goto cleanup;
            }

            virSkipSpaces(&current);
        }

        if (count < numvcpus) {
            VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                      _("Expecting VMX entry 'sched.cpu.affinity' to contain "
                        "at least as many values as 'numvcpus' (%lld) but "
                        "found only %d value(s)"), numvcpus, count);
            goto cleanup;
        }
    }

    /* def:lifecycle */
    def->onReboot = VIR_DOMAIN_LIFECYCLE_RESTART;
    def->onPoweroff = VIR_DOMAIN_LIFECYCLE_DESTROY;
    def->onCrash = VIR_DOMAIN_LIFECYCLE_DESTROY;

    /* def:os */
    def->os.type = strdup("hvm");

    if (def->os.type == NULL) {
        virReportOOMError();
        goto cleanup;
    }

    /* vmx:guestOS -> def:os.arch */
    if (virVMXGetConfigString(conf, "guestOS", &guestOS, true) < 0) {
        goto cleanup;
    }

    if (guestOS != NULL && virFileHasSuffix(guestOS, "-64")) {
        def->os.arch = strdup("x86_64");
    } else {
        def->os.arch = strdup("i686");
    }

    if (def->os.arch == NULL) {
        virReportOOMError();
        goto cleanup;
    }

    /* vmx:smbios.reflecthost -> def:os.smbios_mode */
    if (virVMXGetConfigBoolean(conf, "smbios.reflecthost",
                               &smbios_reflecthost, false, true) < 0) {
        goto cleanup;
    }

    if (smbios_reflecthost) {
        def->os.smbios_mode = VIR_DOMAIN_SMBIOS_HOST;
    }

    /* def:features */
    /* FIXME */

    /* def:clock */
    /* FIXME */

    /* def:graphics */
    if (VIR_ALLOC_N(def->graphics, 1) < 0) {
        virReportOOMError();
        goto cleanup;
    }

    def->ngraphics = 0;

    if (virVMXParseVNC(conf, &def->graphics[def->ngraphics]) < 0) {
        goto cleanup;
    }

    if (def->graphics[def->ngraphics] != NULL) {
        ++def->ngraphics;
    }

    /* def:disks: 4 * 15 scsi + 2 * 2 ide + 2 floppy = 66 */
    if (VIR_ALLOC_N(def->disks, 66) < 0) {
        virReportOOMError();
        goto cleanup;
    }

    def->ndisks = 0;

    /* def:disks (scsi) */
    for (controller = 0; controller < 4; ++controller) {
        if (virVMXParseSCSIController(conf, controller, &present,
                                      &scsi_virtualDev[controller]) < 0) {
            goto cleanup;
        }

        if (! present) {
            continue;
        }

        for (unit = 0; unit < 16; ++unit) {
            if (unit == 7) {
                /*
                 * SCSI unit 7 is assigned to the SCSI controller and cannot be
                 * used for disk devices.
                 */
                continue;
            }

            if (virVMXParseDisk(ctx, caps, conf, VIR_DOMAIN_DISK_DEVICE_DISK,
                                VIR_DOMAIN_DISK_BUS_SCSI, controller, unit,
                                &def->disks[def->ndisks]) < 0) {
                goto cleanup;
            }

            if (def->disks[def->ndisks] != NULL) {
                ++def->ndisks;
                continue;
            }

            if (virVMXParseDisk(ctx, caps, conf, VIR_DOMAIN_DISK_DEVICE_CDROM,
                                 VIR_DOMAIN_DISK_BUS_SCSI, controller, unit,
                                 &def->disks[def->ndisks]) < 0) {
                goto cleanup;
            }

            if (def->disks[def->ndisks] != NULL) {
                ++def->ndisks;
            }
        }
    }

    /* def:disks (ide) */
    for (bus = 0; bus < 2; ++bus) {
        for (unit = 0; unit < 2; ++unit) {
            if (virVMXParseDisk(ctx, caps, conf, VIR_DOMAIN_DISK_DEVICE_DISK,
                                VIR_DOMAIN_DISK_BUS_IDE, bus, unit,
                                &def->disks[def->ndisks]) < 0) {
                goto cleanup;
            }

            if (def->disks[def->ndisks] != NULL) {
                ++def->ndisks;
                continue;
            }

            if (virVMXParseDisk(ctx, caps, conf, VIR_DOMAIN_DISK_DEVICE_CDROM,
                                VIR_DOMAIN_DISK_BUS_IDE, bus, unit,
                                &def->disks[def->ndisks]) < 0) {
                goto cleanup;
            }

            if (def->disks[def->ndisks] != NULL) {
                ++def->ndisks;
            }
        }
    }

    /* def:disks (floppy) */
    for (unit = 0; unit < 2; ++unit) {
        if (virVMXParseDisk(ctx, caps, conf, VIR_DOMAIN_DISK_DEVICE_FLOPPY,
                            VIR_DOMAIN_DISK_BUS_FDC, 0, unit,
                            &def->disks[def->ndisks]) < 0) {
            goto cleanup;
        }

        if (def->disks[def->ndisks] != NULL) {
            ++def->ndisks;
        }
    }

    /* def:controllers */
    if (virDomainDefAddImplicitControllers(def) < 0) {
        VMX_ERROR(VIR_ERR_INTERNAL_ERROR, "%s", _("Could not add controllers"));
        goto cleanup;
    }

    for (controller = 0; controller < def->ncontrollers; ++controller) {
        if (def->controllers[controller]->type == VIR_DOMAIN_CONTROLLER_TYPE_SCSI) {
            if (def->controllers[controller]->idx < 0 ||
                def->controllers[controller]->idx > 3) {
                VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                          _("SCSI controller index %d out of [0..3] range"),
                          def->controllers[controller]->idx);
                goto cleanup;
            }

            def->controllers[controller]->model =
              scsi_virtualDev[def->controllers[controller]->idx];
        }
    }

    /* def:fss */
    /* FIXME */

    /* def:nets */
    if (VIR_ALLOC_N(def->nets, 4) < 0) {
        virReportOOMError();
        goto cleanup;
    }

    def->nnets = 0;

    for (controller = 0; controller < 4; ++controller) {
        if (virVMXParseEthernet(conf, controller,
                                &def->nets[def->nnets]) < 0) {
            goto cleanup;
        }

        if (def->nets[def->nnets] != NULL) {
            ++def->nnets;
        }
    }

    /* def:inputs */
    /* FIXME */

    /* def:videos */
    if (VIR_ALLOC_N(def->videos, 1) < 0) {
        virReportOOMError();
        goto cleanup;
    }

    def->nvideos = 0;

    if (virVMXParseSVGA(conf, &def->videos[def->nvideos]) < 0) {
        goto cleanup;
    }

    def->nvideos = 1;

    /* def:sounds */
    /* FIXME */

    /* def:hostdevs */
    /* FIXME */

    /* def:serials */
    if (VIR_ALLOC_N(def->serials, 4) < 0) {
        virReportOOMError();
        goto cleanup;
    }

    def->nserials = 0;

    for (port = 0; port < 4; ++port) {
        if (virVMXParseSerial(ctx, conf, port,
                              &def->serials[def->nserials]) < 0) {
            goto cleanup;
        }

        if (def->serials[def->nserials] != NULL) {
            ++def->nserials;
        }
    }

    /* def:parallels */
    if (VIR_ALLOC_N(def->parallels, 3) < 0) {
        virReportOOMError();
        goto cleanup;
    }

    def->nparallels = 0;

    for (port = 0; port < 3; ++port) {
        if (virVMXParseParallel(ctx, conf, port,
                                &def->parallels[def->nparallels]) < 0) {
            goto cleanup;
        }

        if (def->parallels[def->nparallels] != NULL) {
            ++def->nparallels;
        }
    }

    success = true;

  cleanup:
    if (! success) {
        virDomainDefFree(def);
        def = NULL;
    }

    virConfFree(conf);
    VIR_FREE(encoding);
    VIR_FREE(sched_cpu_affinity);
    VIR_FREE(guestOS);

    return def;
}



int
virVMXParseVNC(virConfPtr conf, virDomainGraphicsDefPtr *def)
{
    bool enabled = false;
    long long port = 0;

    if (def == NULL || *def != NULL) {
        VMX_ERROR(VIR_ERR_INTERNAL_ERROR, "%s", _("Invalid argument"));
        return -1;
    }

    if (virVMXGetConfigBoolean(conf, "RemoteDisplay.vnc.enabled", &enabled,
                               false, true) < 0) {
        return -1;
    }

    if (! enabled) {
        return 0;
    }

    if (VIR_ALLOC(*def) < 0) {
        virReportOOMError();
        goto failure;
    }

    (*def)->type = VIR_DOMAIN_GRAPHICS_TYPE_VNC;

    if (virVMXGetConfigLong(conf, "RemoteDisplay.vnc.port", &port, -1,
                            true) < 0 ||
        virVMXGetConfigString(conf, "RemoteDisplay.vnc.ip",
                              &(*def)->data.vnc.listenAddr, true) < 0 ||
        virVMXGetConfigString(conf, "RemoteDisplay.vnc.keymap",
                              &(*def)->data.vnc.keymap, true) < 0 ||
        virVMXGetConfigString(conf, "RemoteDisplay.vnc.password",
                              &(*def)->data.vnc.auth.passwd, true) < 0) {
        goto failure;
    }

    if (port < 0) {
        VIR_WARN0("VNC is enabled but VMX entry 'RemoteDisplay.vnc.port' "
                  "is missing, the VNC port is unknown");

        (*def)->data.vnc.port = 0;
        (*def)->data.vnc.autoport = 1;
    } else {
        if (port < 5900 || port > 5964) {
            VIR_WARN("VNC port %lld it out of [5900..5964] range", port);
        }

        (*def)->data.vnc.port = port;
        (*def)->data.vnc.autoport = 0;
    }

    return 0;

  failure:
    virDomainGraphicsDefFree(*def);
    *def = NULL;

    return -1;
}



int
virVMXParseSCSIController(virConfPtr conf, int controller, bool *present,
                          int *virtualDev)
{
    char present_name[32];
    char virtualDev_name[32];
    char *virtualDev_string = NULL;
    char *tmp;

    if (virtualDev == NULL || *virtualDev != -1) {
        VMX_ERROR(VIR_ERR_INTERNAL_ERROR, "%s", _("Invalid argument"));
        return -1;
    }

    if (controller < 0 || controller > 3) {
        VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                  _("SCSI controller index %d out of [0..3] range"),
                  controller);
        return -1;
    }

    snprintf(present_name, sizeof(present_name), "scsi%d.present", controller);
    snprintf(virtualDev_name, sizeof(virtualDev_name), "scsi%d.virtualDev",
             controller);

    if (virVMXGetConfigBoolean(conf, present_name, present, false, true) < 0) {
        goto failure;
    }

    if (! *present) {
        return 0;
    }

    if (virVMXGetConfigString(conf, virtualDev_name, &virtualDev_string,
                              true) < 0) {
        goto failure;
    }

    if (virtualDev_string != NULL) {
        tmp = virtualDev_string;

        for (; *tmp != '\0'; ++tmp) {
            *tmp = c_tolower(*tmp);
        }

        *virtualDev = virVMXSCSIControllerModelTypeFromString(virtualDev_string);

        if (*virtualDev == -1 ||
            (*virtualDev != VIR_DOMAIN_CONTROLLER_MODEL_BUSLOGIC &&
             *virtualDev != VIR_DOMAIN_CONTROLLER_MODEL_LSILOGIC &&
             *virtualDev != VIR_DOMAIN_CONTROLLER_MODEL_LSISAS1068 &&
             *virtualDev != VIR_DOMAIN_CONTROLLER_MODEL_VMPVSCSI)) {
            VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                      _("Expecting VMX entry '%s' to be 'buslogic' or 'lsilogic' "
                        "or 'lsisas1068' or 'pvscsi' but found '%s'"),
                       virtualDev_name, virtualDev_string);
            goto failure;
        }
    }

    return 0;

  failure:
    VIR_FREE(virtualDev_string);

    return -1;
}



int
virVMXParseDisk(virVMXContext *ctx, virCapsPtr caps, virConfPtr conf,
                int device, int busType, int controllerOrBus, int unit,
                virDomainDiskDefPtr *def)
{
    /*
     *          device = {VIR_DOMAIN_DISK_DEVICE_DISK, VIR_DOMAIN_DISK_DEVICE_CDROM}
     *         busType = VIR_DOMAIN_DISK_BUS_SCSI
     * controllerOrBus = [0..3] -> controller
     *            unit = [0..6,8..15]
     *
     *          device = {VIR_DOMAIN_DISK_DEVICE_DISK, VIR_DOMAIN_DISK_DEVICE_CDROM}
     *         busType = VIR_DOMAIN_DISK_BUS_IDE
     * controllerOrBus = [0..1] -> bus
     *            unit = [0..1]
     *
     *          device = VIR_DOMAIN_DISK_DEVICE_FLOPPY
     *         busType = VIR_DOMAIN_DISK_BUS_FDC
     * controllerOrBus = [0]
     *            unit = [0..1]
     */

    int result = -1;
    char *prefix = NULL;

    char present_name[32] = "";
    bool present = false;

    char startConnected_name[32] = "";
    bool startConnected = false;

    char deviceType_name[32] = "";
    char *deviceType = NULL;

    char clientDevice_name[32] = "";
    bool clientDevice = false;

    char fileType_name[32] = "";
    char *fileType = NULL;

    char fileName_name[32] = "";
    char *fileName = NULL;

    char writeThrough_name[32] = "";
    bool writeThrough = false;

    if (def == NULL || *def != NULL) {
        VMX_ERROR(VIR_ERR_INTERNAL_ERROR, "%s", _("Invalid argument"));
        return -1;
    }

    if (VIR_ALLOC(*def) < 0) {
        virReportOOMError();
        return -1;
    }

    (*def)->device = device;
    (*def)->bus = busType;

    /* def:dst, def:driverName */
    if (device == VIR_DOMAIN_DISK_DEVICE_DISK ||
        device == VIR_DOMAIN_DISK_DEVICE_CDROM) {
        if (busType == VIR_DOMAIN_DISK_BUS_SCSI) {
            if (controllerOrBus < 0 || controllerOrBus > 3) {
                VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                          _("SCSI controller index %d out of [0..3] range"),
                          controllerOrBus);
                goto cleanup;
            }

            if (unit < 0 || unit > 15 || unit == 7) {
                VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                          _("SCSI unit index %d out of [0..6,8..15] range"),
                          unit);
                goto cleanup;
            }

            if (virAsprintf(&prefix, "scsi%d:%d", controllerOrBus, unit) < 0) {
                virReportOOMError();
                goto cleanup;
            }

            (*def)->dst =
               virIndexToDiskName
                 (controllerOrBus * 15 + (unit < 7 ? unit : unit - 1), "sd");

            if ((*def)->dst == NULL) {
                goto cleanup;
            }
        } else if (busType == VIR_DOMAIN_DISK_BUS_IDE) {
            if (controllerOrBus < 0 || controllerOrBus > 1) {
                VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                          _("IDE bus index %d out of [0..1] range"),
                          controllerOrBus);
                goto cleanup;
            }

            if (unit < 0 || unit > 1) {
                VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                          _("IDE unit index %d out of [0..1] range"), unit);
                goto cleanup;
            }

            if (virAsprintf(&prefix, "ide%d:%d", controllerOrBus, unit) < 0) {
                virReportOOMError();
                goto cleanup;
            }

            (*def)->dst = virIndexToDiskName(controllerOrBus * 2 + unit, "hd");

            if ((*def)->dst == NULL) {
                goto cleanup;
            }
        } else {
            VMX_ERROR(VIR_ERR_CONFIG_UNSUPPORTED,
                      _("Unsupported bus type '%s' for device type '%s'"),
                      virDomainDiskBusTypeToString(busType),
                      virDomainDiskDeviceTypeToString(device));
            goto cleanup;
        }
    } else if (device == VIR_DOMAIN_DISK_DEVICE_FLOPPY) {
        if (busType == VIR_DOMAIN_DISK_BUS_FDC) {
            if (controllerOrBus != 0) {
                VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                          _("FDC controller index %d out of [0] range"),
                          controllerOrBus);
                goto cleanup;
            }

            if (unit < 0 || unit > 1) {
                VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                          _("FDC unit index %d out of [0..1] range"),
                          unit);
                goto cleanup;
            }

            if (virAsprintf(&prefix, "floppy%d", unit) < 0) {
                virReportOOMError();
                goto cleanup;
            }

            (*def)->dst = virIndexToDiskName(unit, "fd");

            if ((*def)->dst == NULL) {
                goto cleanup;
            }
        } else {
            VMX_ERROR(VIR_ERR_CONFIG_UNSUPPORTED,
                      _("Unsupported bus type '%s' for device type '%s'"),
                      virDomainDiskBusTypeToString(busType),
                      virDomainDiskDeviceTypeToString(device));
            goto cleanup;
        }
    } else {
        VMX_ERROR(VIR_ERR_CONFIG_UNSUPPORTED,
                  _("Unsupported device type '%s'"),
                  virDomainDiskDeviceTypeToString(device));
        goto cleanup;
    }

    VMX_BUILD_NAME(present);
    VMX_BUILD_NAME(startConnected);
    VMX_BUILD_NAME(deviceType);
    VMX_BUILD_NAME(clientDevice);
    VMX_BUILD_NAME(fileType);
    VMX_BUILD_NAME(fileName);
    VMX_BUILD_NAME(writeThrough);

    /* vmx:present */
    if (virVMXGetConfigBoolean(conf, present_name, &present, false, true) < 0) {
        goto cleanup;
    }

    /* vmx:startConnected */
    if (virVMXGetConfigBoolean(conf, startConnected_name, &startConnected,
                               true, true) < 0) {
        goto cleanup;
    }

    /* FIXME: Need to distiguish between active and inactive domains here */
    if (! present/* && ! startConnected*/) {
        goto ignore;
    }

    /* vmx:deviceType -> def:type */
    if (virVMXGetConfigString(conf, deviceType_name, &deviceType, true) < 0) {
        goto cleanup;
    }

    /* vmx:clientDevice */
    if (virVMXGetConfigBoolean(conf, clientDevice_name, &clientDevice, false,
                               true) < 0) {
        goto cleanup;
    }

    if (clientDevice) {
        /*
         * Just ignore devices in client mode, because I have no clue how to
         * handle them (e.g. assign an image) without the VI Client GUI.
         */
        goto ignore;
    }

    /* vmx:fileType -> def:type */
    if (virVMXGetConfigString(conf, fileType_name, &fileType, true) < 0) {
        goto cleanup;
    }

    /* vmx:fileName -> def:src, def:type */
    if (virVMXGetConfigString(conf, fileName_name, &fileName, false) < 0) {
        goto cleanup;
    }

    /* vmx:writeThrough -> def:cachemode */
    if (virVMXGetConfigBoolean(conf, writeThrough_name, &writeThrough, false,
                               true) < 0) {
        goto cleanup;
    }

    /* Setup virDomainDiskDef */
    if (device == VIR_DOMAIN_DISK_DEVICE_DISK) {
        if (virFileHasSuffix(fileName, ".vmdk")) {
            if (deviceType != NULL) {
                if (busType == VIR_DOMAIN_DISK_BUS_SCSI &&
                    STRCASENEQ(deviceType, "scsi-hardDisk") &&
                    STRCASENEQ(deviceType, "disk")) {
                    VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                              _("Expecting VMX entry '%s' to be 'scsi-hardDisk' "
                                "or 'disk' but found '%s'"), deviceType_name,
                              deviceType);
                    goto cleanup;
                } else if (busType == VIR_DOMAIN_DISK_BUS_IDE &&
                           STRCASENEQ(deviceType, "ata-hardDisk") &&
                           STRCASENEQ(deviceType, "disk")) {
                    VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                              _("Expecting VMX entry '%s' to be 'ata-hardDisk' "
                                "or 'disk' but found '%s'"), deviceType_name,
                              deviceType);
                    goto cleanup;
                }
            }

            (*def)->type = VIR_DOMAIN_DISK_TYPE_FILE;
            (*def)->src = ctx->parseFileName(fileName, ctx->opaque);
            (*def)->cachemode = writeThrough ? VIR_DOMAIN_DISK_CACHE_WRITETHRU
                                             : VIR_DOMAIN_DISK_CACHE_DEFAULT;

            if ((*def)->src == NULL) {
                goto cleanup;
            }
        } else if (virFileHasSuffix(fileName, ".iso") ||
                   STREQ(deviceType, "atapi-cdrom")) {
            /*
             * This function was called in order to parse a harddisk device,
             * but .iso files and 'atapi-cdrom' devices are for CDROM devices
             * only. Just ignore it, another call to this function to parse a
             * CDROM device may handle it.
             */
            goto ignore;
        } else {
            VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                      _("Invalid or not yet handled value '%s' for VMX entry "
                        "'%s'"), fileName, fileName_name);
            goto cleanup;
        }
    } else if (device == VIR_DOMAIN_DISK_DEVICE_CDROM) {
        if (virFileHasSuffix(fileName, ".iso")) {
            if (deviceType != NULL) {
                if (STRCASENEQ(deviceType, "cdrom-image")) {
                    VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                              _("Expecting VMX entry '%s' to be 'cdrom-image' "
                                "but found '%s'"), deviceType_name, deviceType);
                    goto cleanup;
                }
            }

            (*def)->type = VIR_DOMAIN_DISK_TYPE_FILE;
            (*def)->src = ctx->parseFileName(fileName, ctx->opaque);

            if ((*def)->src == NULL) {
                goto cleanup;
            }
        } else if (virFileHasSuffix(fileName, ".vmdk")) {
            /*
             * This function was called in order to parse a CDROM device, but
             * .vmdk files are for harddisk devices only. Just ignore it,
             * another call to this function to parse a harddisk device may
             * handle it.
             */
            goto ignore;
        } else if (STREQ(deviceType, "atapi-cdrom")) {
            (*def)->type = VIR_DOMAIN_DISK_TYPE_BLOCK;
            (*def)->src = fileName;

            fileName = NULL;
        } else {
            VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                      _("Invalid or not yet handled value '%s' for VMX entry "
                        "'%s'"), fileName, fileName_name);
            goto cleanup;
        }
    } else if (device == VIR_DOMAIN_DISK_DEVICE_FLOPPY) {
        if (virFileHasSuffix(fileName, ".flp")) {
            if (fileType != NULL) {
                if (STRCASENEQ(fileType, "file")) {
                    VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                              _("Expecting VMX entry '%s' to be 'file' but "
                                "found '%s'"), fileType_name, fileType);
                    goto cleanup;
                }
            }

            (*def)->type = VIR_DOMAIN_DISK_TYPE_FILE;
            (*def)->src = ctx->parseFileName(fileName, ctx->opaque);

            if ((*def)->src == NULL) {
                goto cleanup;
            }
        } else if (fileType != NULL && STREQ(fileType, "device")) {
            (*def)->type = VIR_DOMAIN_DISK_TYPE_BLOCK;
            (*def)->src = fileName;

            fileName = NULL;
        } else {
            VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                      _("Invalid or not yet handled value '%s' for VMX entry "
                        "'%s'"), fileName, fileName_name);
            goto cleanup;
        }
    } else {
        VMX_ERROR(VIR_ERR_CONFIG_UNSUPPORTED, _("Unsupported device type '%s'"),
                  virDomainDiskDeviceTypeToString(device));
        goto cleanup;
    }

    if (virDomainDiskDefAssignAddress(caps, *def) < 0) {
        VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                  _("Could not assign address to disk '%s'"), (*def)->src);
        goto cleanup;
    }

    result = 0;

  cleanup:
    if (result < 0) {
        virDomainDiskDefFree(*def);
        *def = NULL;
    }

    VIR_FREE(prefix);
    VIR_FREE(deviceType);
    VIR_FREE(fileType);
    VIR_FREE(fileName);

    return result;

  ignore:
    virDomainDiskDefFree(*def);
    *def = NULL;

    result = 0;

    goto cleanup;
}



int
virVMXParseEthernet(virConfPtr conf, int controller, virDomainNetDefPtr *def)
{
    int result = -1;
    char prefix[48] = "";

    char present_name[48] = "";
    bool present = false;

    char startConnected_name[48] = "";
    bool startConnected = false;

    char connectionType_name[48] = "";
    char *connectionType = NULL;

    char addressType_name[48] = "";
    char *addressType = NULL;

    char generatedAddress_name[48] = "";
    char *generatedAddress = NULL;

    char address_name[48] = "";
    char *address = NULL;

    char virtualDev_name[48] = "";
    char *virtualDev = NULL;

    char features_name[48] = "";
    long long features = 0;

    char vnet_name[48] = "";
    char *vnet = NULL;

    char networkName_name[48] = "";
    char *networkName = NULL;

    if (def == NULL || *def != NULL) {
        VMX_ERROR(VIR_ERR_INTERNAL_ERROR, "%s", _("Invalid argument"));
        return -1;
    }

    if (controller < 0 || controller > 3) {
        VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                  _("Ethernet controller index %d out of [0..3] range"),
                  controller);
        return -1;
    }

    if (VIR_ALLOC(*def) < 0) {
        virReportOOMError();
        return -1;
    }

    snprintf(prefix, sizeof(prefix), "ethernet%d", controller);

    VMX_BUILD_NAME(present);
    VMX_BUILD_NAME(startConnected);
    VMX_BUILD_NAME(connectionType);
    VMX_BUILD_NAME(addressType);
    VMX_BUILD_NAME(generatedAddress);
    VMX_BUILD_NAME(address);
    VMX_BUILD_NAME(virtualDev);
    VMX_BUILD_NAME(features);
    VMX_BUILD_NAME(networkName);
    VMX_BUILD_NAME(vnet);

    /* vmx:present */
    if (virVMXGetConfigBoolean(conf, present_name, &present, false, true) < 0) {
        goto cleanup;
    }

    /* vmx:startConnected */
    if (virVMXGetConfigBoolean(conf, startConnected_name, &startConnected,
                               true, true) < 0) {
        goto cleanup;
    }

    /* FIXME: Need to distiguish between active and inactive domains here */
    if (! present/* && ! startConnected*/) {
        goto ignore;
    }

    /* vmx:connectionType -> def:type */
    if (virVMXGetConfigString(conf, connectionType_name, &connectionType,
                              true) < 0) {
        goto cleanup;
    }

    /* vmx:addressType, vmx:generatedAddress, vmx:address -> def:mac */
    if (virVMXGetConfigString(conf, addressType_name, &addressType,
                              true) < 0 ||
        virVMXGetConfigString(conf, generatedAddress_name, &generatedAddress,
                              true) < 0 ||
        virVMXGetConfigString(conf, address_name, &address, true) < 0) {
        goto cleanup;
    }

    if (addressType == NULL || STRCASEEQ(addressType, "generated") ||
        STRCASEEQ(addressType, "vpx")) {
        if (generatedAddress != NULL) {
            if (virParseMacAddr(generatedAddress, (*def)->mac) < 0) {
                VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                          _("Expecting VMX entry '%s' to be MAC address but "
                            "found '%s'"), generatedAddress_name,
                          generatedAddress);
                goto cleanup;
            }
        }
    } else if (STRCASEEQ(addressType, "static")) {
        if (address != NULL) {
            if (virParseMacAddr(address, (*def)->mac) < 0) {
                VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                          _("Expecting VMX entry '%s' to be MAC address but "
                            "found '%s'"), address_name, address);
                goto cleanup;
            }
        }
    } else {
        VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                  _("Expecting VMX entry '%s' to be 'generated' or 'static' or "
                    "'vpx' but found '%s'"), addressType_name, addressType);
        goto cleanup;
    }

    /* vmx:virtualDev, vmx:features -> def:model */
    if (virVMXGetConfigString(conf, virtualDev_name, &virtualDev, true) < 0 ||
        virVMXGetConfigLong(conf, features_name, &features, 0, true) < 0) {
        goto cleanup;
    }

    if (virtualDev != NULL) {
        if (STRCASENEQ(virtualDev, "vlance") &&
            STRCASENEQ(virtualDev, "vmxnet") &&
            STRCASENEQ(virtualDev, "vmxnet3") &&
            STRCASENEQ(virtualDev, "e1000")) {
            VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                      _("Expecting VMX entry '%s' to be 'vlance' or 'vmxnet' or "
                        "'vmxnet3' or 'e1000' but found '%s'"), virtualDev_name,
                      virtualDev);
            goto cleanup;
        }

        if (STRCASEEQ(virtualDev, "vmxnet") && features == 15) {
            VIR_FREE(virtualDev);

            virtualDev = strdup("vmxnet2");

            if (virtualDev == NULL) {
                virReportOOMError();
                goto cleanup;
            }
        }
    }

    /* vmx:networkName -> def:data.bridge.brname */
    if ((connectionType == NULL ||
         STRCASEEQ(connectionType, "bridged") ||
         STRCASEEQ(connectionType, "custom")) &&
        virVMXGetConfigString(conf, networkName_name, &networkName,
                              false) < 0) {
        goto cleanup;
    }

    /* vmx:vnet -> def:data.ifname */
    if (connectionType != NULL && STRCASEEQ(connectionType, "custom") &&
        virVMXGetConfigString(conf, vnet_name, &vnet, false) < 0) {
        goto cleanup;
    }

    /* Setup virDomainNetDef */
    if (connectionType == NULL || STRCASEEQ(connectionType, "bridged")) {
        (*def)->type = VIR_DOMAIN_NET_TYPE_BRIDGE;
        (*def)->model = virtualDev;
        (*def)->data.bridge.brname = networkName;

        virtualDev = NULL;
        networkName = NULL;
    } else if (STRCASEEQ(connectionType, "hostonly")) {
        /* FIXME */
        VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                  _("No yet handled value '%s' for VMX entry '%s'"),
                  connectionType, connectionType_name);
        goto cleanup;
    } else if (STRCASEEQ(connectionType, "nat")) {
        /* FIXME */
        VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                  _("No yet handled value '%s' for VMX entry '%s'"),
                  connectionType, connectionType_name);
        goto cleanup;
    } else if (STRCASEEQ(connectionType, "custom")) {
        (*def)->type = VIR_DOMAIN_NET_TYPE_BRIDGE;
        (*def)->model = virtualDev;
        (*def)->data.bridge.brname = networkName;
        (*def)->ifname = vnet;

        virtualDev = NULL;
        networkName = NULL;
        vnet = NULL;
    } else {
        VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                  _("Invalid value '%s' for VMX entry '%s'"), connectionType,
                  connectionType_name);
        goto cleanup;
    }

    result = 0;

  cleanup:
    if (result < 0) {
        virDomainNetDefFree(*def);
        *def = NULL;
    }

    VIR_FREE(connectionType);
    VIR_FREE(addressType);
    VIR_FREE(generatedAddress);
    VIR_FREE(address);
    VIR_FREE(virtualDev);
    VIR_FREE(vnet);

    return result;

  ignore:
    virDomainNetDefFree(*def);
    *def = NULL;

    result = 0;

    goto cleanup;
}



int
virVMXParseSerial(virVMXContext *ctx, virConfPtr conf, int port,
                  virDomainChrDefPtr *def)
{
    int result = -1;
    char prefix[48] = "";

    char present_name[48] = "";
    bool present = false;

    char startConnected_name[48] = "";
    bool startConnected = false;

    char fileType_name[48] = "";
    char *fileType = NULL;

    char fileName_name[48] = "";
    char *fileName = NULL;

    char network_endPoint_name[48] = "";
    char *network_endPoint = NULL;

    xmlURIPtr parsedUri = NULL;

    if (def == NULL || *def != NULL) {
        VMX_ERROR(VIR_ERR_INTERNAL_ERROR, "%s", _("Invalid argument"));
        return -1;
    }

    if (port < 0 || port > 3) {
        VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                  _("Serial port index %d out of [0..3] range"), port);
        return -1;
    }

    if (VIR_ALLOC(*def) < 0) {
        virReportOOMError();
        return -1;
    }

    (*def)->deviceType = VIR_DOMAIN_CHR_DEVICE_TYPE_SERIAL;

    snprintf(prefix, sizeof(prefix), "serial%d", port);

    VMX_BUILD_NAME(present);
    VMX_BUILD_NAME(startConnected);
    VMX_BUILD_NAME(fileType);
    VMX_BUILD_NAME(fileName);
    VMX_BUILD_NAME_EXTRA(network_endPoint, "network.endPoint");

    /* vmx:present */
    if (virVMXGetConfigBoolean(conf, present_name, &present, false, true) < 0) {
        goto cleanup;
    }

    /* vmx:startConnected */
    if (virVMXGetConfigBoolean(conf, startConnected_name, &startConnected,
                               true, true) < 0) {
        goto cleanup;
    }

    /* FIXME: Need to distiguish between active and inactive domains here */
    if (! present/* && ! startConnected*/) {
        goto ignore;
    }

    /* vmx:fileType -> def:type */
    if (virVMXGetConfigString(conf, fileType_name, &fileType, false) < 0) {
        goto cleanup;
    }

    /* vmx:fileName -> def:data.file.path */
    if (virVMXGetConfigString(conf, fileName_name, &fileName, false) < 0) {
        goto cleanup;
    }

    /* vmx:network.endPoint -> def:data.tcp.listen */
    if (virVMXGetConfigString(conf, network_endPoint_name, &network_endPoint,
                              true) < 0) {
        goto cleanup;
    }

    /* Setup virDomainChrDef */
    if (STRCASEEQ(fileType, "device")) {
        (*def)->target.port = port;
        (*def)->source.type = VIR_DOMAIN_CHR_TYPE_DEV;
        (*def)->source.data.file.path = fileName;

        fileName = NULL;
    } else if (STRCASEEQ(fileType, "file")) {
        (*def)->target.port = port;
        (*def)->source.type = VIR_DOMAIN_CHR_TYPE_FILE;
        (*def)->source.data.file.path = ctx->parseFileName(fileName,
                                                           ctx->opaque);

        if ((*def)->source.data.file.path == NULL) {
            goto cleanup;
        }
    } else if (STRCASEEQ(fileType, "pipe")) {
        /*
         * FIXME: Differences between client/server and VM/application pipes
         *        not representable in domain XML form
         */
        (*def)->target.port = port;
        (*def)->source.type = VIR_DOMAIN_CHR_TYPE_PIPE;
        (*def)->source.data.file.path = fileName;

        fileName = NULL;
    } else if (STRCASEEQ(fileType, "network")) {
        (*def)->target.port = port;
        (*def)->source.type = VIR_DOMAIN_CHR_TYPE_TCP;

        parsedUri = xmlParseURI(fileName);

        if (parsedUri == NULL) {
            virReportOOMError();
            goto cleanup;
        }

        if (parsedUri->port == 0) {
            VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                      _("VMX entry '%s' doesn't contain a port part"),
                      fileName_name);
            goto cleanup;
        }

        (*def)->source.data.tcp.host = strdup(parsedUri->server);

        if ((*def)->source.data.tcp.host == NULL) {
            virReportOOMError();
            goto cleanup;
        }

        if (virAsprintf(&(*def)->source.data.tcp.service, "%d",
                        parsedUri->port) < 0) {
            virReportOOMError();
            goto cleanup;
        }

        /* See vSphere API documentation about VirtualSerialPortURIBackingInfo */
        if (parsedUri->scheme == NULL ||
            STRCASEEQ(parsedUri->scheme, "tcp") ||
            STRCASEEQ(parsedUri->scheme, "tcp4") ||
            STRCASEEQ(parsedUri->scheme, "tcp6")) {
            (*def)->source.data.tcp.protocol = VIR_DOMAIN_CHR_TCP_PROTOCOL_RAW;
        } else if (STRCASEEQ(parsedUri->scheme, "telnet")) {
            (*def)->source.data.tcp.protocol
                = VIR_DOMAIN_CHR_TCP_PROTOCOL_TELNET;
        } else if (STRCASEEQ(parsedUri->scheme, "telnets")) {
            (*def)->source.data.tcp.protocol
                = VIR_DOMAIN_CHR_TCP_PROTOCOL_TELNETS;
        } else if (STRCASEEQ(parsedUri->scheme, "ssl") ||
                   STRCASEEQ(parsedUri->scheme, "tcp+ssl") ||
                   STRCASEEQ(parsedUri->scheme, "tcp4+ssl") ||
                   STRCASEEQ(parsedUri->scheme, "tcp6+ssl")) {
            (*def)->source.data.tcp.protocol = VIR_DOMAIN_CHR_TCP_PROTOCOL_TLS;
        } else {
            VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                      _("VMX entry '%s' contains unsupported scheme '%s'"),
                      fileName_name, parsedUri->scheme);
            goto cleanup;
        }

        if (network_endPoint == NULL || STRCASEEQ(network_endPoint, "server")) {
            (*def)->source.data.tcp.listen = true;
        } else if (STRCASEEQ(network_endPoint, "client")) {
            (*def)->source.data.tcp.listen = false;
        } else {
            VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                      _("Expecting VMX entry '%s' to be 'server' or 'client' "
                        "but found '%s'"), network_endPoint_name, network_endPoint);
            goto cleanup;
        }
    } else {
        VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                  _("Expecting VMX entry '%s' to be 'device', 'file' or 'pipe' "
                    "or 'network' but found '%s'"), fileType_name, fileType);
        goto cleanup;
    }

    result = 0;

  cleanup:
    if (result < 0) {
        virDomainChrDefFree(*def);
        *def = NULL;
    }

    VIR_FREE(fileType);
    VIR_FREE(fileName);
    VIR_FREE(network_endPoint);
    xmlFreeURI(parsedUri);

    return result;

  ignore:
    virDomainChrDefFree(*def);
    *def = NULL;

    result = 0;

    goto cleanup;
}



int
virVMXParseParallel(virVMXContext *ctx, virConfPtr conf, int port,
                    virDomainChrDefPtr *def)
{
    int result = -1;
    char prefix[48] = "";

    char present_name[48] = "";
    bool present = false;

    char startConnected_name[48] = "";
    bool startConnected = false;

    char fileType_name[48] = "";
    char *fileType = NULL;

    char fileName_name[48] = "";
    char *fileName = NULL;

    if (def == NULL || *def != NULL) {
        VMX_ERROR(VIR_ERR_INTERNAL_ERROR, "%s", _("Invalid argument"));
        return -1;
    }

    if (port < 0 || port > 2) {
        VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                  _("Parallel port index %d out of [0..2] range"), port);
        return -1;
    }

    if (VIR_ALLOC(*def) < 0) {
        virReportOOMError();
        return -1;
    }

    (*def)->deviceType = VIR_DOMAIN_CHR_DEVICE_TYPE_PARALLEL;

    snprintf(prefix, sizeof(prefix), "parallel%d", port);

    VMX_BUILD_NAME(present);
    VMX_BUILD_NAME(startConnected);
    VMX_BUILD_NAME(fileType);
    VMX_BUILD_NAME(fileName);

    /* vmx:present */
    if (virVMXGetConfigBoolean(conf, present_name, &present, false, true) < 0) {
        goto cleanup;
    }

    /* vmx:startConnected */
    if (virVMXGetConfigBoolean(conf, startConnected_name, &startConnected,
                               true, true) < 0) {
        goto cleanup;
    }

    /* FIXME: Need to distiguish between active and inactive domains here */
    if (! present/* && ! startConnected*/) {
        goto ignore;
    }

    /* vmx:fileType -> def:type */
    if (virVMXGetConfigString(conf, fileType_name, &fileType, false) < 0) {
        goto cleanup;
    }

    /* vmx:fileName -> def:data.file.path */
    if (virVMXGetConfigString(conf, fileName_name, &fileName, false) < 0) {
        goto cleanup;
    }

    /* Setup virDomainChrDef */
    if (STRCASEEQ(fileType, "device")) {
        (*def)->target.port = port;
        (*def)->source.type = VIR_DOMAIN_CHR_TYPE_DEV;
        (*def)->source.data.file.path = fileName;

        fileName = NULL;
    } else if (STRCASEEQ(fileType, "file")) {
        (*def)->target.port = port;
        (*def)->source.type = VIR_DOMAIN_CHR_TYPE_FILE;
        (*def)->source.data.file.path = ctx->parseFileName(fileName,
                                                           ctx->opaque);

        if ((*def)->source.data.file.path == NULL) {
            goto cleanup;
        }
    } else {
        VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                  _("Expecting VMX entry '%s' to be 'device' or 'file' but "
                    "found '%s'"), fileType_name, fileType);
        goto cleanup;
    }

    result = 0;

  cleanup:
    if (result < 0) {
        virDomainChrDefFree(*def);
        *def = NULL;
    }

    VIR_FREE(fileType);
    VIR_FREE(fileName);

    return result;

  ignore:
    virDomainChrDefFree(*def);
    *def = NULL;

    result = 0;

    goto cleanup;
}



int
virVMXParseSVGA(virConfPtr conf, virDomainVideoDefPtr *def)
{
    int result = -1;
    long long svga_vramSize = 0;

    if (def == NULL || *def != NULL) {
        VMX_ERROR(VIR_ERR_INTERNAL_ERROR, "%s", _("Invalid argument"));
        return -1;
    }

    if (VIR_ALLOC(*def) < 0) {
        virReportOOMError();
        return -1;
    }

    (*def)->type = VIR_DOMAIN_VIDEO_TYPE_VMVGA;

    /* vmx:vramSize */
    if (virVMXGetConfigLong(conf, "svga.vramSize", &svga_vramSize,
                            4 * 1024 * 1024, true) < 0) {
        goto cleanup;
    }

    (*def)->vram = VIR_DIV_UP(svga_vramSize, 1024); /* Scale from bytes to kilobytes */

    result = 0;

  cleanup:
    if (result < 0) {
        virDomainVideoDefFree(*def);
        *def = NULL;
    }

    return result;
}



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Domain XML -> VMX
 */

char *
virVMXFormatConfig(virVMXContext *ctx, virCapsPtr caps, virDomainDefPtr def,
                   int virtualHW_version)
{
    char *vmx = NULL;
    int i;
    int sched_cpu_affinity_length;
    unsigned char zero[VIR_UUID_BUFLEN];
    virBuffer buffer = VIR_BUFFER_INITIALIZER;
    char *preliminaryDisplayName = NULL;
    char *displayName = NULL;
    char *annotation = NULL;
    unsigned long max_balloon;
    bool scsi_present[4] = { false, false, false, false };
    int scsi_virtualDev[4] = { -1, -1, -1, -1 };
    bool floppy_present[2] = { false, false };

    if (ctx->formatFileName == NULL) {
        VMX_ERROR(VIR_ERR_INTERNAL_ERROR, "%s",
                  _("virVMXContext has no formatFileName function set"));
        return NULL;
    }

    memset(zero, 0, VIR_UUID_BUFLEN);

    if (def->virtType != VIR_DOMAIN_VIRT_VMWARE) {
        VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                  _("Expecting virt type to be '%s' but found '%s'"),
                  virDomainVirtTypeToString(VIR_DOMAIN_VIRT_VMWARE),
                  virDomainVirtTypeToString(def->virtType));
        return NULL;
    }

    /* vmx:.encoding */
    virBufferAddLit(&buffer, ".encoding = \"UTF-8\"\n");

    /* vmx:config.version */
    virBufferAddLit(&buffer, "config.version = \"8\"\n");

    /* vmx:virtualHW.version */
    virBufferVSprintf(&buffer, "virtualHW.version = \"%d\"\n",
                      virtualHW_version);

    /* def:os.arch -> vmx:guestOS */
    if (def->os.arch == NULL || STRCASEEQ(def->os.arch, "i686")) {
        virBufferAddLit(&buffer, "guestOS = \"other\"\n");
    } else if (STRCASEEQ(def->os.arch, "x86_64")) {
        virBufferAddLit(&buffer, "guestOS = \"other-64\"\n");
    } else {
        VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                  _("Expecting domain XML attribute 'arch' of entry 'os/type' "
                    "to be 'i686' or 'x86_64' but found '%s'"), def->os.arch);
        goto cleanup;
    }

    /* def:os.smbios_mode -> vmx:smbios.reflecthost */
    if (def->os.smbios_mode == VIR_DOMAIN_SMBIOS_NONE ||
        def->os.smbios_mode == VIR_DOMAIN_SMBIOS_EMULATE) {
        /* nothing */
    } else if (def->os.smbios_mode == VIR_DOMAIN_SMBIOS_HOST) {
        virBufferAddLit(&buffer, "smbios.reflecthost = \"true\"\n");
    } else {
        VMX_ERROR(VIR_ERR_CONFIG_UNSUPPORTED,
                  _("Unsupported SMBIOS mode '%s'"),
                  virDomainSmbiosModeTypeToString(def->os.smbios_mode));
        goto cleanup;
    }

    /* def:uuid -> vmx:uuid.action, vmx:uuid.bios */
    if (memcmp(def->uuid, zero, VIR_UUID_BUFLEN) == 0) {
        virBufferAddLit(&buffer, "uuid.action = \"create\"\n");
    } else {
        virBufferVSprintf(&buffer, "uuid.bios = \"%02x %02x %02x %02x %02x %02x "
                          "%02x %02x-%02x %02x %02x %02x %02x %02x %02x %02x\"\n",
                          def->uuid[0], def->uuid[1], def->uuid[2], def->uuid[3],
                          def->uuid[4], def->uuid[5], def->uuid[6], def->uuid[7],
                          def->uuid[8], def->uuid[9], def->uuid[10], def->uuid[11],
                          def->uuid[12], def->uuid[13], def->uuid[14],
                          def->uuid[15]);
    }

    /* def:name -> vmx:displayName */
    preliminaryDisplayName = virVMXEscapeHexPipe(def->name);

    if (preliminaryDisplayName == NULL) {
        goto cleanup;
    }

    displayName = virVMXEscapeHexPercent(preliminaryDisplayName);

    if (displayName == NULL) {
        goto cleanup;
    }

    virBufferVSprintf(&buffer, "displayName = \"%s\"\n", displayName);

    /* def:description -> vmx:annotation */
    if (def->description != NULL) {
        annotation = virVMXEscapeHexPipe(def->description);

        virBufferVSprintf(&buffer, "annotation = \"%s\"\n", annotation);
    }

    /* def:mem.max_balloon -> vmx:memsize */
    /* max-memory must be a multiple of 4096 kilobyte */
    max_balloon = VIR_DIV_UP(def->mem.max_balloon, 4096) * 4096;

    virBufferVSprintf(&buffer, "memsize = \"%lu\"\n",
                      max_balloon / 1024); /* Scale from kilobytes to megabytes */

    /* def:mem.cur_balloon -> vmx:sched.mem.max */
    if (def->mem.cur_balloon < max_balloon) {
        virBufferVSprintf(&buffer, "sched.mem.max = \"%lu\"\n",
                          VIR_DIV_UP(def->mem.cur_balloon,
                                     1024)); /* Scale from kilobytes to megabytes */
    }

    /* def:mem.min_guarantee -> vmx:sched.mem.minsize */
    if (def->mem.min_guarantee > 0) {
        virBufferVSprintf(&buffer, "sched.mem.minsize = \"%lu\"\n",
                          VIR_DIV_UP(def->mem.min_guarantee,
                                     1024)); /* Scale from kilobytes to megabytes */
    }

    /* def:maxvcpus -> vmx:numvcpus */
    if (def->vcpus != def->maxvcpus) {
        VMX_ERROR(VIR_ERR_CONFIG_UNSUPPORTED, "%s",
                  _("No support for domain XML entry 'vcpu' attribute "
                    "'current'"));
        goto cleanup;
    }
    if (def->maxvcpus <= 0 || (def->maxvcpus % 2 != 0 && def->maxvcpus != 1)) {
        VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                  _("Expecting domain XML entry 'vcpu' to be an unsigned "
                    "integer (1 or a multiple of 2) but found %d"),
                  def->maxvcpus);
        goto cleanup;
    }

    virBufferVSprintf(&buffer, "numvcpus = \"%d\"\n", def->maxvcpus);

    /* def:cpumask -> vmx:sched.cpu.affinity */
    if (def->cpumasklen > 0) {
        virBufferAddLit(&buffer, "sched.cpu.affinity = \"");

        sched_cpu_affinity_length = 0;

        for (i = 0; i < def->cpumasklen; ++i) {
            if (def->cpumask[i]) {
                ++sched_cpu_affinity_length;
            }
        }

        if (sched_cpu_affinity_length < def->maxvcpus) {
            VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                      _("Expecting domain XML attribute 'cpuset' of entry "
                        "'vcpu' to contain at least %d CPU(s)"),
                      def->maxvcpus);
            goto cleanup;
        }

        for (i = 0; i < def->cpumasklen; ++i) {
            if (def->cpumask[i]) {
                virBufferVSprintf(&buffer, "%d", i);

                if (sched_cpu_affinity_length > 1) {
                    virBufferAddChar(&buffer, ',');
                }

                --sched_cpu_affinity_length;
            }
        }

        virBufferAddLit(&buffer, "\"\n");
    }

    /* def:graphics */
    for (i = 0; i < def->ngraphics; ++i) {
        switch (def->graphics[i]->type) {
          case VIR_DOMAIN_GRAPHICS_TYPE_VNC:
            if (virVMXFormatVNC(def->graphics[i], &buffer) < 0) {
                goto cleanup;
            }

            break;

          default:
            VMX_ERROR(VIR_ERR_CONFIG_UNSUPPORTED,
                      _("Unsupported graphics type '%s'"),
                      virDomainGraphicsTypeToString(def->graphics[i]->type));
            goto cleanup;
        }
    }

    /* def:disks */
    for (i = 0; i < def->ndisks; ++i) {
        if (virVMXVerifyDiskAddress(caps, def->disks[i]) < 0 ||
            virVMXHandleLegacySCSIDiskDriverName(def, def->disks[i]) < 0) {
            goto cleanup;
        }
    }

    if (virVMXGatherSCSIControllers(ctx, def, scsi_virtualDev,
                                    scsi_present) < 0) {
        goto cleanup;
    }

    for (i = 0; i < 4; ++i) {
        if (scsi_present[i]) {
            virBufferVSprintf(&buffer, "scsi%d.present = \"true\"\n", i);

            if (scsi_virtualDev[i] != -1) {
                virBufferVSprintf(&buffer, "scsi%d.virtualDev = \"%s\"\n", i,
                                  virVMXSCSIControllerModelTypeToString
                                    (scsi_virtualDev[i]));
            }
        }
    }

    for (i = 0; i < def->ndisks; ++i) {
        switch (def->disks[i]->device) {
          case VIR_DOMAIN_DISK_DEVICE_DISK:
            if (virVMXFormatHardDisk(ctx, def->disks[i], &buffer) < 0) {
                goto cleanup;
            }

            break;

          case VIR_DOMAIN_DISK_DEVICE_CDROM:
            if (virVMXFormatCDROM(ctx, def->disks[i], &buffer) < 0) {
                goto cleanup;
            }

            break;

          case VIR_DOMAIN_DISK_DEVICE_FLOPPY:
            if (virVMXFormatFloppy(ctx, def->disks[i], &buffer,
                                   floppy_present) < 0) {
                goto cleanup;
            }

            break;

          default:
            VMX_ERROR(VIR_ERR_CONFIG_UNSUPPORTED,
                      _("Unsupported disk device type '%s'"),
                      virDomainDiskDeviceTypeToString(def->disks[i]->device));
            goto cleanup;
        }
    }

    for (i = 0; i < 2; ++i) {
        /* floppy[0..1].present defaults to true, disable it explicitly */
        if (! floppy_present[i]) {
            virBufferVSprintf(&buffer, "floppy%d.present = \"false\"\n", i);
        }
    }

    /* def:fss */
    /* FIXME */

    /* def:nets */
    for (i = 0; i < def->nnets; ++i) {
        if (virVMXFormatEthernet(def->nets[i], i, &buffer) < 0) {
            goto cleanup;
        }
    }

    /* def:inputs */
    /* FIXME */

    /* def:sounds */
    /* FIXME */

    /* def:videos */
    if (def->nvideos > 0) {
        if (def->nvideos > 1) {
            VMX_ERROR(VIR_ERR_INTERNAL_ERROR, "%s",
                      _("No support for multiple video devices"));
            goto cleanup;
        }

        if (virVMXFormatSVGA(def->videos[0], &buffer) < 0) {
            goto cleanup;
        }
    }

    /* def:hostdevs */
    /* FIXME */

    /* def:serials */
    for (i = 0; i < def->nserials; ++i) {
        if (virVMXFormatSerial(ctx, def->serials[i], &buffer) < 0) {
            goto cleanup;
        }
    }

    /* def:parallels */
    for (i = 0; i < def->nparallels; ++i) {
        if (virVMXFormatParallel(ctx, def->parallels[i], &buffer) < 0) {
            goto cleanup;
        }
    }

    /* Get final VMX output */
    if (virBufferError(&buffer)) {
        virReportOOMError();
        goto cleanup;
    }

    vmx = virBufferContentAndReset(&buffer);

  cleanup:
    if (vmx == NULL) {
        virBufferFreeAndReset(&buffer);
    }

    VIR_FREE(preliminaryDisplayName);
    VIR_FREE(displayName);
    VIR_FREE(annotation);

    return vmx;
}



int
virVMXFormatVNC(virDomainGraphicsDefPtr def, virBufferPtr buffer)
{
    if (def->type != VIR_DOMAIN_GRAPHICS_TYPE_VNC) {
        VMX_ERROR(VIR_ERR_INTERNAL_ERROR, "%s", _("Invalid argument"));
        return -1;
    }

    virBufferVSprintf(buffer, "RemoteDisplay.vnc.enabled = \"true\"\n");

    if (def->data.vnc.autoport) {
        VIR_WARN0("VNC autoport is enabled, but the automatically assigned "
                  "VNC port cannot be read back");
    } else {
        if (def->data.vnc.port < 5900 || def->data.vnc.port > 5964) {
            VIR_WARN("VNC port %d it out of [5900..5964] range",
                     def->data.vnc.port);
        }

        virBufferVSprintf(buffer, "RemoteDisplay.vnc.port = \"%d\"\n",
                          def->data.vnc.port);
    }

    if (def->data.vnc.listenAddr != NULL) {
        virBufferVSprintf(buffer, "RemoteDisplay.vnc.ip = \"%s\"\n",
                          def->data.vnc.listenAddr);
    }

    if (def->data.vnc.keymap != NULL) {
        virBufferVSprintf(buffer, "RemoteDisplay.vnc.keymap = \"%s\"\n",
                          def->data.vnc.keymap);
    }

    if (def->data.vnc.auth.passwd != NULL) {
        virBufferVSprintf(buffer, "RemoteDisplay.vnc.password = \"%s\"\n",
                          def->data.vnc.auth.passwd);
    }

    return 0;
}



int
virVMXFormatHardDisk(virVMXContext *ctx, virDomainDiskDefPtr def,
                     virBufferPtr buffer)
{
    int controllerOrBus, unit;
    const char *busName = NULL;
    const char *entryPrefix = NULL;
    const char *deviceTypePrefix = NULL;
    char *fileName = NULL;

    if (def->device != VIR_DOMAIN_DISK_DEVICE_DISK) {
        VMX_ERROR(VIR_ERR_INTERNAL_ERROR, "%s", _("Invalid argument"));
        return -1;
    }

    if (def->bus == VIR_DOMAIN_DISK_BUS_SCSI) {
        busName = "SCSI";
        entryPrefix = "scsi";
        deviceTypePrefix = "scsi";

        if (virVMXSCSIDiskNameToControllerAndUnit(def->dst, &controllerOrBus,
                                                  &unit) < 0) {
            return -1;
        }
    } else if (def->bus == VIR_DOMAIN_DISK_BUS_IDE) {
        busName = "IDE";
        entryPrefix = "ide";
        deviceTypePrefix = "ata";

        if (virVMXIDEDiskNameToBusAndUnit(def->dst, &controllerOrBus,
                                           &unit) < 0) {
            return -1;
        }
    } else {
        VMX_ERROR(VIR_ERR_CONFIG_UNSUPPORTED,
                  _("Unsupported bus type '%s' for harddisk"),
                  virDomainDiskBusTypeToString(def->bus));
        return -1;
    }

    if (def->type != VIR_DOMAIN_DISK_TYPE_FILE) {
        VMX_ERROR(VIR_ERR_CONFIG_UNSUPPORTED,
                  _("%s harddisk '%s' has unsupported type '%s', expecting '%s'"),
                  busName, def->dst, virDomainDiskTypeToString(def->type),
                  virDomainDiskTypeToString(VIR_DOMAIN_DISK_TYPE_FILE));
        return -1;
    }

    virBufferVSprintf(buffer, "%s%d:%d.present = \"true\"\n",
                      entryPrefix, controllerOrBus, unit);
    virBufferVSprintf(buffer, "%s%d:%d.deviceType = \"%s-hardDisk\"\n",
                      entryPrefix, controllerOrBus, unit, deviceTypePrefix);

    if (def->src != NULL) {
        if (! virFileHasSuffix(def->src, ".vmdk")) {
            VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                      _("Image file for %s harddisk '%s' has unsupported suffix, "
                        "expecting '.vmdk'"), busName, def->dst);
            return -1;
        }

        fileName = ctx->formatFileName(def->src, ctx->opaque);

        if (fileName == NULL) {
            return -1;
        }

        virBufferVSprintf(buffer, "%s%d:%d.fileName = \"%s\"\n",
                          entryPrefix, controllerOrBus, unit, fileName);

        VIR_FREE(fileName);
    }

    if (def->bus == VIR_DOMAIN_DISK_BUS_SCSI) {
        if (def->cachemode == VIR_DOMAIN_DISK_CACHE_WRITETHRU) {
            virBufferVSprintf(buffer, "%s%d:%d.writeThrough = \"true\"\n",
                              entryPrefix, controllerOrBus, unit);
        } else if (def->cachemode != VIR_DOMAIN_DISK_CACHE_DEFAULT) {
            VMX_ERROR(VIR_ERR_CONFIG_UNSUPPORTED,
                      _("%s harddisk '%s' has unsupported cache mode '%s'"),
                      busName, def->dst,
                      virDomainDiskCacheTypeToString(def->cachemode));
            return -1;
        }
    }

    return 0;
}



int
virVMXFormatCDROM(virVMXContext *ctx, virDomainDiskDefPtr def,
                  virBufferPtr buffer)
{
    int controllerOrBus, unit;
    const char *busName = NULL;
    const char *entryPrefix = NULL;
    char *fileName = NULL;

    if (def->device != VIR_DOMAIN_DISK_DEVICE_CDROM) {
        VMX_ERROR(VIR_ERR_INTERNAL_ERROR, "%s", _("Invalid argument"));
        return -1;
    }

    if (def->bus == VIR_DOMAIN_DISK_BUS_SCSI) {
        busName = "SCSI";
        entryPrefix = "scsi";

        if (virVMXSCSIDiskNameToControllerAndUnit(def->dst, &controllerOrBus,
                                                  &unit) < 0) {
            return -1;
        }
    } else if (def->bus == VIR_DOMAIN_DISK_BUS_IDE) {
        busName = "IDE";
        entryPrefix = "ide";

        if (virVMXIDEDiskNameToBusAndUnit(def->dst, &controllerOrBus,
                                          &unit) < 0) {
            return -1;
        }
    } else {
        VMX_ERROR(VIR_ERR_CONFIG_UNSUPPORTED,
                  _("Unsupported bus type '%s' for cdrom"),
                  virDomainDiskBusTypeToString(def->bus));
        return -1;
    }

    virBufferVSprintf(buffer, "%s%d:%d.present = \"true\"\n",
                      entryPrefix, controllerOrBus, unit);

    if (def->type == VIR_DOMAIN_DISK_TYPE_FILE) {
        virBufferVSprintf(buffer, "%s%d:%d.deviceType = \"cdrom-image\"\n",
                          entryPrefix, controllerOrBus, unit);

        if (def->src != NULL) {
            if (! virFileHasSuffix(def->src, ".iso")) {
                VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                          _("Image file for %s cdrom '%s' has unsupported "
                            "suffix, expecting '.iso'"), busName, def->dst);
                return -1;
            }

            fileName = ctx->formatFileName(def->src, ctx->opaque);

            if (fileName == NULL) {
                return -1;
            }

            virBufferVSprintf(buffer, "%s%d:%d.fileName = \"%s\"\n",
                              entryPrefix, controllerOrBus, unit, fileName);

            VIR_FREE(fileName);
        }
    } else if (def->type == VIR_DOMAIN_DISK_TYPE_BLOCK) {
        virBufferVSprintf(buffer, "%s%d:%d.deviceType = \"atapi-cdrom\"\n",
                          entryPrefix, controllerOrBus, unit);

        if (def->src != NULL) {
            virBufferVSprintf(buffer, "%s%d:%d.fileName = \"%s\"\n",
                              entryPrefix, controllerOrBus, unit, def->src);
        }
    } else {
        VMX_ERROR(VIR_ERR_CONFIG_UNSUPPORTED,
                  _("%s cdrom '%s' has unsupported type '%s', expecting '%s' "
                    "or '%s'"), busName, def->dst,
                  virDomainDiskTypeToString(def->type),
                  virDomainDiskTypeToString(VIR_DOMAIN_DISK_TYPE_FILE),
                  virDomainDiskTypeToString(VIR_DOMAIN_DISK_TYPE_BLOCK));
        return -1;
    }

    return 0;
}



int
virVMXFormatFloppy(virVMXContext *ctx, virDomainDiskDefPtr def,
                   virBufferPtr buffer, bool floppy_present[2])
{
    int unit;
    char *fileName = NULL;

    if (def->device != VIR_DOMAIN_DISK_DEVICE_FLOPPY) {
        VMX_ERROR(VIR_ERR_INTERNAL_ERROR, "%s", _("Invalid argument"));
        return -1;
    }

    if (virVMXFloppyDiskNameToUnit(def->dst, &unit) < 0) {
        return -1;
    }

    floppy_present[unit] = true;

    virBufferVSprintf(buffer, "floppy%d.present = \"true\"\n", unit);

    if (def->type == VIR_DOMAIN_DISK_TYPE_FILE) {
        virBufferVSprintf(buffer, "floppy%d.fileType = \"file\"\n", unit);

        if (def->src != NULL) {
            if (! virFileHasSuffix(def->src, ".flp")) {
                VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                          _("Image file for floppy '%s' has unsupported "
                            "suffix, expecting '.flp'"), def->dst);
                return -1;
            }

            fileName = ctx->formatFileName(def->src, ctx->opaque);

            if (fileName == NULL) {
                return -1;
            }

            virBufferVSprintf(buffer, "floppy%d.fileName = \"%s\"\n",
                              unit, fileName);

            VIR_FREE(fileName);
        }
    } else if (def->type == VIR_DOMAIN_DISK_TYPE_BLOCK) {
        virBufferVSprintf(buffer, "floppy%d.fileType = \"device\"\n", unit);

        if (def->src != NULL) {
            virBufferVSprintf(buffer, "floppy%d.fileName = \"%s\"\n",
                              unit, def->src);
        }
    } else {
        VMX_ERROR(VIR_ERR_CONFIG_UNSUPPORTED,
                  _("Floppy '%s' has unsupported type '%s', expecting '%s' "
                    "or '%s'"), def->dst,
                  virDomainDiskTypeToString(def->type),
                  virDomainDiskTypeToString(VIR_DOMAIN_DISK_TYPE_FILE),
                  virDomainDiskTypeToString(VIR_DOMAIN_DISK_TYPE_BLOCK));
        return -1;
    }

    return 0;
}



int
virVMXFormatEthernet(virDomainNetDefPtr def, int controller,
                     virBufferPtr buffer)
{
    char mac_string[VIR_MAC_STRING_BUFLEN];
    unsigned int prefix, suffix;

    if (controller < 0 || controller > 3) {
        VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                  _("Ethernet controller index %d out of [0..3] range"),
                  controller);
        return -1;
    }

    virBufferVSprintf(buffer, "ethernet%d.present = \"true\"\n", controller);

    /* def:model -> vmx:virtualDev, vmx:features */
    if (def->model != NULL) {
        if (STRCASENEQ(def->model, "vlance") &&
            STRCASENEQ(def->model, "vmxnet") &&
            STRCASENEQ(def->model, "vmxnet2") &&
            STRCASENEQ(def->model, "vmxnet3") &&
            STRCASENEQ(def->model, "e1000")) {
            VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                      _("Expecting domain XML entry 'devices/interfase/model' "
                        "to be 'vlance' or 'vmxnet' or 'vmxnet2' or 'vmxnet3' "
                        "or 'e1000' but found '%s'"), def->model);
            return -1;
        }

        if (STRCASEEQ(def->model, "vmxnet2")) {
            virBufferVSprintf(buffer, "ethernet%d.virtualDev = \"vmxnet\"\n",
                              controller);
            virBufferVSprintf(buffer, "ethernet%d.features = \"15\"\n",
                              controller);
        } else {
            virBufferVSprintf(buffer, "ethernet%d.virtualDev = \"%s\"\n",
                              controller, def->model);
        }
    }

    /* def:type, def:ifname -> vmx:connectionType */
    switch (def->type) {
      case VIR_DOMAIN_NET_TYPE_BRIDGE:
        virBufferVSprintf(buffer, "ethernet%d.networkName = \"%s\"\n",
                          controller, def->data.bridge.brname);

        if (def->ifname != NULL) {
            virBufferVSprintf(buffer, "ethernet%d.connectionType = \"custom\"\n",
                              controller);
            virBufferVSprintf(buffer, "ethernet%d.vnet = \"%s\"\n",
                              controller, def->ifname);
        } else {
            virBufferVSprintf(buffer, "ethernet%d.connectionType = \"bridged\"\n",
                              controller);
        }

        break;

      default:
        VMX_ERROR(VIR_ERR_CONFIG_UNSUPPORTED, _("Unsupported net type '%s'"),
                  virDomainNetTypeToString(def->type));
        return -1;
    }

    /* def:mac -> vmx:addressType, vmx:(generated)Address, vmx:checkMACAddress */
    virFormatMacAddr(def->mac, mac_string);

    prefix = (def->mac[0] << 16) | (def->mac[1] << 8) | def->mac[2];
    suffix = (def->mac[3] << 16) | (def->mac[4] << 8) | def->mac[5];

    if (prefix == 0x000c29) {
        virBufferVSprintf(buffer, "ethernet%d.addressType = \"generated\"\n",
                          controller);
        virBufferVSprintf(buffer, "ethernet%d.generatedAddress = \"%s\"\n",
                          controller, mac_string);
        virBufferVSprintf(buffer, "ethernet%d.generatedAddressOffset = \"0\"\n",
                          controller);
    } else if (prefix == 0x005056 && suffix <= 0x3fffff) {
        virBufferVSprintf(buffer, "ethernet%d.addressType = \"static\"\n",
                          controller);
        virBufferVSprintf(buffer, "ethernet%d.address = \"%s\"\n",
                          controller, mac_string);
    } else if (prefix == 0x005056 && suffix >= 0x800000 && suffix <= 0xbfffff) {
        virBufferVSprintf(buffer, "ethernet%d.addressType = \"vpx\"\n",
                          controller);
        virBufferVSprintf(buffer, "ethernet%d.generatedAddress = \"%s\"\n",
                          controller, mac_string);
    } else {
        virBufferVSprintf(buffer, "ethernet%d.addressType = \"static\"\n",
                          controller);
        virBufferVSprintf(buffer, "ethernet%d.address = \"%s\"\n",
                          controller, mac_string);
        virBufferVSprintf(buffer, "ethernet%d.checkMACAddress = \"false\"\n",
                          controller);
    }

    return 0;
}



int
virVMXFormatSerial(virVMXContext *ctx, virDomainChrDefPtr def,
                   virBufferPtr buffer)
{
    char *fileName = NULL;
    const char *protocol;

    if (def->target.port < 0 || def->target.port > 3) {
        VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                  _("Serial port index %d out of [0..3] range"),
                  def->target.port);
        return -1;
    }

    virBufferVSprintf(buffer, "serial%d.present = \"true\"\n", def->target.port);

    /* def:type -> vmx:fileType and def:data.file.path -> vmx:fileName */
    switch (def->source.type) {
      case VIR_DOMAIN_CHR_TYPE_DEV:
        virBufferVSprintf(buffer, "serial%d.fileType = \"device\"\n",
                          def->target.port);
        virBufferVSprintf(buffer, "serial%d.fileName = \"%s\"\n",
                          def->target.port, def->source.data.file.path);
        break;

      case VIR_DOMAIN_CHR_TYPE_FILE:
        virBufferVSprintf(buffer, "serial%d.fileType = \"file\"\n",
                          def->target.port);

        fileName = ctx->formatFileName(def->source.data.file.path, ctx->opaque);

        if (fileName == NULL) {
            return -1;
        }

        virBufferVSprintf(buffer, "serial%d.fileName = \"%s\"\n",
                          def->target.port, fileName);

        VIR_FREE(fileName);
        break;

      case VIR_DOMAIN_CHR_TYPE_PIPE:
        virBufferVSprintf(buffer, "serial%d.fileType = \"pipe\"\n",
                          def->target.port);
        /* FIXME: Based on VI Client GUI default */
        virBufferVSprintf(buffer, "serial%d.pipe.endPoint = \"client\"\n",
                          def->target.port);
        /* FIXME: Based on VI Client GUI default */
        virBufferVSprintf(buffer, "serial%d.tryNoRxLoss = \"false\"\n",
                          def->target.port);
        virBufferVSprintf(buffer, "serial%d.fileName = \"%s\"\n",
                          def->target.port, def->source.data.file.path);
        break;

      case VIR_DOMAIN_CHR_TYPE_TCP:
        switch (def->source.data.tcp.protocol) {
          case VIR_DOMAIN_CHR_TCP_PROTOCOL_RAW:
            protocol = "tcp";
            break;

          case VIR_DOMAIN_CHR_TCP_PROTOCOL_TELNET:
            protocol = "telnet";
            break;

          case VIR_DOMAIN_CHR_TCP_PROTOCOL_TELNETS:
            protocol = "telnets";
            break;

          case VIR_DOMAIN_CHR_TCP_PROTOCOL_TLS:
            protocol = "ssl";
            break;

          default:
            VMX_ERROR(VIR_ERR_CONFIG_UNSUPPORTED,
                      _("Unsupported character device TCP protocol '%s'"),
                      virDomainChrTcpProtocolTypeToString(
                          def->source.data.tcp.protocol));
            return -1;
        }

        virBufferVSprintf(buffer, "serial%d.fileType = \"network\"\n",
                          def->target.port);
        virBufferVSprintf(buffer, "serial%d.fileName = \"%s://%s:%s\"\n",
                          def->target.port, protocol, def->source.data.tcp.host,
                          def->source.data.tcp.service);
        virBufferVSprintf(buffer, "serial%d.network.endPoint = \"%s\"\n",
                          def->target.port,
                          def->source.data.tcp.listen ? "server" : "client");
        break;

      default:
        VMX_ERROR(VIR_ERR_CONFIG_UNSUPPORTED,
                  _("Unsupported character device type '%s'"),
                  virDomainChrTypeToString(def->source.type));
        return -1;
    }

    /* vmx:yieldOnMsrRead */
    /* FIXME: Based on VI Client GUI default */
    virBufferVSprintf(buffer, "serial%d.yieldOnMsrRead = \"true\"\n",
                      def->target.port);

    return 0;
}



int
virVMXFormatParallel(virVMXContext *ctx, virDomainChrDefPtr def,
                     virBufferPtr buffer)
{
    char *fileName = NULL;

    if (def->target.port < 0 || def->target.port > 2) {
        VMX_ERROR(VIR_ERR_INTERNAL_ERROR,
                  _("Parallel port index %d out of [0..2] range"),
                  def->target.port);
        return -1;
    }

    virBufferVSprintf(buffer, "parallel%d.present = \"true\"\n",
                      def->target.port);

    /* def:type -> vmx:fileType and def:data.file.path -> vmx:fileName */
    switch (def->source.type) {
      case VIR_DOMAIN_CHR_TYPE_DEV:
        virBufferVSprintf(buffer, "parallel%d.fileType = \"device\"\n",
                          def->target.port);
        virBufferVSprintf(buffer, "parallel%d.fileName = \"%s\"\n",
                          def->target.port, def->source.data.file.path);
        break;

      case VIR_DOMAIN_CHR_TYPE_FILE:
        virBufferVSprintf(buffer, "parallel%d.fileType = \"file\"\n",
                          def->target.port);

        fileName = ctx->formatFileName(def->source.data.file.path, ctx->opaque);

        if (fileName == NULL) {
            return -1;
        }

        virBufferVSprintf(buffer, "parallel%d.fileName = \"%s\"\n",
                          def->target.port, fileName);

        VIR_FREE(fileName);
        break;

      default:
        VMX_ERROR(VIR_ERR_CONFIG_UNSUPPORTED,
                  _("Unsupported character device type '%s'"),
                  virDomainChrTypeToString(def->source.type));
        return -1;
    }

    return 0;
}



int
virVMXFormatSVGA(virDomainVideoDefPtr def, virBufferPtr buffer)
{
    unsigned long long vram;

    if (def->type != VIR_DOMAIN_VIDEO_TYPE_VMVGA) {
        VMX_ERROR(VIR_ERR_CONFIG_UNSUPPORTED,
                  _("Unsupported video device type '%s'"),
                  virDomainVideoTypeToString(def->type));
        return -1;
    }

    /*
     * For Windows guests the VRAM size should be a multiple of 64 kilobyte.
     * See http://kb.vmware.com/kb/1003 and http://kb.vmware.com/kb/1001558
     */
    vram = VIR_DIV_UP(def->vram, 64) * 64;

    if (def->heads > 1) {
        VMX_ERROR(VIR_ERR_CONFIG_UNSUPPORTED, "%s",
                  _("Multi-head video devices are unsupported"));
        return -1;
    }

    virBufferVSprintf(buffer, "svga.vramSize = \"%lld\"\n",
                      vram * 1024); /* kilobyte to byte */

    return 0;
}
