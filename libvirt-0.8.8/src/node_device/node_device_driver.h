/*
 * node_device.h: node device enumeration
 *
 * Copyright (C) 2008 Virtual Iron Software, Inc.
 * Copyright (C) 2008 David F. Lively
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
 * Author: David F. Lively <dlively@virtualiron.com>
 */

#ifndef __VIR_NODE_DEVICE_H__
# define __VIR_NODE_DEVICE_H__

# include "internal.h"
# include "driver.h"
# include "node_device_conf.h"

# define LINUX_SYSFS_SCSI_HOST_PREFIX "/sys/class/scsi_host/"
# define LINUX_SYSFS_SCSI_HOST_POSTFIX "device"
# define LINUX_SYSFS_FC_HOST_PREFIX "/sys/class/fc_host/"

# define VPORT_CREATE 0
# define VPORT_DELETE 1
# define LINUX_SYSFS_VPORT_CREATE_POSTFIX "/vport_create"
# define LINUX_SYSFS_VPORT_DELETE_POSTFIX "/vport_delete"

# define SRIOV_FOUND 0
# define SRIOV_NOT_FOUND 1
# define SRIOV_ERROR -1

# define LINUX_NEW_DEVICE_WAIT_TIME 60

# ifdef HAVE_HAL
int halNodeRegister(void);
# endif
# ifdef HAVE_UDEV
int udevNodeRegister(void);
# endif

void nodeDeviceLock(virDeviceMonitorStatePtr driver);
void nodeDeviceUnlock(virDeviceMonitorStatePtr driver);

void registerCommonNodeFuncs(virDeviceMonitorPtr mon);

int nodedevRegister(void);

# ifdef __linux__

#  define check_fc_host(d) check_fc_host_linux(d)
int check_fc_host_linux(union _virNodeDevCapData *d);

#  define check_vport_capable(d) check_vport_capable_linux(d)
int check_vport_capable_linux(union _virNodeDevCapData *d);

#  define get_physical_function(s,d) get_physical_function_linux(s,d)
int get_physical_function_linux(const char *sysfs_path,
                                union _virNodeDevCapData *d);

#  define get_virtual_functions(s,d) get_virtual_functions_linux(s,d)
int get_virtual_functions_linux(const char *sysfs_path,
                                union _virNodeDevCapData *d);

#  define read_wwn(host, file, wwn) read_wwn_linux(host, file, wwn)
int read_wwn_linux(int host, const char *file, char **wwn);

# else  /* __linux__ */

#  define check_fc_host(d)
#  define check_vport_capable(d)
#  define get_physical_function(sysfs_path, d)
#  define get_virtual_functions(sysfs_path, d)
#  define read_wwn(host, file, wwn)

# endif /* __linux__ */

#endif /* __VIR_NODE_DEVICE_H__ */
