
/*
 * esx_interface_driver.c: interface driver functions for managing VMware ESX
 *                         host interfaces
 *
 * Copyright (C) 2010 Red Hat, Inc.
 * Copyright (C) 2010 Matthias Bolte <matthias.bolte@googlemail.com>
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

#include "internal.h"
#include "util.h"
#include "memory.h"
#include "logging.h"
#include "uuid.h"
#include "esx_private.h"
#include "esx_interface_driver.h"
#include "esx_vi.h"
#include "esx_vi_methods.h"
#include "esx_util.h"

#define VIR_FROM_THIS VIR_FROM_ESX



static virDrvOpenStatus
esxInterfaceOpen(virConnectPtr conn,
                 virConnectAuthPtr auth ATTRIBUTE_UNUSED,
                 int flags ATTRIBUTE_UNUSED)
{
    if (conn->driver->no != VIR_DRV_ESX) {
        return VIR_DRV_OPEN_DECLINED;
    }

    conn->interfacePrivateData = conn->privateData;

    return VIR_DRV_OPEN_SUCCESS;
}



static int
esxInterfaceClose(virConnectPtr conn)
{
    conn->interfacePrivateData = NULL;

    return 0;
}



static virInterfaceDriver esxInterfaceDriver = {
    "ESX",                                 /* name */
    esxInterfaceOpen,                      /* open */
    esxInterfaceClose,                     /* close */
    NULL,                                  /* numOfInterfaces */
    NULL,                                  /* listInterfaces */
    NULL,                                  /* numOfDefinedInterfaces */
    NULL,                                  /* listDefinedInterfaces */
    NULL,                                  /* interfaceLookupByName */
    NULL,                                  /* interfaceLookupByMACString */
    NULL,                                  /* interfaceGetXMLDesc */
    NULL,                                  /* interfaceDefineXML */
    NULL,                                  /* interfaceUndefine */
    NULL,                                  /* interfaceCreate */
    NULL,                                  /* interfaceDestroy */
    NULL,                                  /* interfaceIsActive */
};



int
esxInterfaceRegister(void)
{
    return virRegisterInterfaceDriver(&esxInterfaceDriver);
}
