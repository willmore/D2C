/* Generated */

#include <Python.h>
#include <libvirt/libvirt.h>
#include "typewrappers.h"
#include "libvirt.h"

PyObject *
libvirt_virStoragePoolGetXMLDesc(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    char * c_retval;
    virStoragePoolPtr pool;
    PyObject *pyobj_pool;
    unsigned int flags;

    if (!PyArg_ParseTuple(args, (char *)"Oi:virStoragePoolGetXMLDesc", &pyobj_pool, &flags))
        return(NULL);
    pool = (virStoragePoolPtr) PyvirStoragePool_Get(pyobj_pool);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virStoragePoolGetXMLDesc(pool, flags);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_charPtrWrap((char *) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virStorageVolGetKey(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    const char * c_retval;
    virStorageVolPtr vol;
    PyObject *pyobj_vol;

    if (!PyArg_ParseTuple(args, (char *)"O:virStorageVolGetKey", &pyobj_vol))
        return(NULL);
    vol = (virStorageVolPtr) PyvirStorageVol_Get(pyobj_vol);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virStorageVolGetKey(vol);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_charPtrConstWrap((const char *) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virDomainManagedSave(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virDomainPtr dom;
    PyObject *pyobj_dom;
    unsigned int flags;

    if (!PyArg_ParseTuple(args, (char *)"Oi:virDomainManagedSave", &pyobj_dom, &flags))
        return(NULL);
    dom = (virDomainPtr) PyvirDomain_Get(pyobj_dom);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virDomainManagedSave(dom, flags);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virDomainDefineXML(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    virDomainPtr c_retval;
    virConnectPtr conn;
    PyObject *pyobj_conn;
    char * xml;

    if (!PyArg_ParseTuple(args, (char *)"Oz:virDomainDefineXML", &pyobj_conn, &xml))
        return(NULL);
    conn = (virConnectPtr) PyvirConnect_Get(pyobj_conn);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virDomainDefineXML(conn, xml);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_virDomainPtrWrap((virDomainPtr) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virDomainShutdown(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virDomainPtr domain;
    PyObject *pyobj_domain;

    if (!PyArg_ParseTuple(args, (char *)"O:virDomainShutdown", &pyobj_domain))
        return(NULL);
    domain = (virDomainPtr) PyvirDomain_Get(pyobj_domain);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virDomainShutdown(domain);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virDomainUndefine(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virDomainPtr domain;
    PyObject *pyobj_domain;

    if (!PyArg_ParseTuple(args, (char *)"O:virDomainUndefine", &pyobj_domain))
        return(NULL);
    domain = (virDomainPtr) PyvirDomain_Get(pyobj_domain);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virDomainUndefine(domain);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virStorageVolWipe(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virStorageVolPtr vol;
    PyObject *pyobj_vol;
    unsigned int flags;

    if (!PyArg_ParseTuple(args, (char *)"Oi:virStorageVolWipe", &pyobj_vol, &flags))
        return(NULL);
    vol = (virStorageVolPtr) PyvirStorageVol_Get(pyobj_vol);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virStorageVolWipe(vol, flags);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virDomainAbortJob(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virDomainPtr domain;
    PyObject *pyobj_domain;

    if (!PyArg_ParseTuple(args, (char *)"O:virDomainAbortJob", &pyobj_domain))
        return(NULL);
    domain = (virDomainPtr) PyvirDomain_Get(pyobj_domain);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virDomainAbortJob(domain);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virNodeDeviceGetName(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    const char * c_retval;
    virNodeDevicePtr dev;
    PyObject *pyobj_dev;

    if (!PyArg_ParseTuple(args, (char *)"O:virNodeDeviceGetName", &pyobj_dev))
        return(NULL);
    dev = (virNodeDevicePtr) PyvirNodeDevice_Get(pyobj_dev);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virNodeDeviceGetName(dev);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_charPtrConstWrap((const char *) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virConnectNumOfSecrets(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virConnectPtr conn;
    PyObject *pyobj_conn;

    if (!PyArg_ParseTuple(args, (char *)"O:virConnectNumOfSecrets", &pyobj_conn))
        return(NULL);
    conn = (virConnectPtr) PyvirConnect_Get(pyobj_conn);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virConnectNumOfSecrets(conn);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virDomainSnapshotCurrent(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    virDomainSnapshotPtr c_retval;
    virDomainPtr domain;
    PyObject *pyobj_domain;
    unsigned int flags;

    if (!PyArg_ParseTuple(args, (char *)"Oi:virDomainSnapshotCurrent", &pyobj_domain, &flags))
        return(NULL);
    domain = (virDomainPtr) PyvirDomain_Get(pyobj_domain);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virDomainSnapshotCurrent(domain, flags);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_virDomainSnapshotPtrWrap((virDomainSnapshotPtr) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virConnectDomainXMLFromNative(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    char * c_retval;
    virConnectPtr conn;
    PyObject *pyobj_conn;
    char * nativeFormat;
    char * nativeConfig;
    unsigned int flags;

    if (!PyArg_ParseTuple(args, (char *)"Ozzi:virConnectDomainXMLFromNative", &pyobj_conn, &nativeFormat, &nativeConfig, &flags))
        return(NULL);
    conn = (virConnectPtr) PyvirConnect_Get(pyobj_conn);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virConnectDomainXMLFromNative(conn, nativeFormat, nativeConfig, flags);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_charPtrWrap((char *) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virNodeDeviceDettach(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virNodeDevicePtr dev;
    PyObject *pyobj_dev;

    if (!PyArg_ParseTuple(args, (char *)"O:virNodeDeviceDettach", &pyobj_dev))
        return(NULL);
    dev = (virNodeDevicePtr) PyvirNodeDevice_Get(pyobj_dev);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virNodeDeviceDettach(dev);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virDomainLookupByUUIDString(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    virDomainPtr c_retval;
    virConnectPtr conn;
    PyObject *pyobj_conn;
    char * uuidstr;

    if (!PyArg_ParseTuple(args, (char *)"Oz:virDomainLookupByUUIDString", &pyobj_conn, &uuidstr))
        return(NULL);
    conn = (virConnectPtr) PyvirConnect_Get(pyobj_conn);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virDomainLookupByUUIDString(conn, uuidstr);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_virDomainPtrWrap((virDomainPtr) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virDomainUpdateDeviceFlags(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virDomainPtr domain;
    PyObject *pyobj_domain;
    char * xml;
    unsigned int flags;

    if (!PyArg_ParseTuple(args, (char *)"Ozi:virDomainUpdateDeviceFlags", &pyobj_domain, &xml, &flags))
        return(NULL);
    domain = (virDomainPtr) PyvirDomain_Get(pyobj_domain);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virDomainUpdateDeviceFlags(domain, xml, flags);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virConnectNumOfDefinedNetworks(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virConnectPtr conn;
    PyObject *pyobj_conn;

    if (!PyArg_ParseTuple(args, (char *)"O:virConnectNumOfDefinedNetworks", &pyobj_conn))
        return(NULL);
    conn = (virConnectPtr) PyvirConnect_Get(pyobj_conn);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virConnectNumOfDefinedNetworks(conn);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virStoragePoolIsActive(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virStoragePoolPtr pool;
    PyObject *pyobj_pool;

    if (!PyArg_ParseTuple(args, (char *)"O:virStoragePoolIsActive", &pyobj_pool))
        return(NULL);
    pool = (virStoragePoolPtr) PyvirStoragePool_Get(pyobj_pool);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virStoragePoolIsActive(pool);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virDomainIsUpdated(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virDomainPtr dom;
    PyObject *pyobj_dom;

    if (!PyArg_ParseTuple(args, (char *)"O:virDomainIsUpdated", &pyobj_dom))
        return(NULL);
    dom = (virDomainPtr) PyvirDomain_Get(pyobj_dom);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virDomainIsUpdated(dom);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virStoragePoolSetAutostart(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virStoragePoolPtr pool;
    PyObject *pyobj_pool;
    int autostart;

    if (!PyArg_ParseTuple(args, (char *)"Oi:virStoragePoolSetAutostart", &pyobj_pool, &autostart))
        return(NULL);
    pool = (virStoragePoolPtr) PyvirStoragePool_Get(pyobj_pool);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virStoragePoolSetAutostart(pool, autostart);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virInterfaceGetXMLDesc(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    char * c_retval;
    virInterfacePtr iface;
    PyObject *pyobj_iface;
    unsigned int flags;

    if (!PyArg_ParseTuple(args, (char *)"Oi:virInterfaceGetXMLDesc", &pyobj_iface, &flags))
        return(NULL);
    iface = (virInterfacePtr) PyvirInterface_Get(pyobj_iface);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virInterfaceGetXMLDesc(iface, flags);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_charPtrWrap((char *) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virDomainFree(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virDomainPtr domain;
    PyObject *pyobj_domain;

    if (!PyArg_ParseTuple(args, (char *)"O:virDomainFree", &pyobj_domain))
        return(NULL);
    domain = (virDomainPtr) PyvirDomain_Get(pyobj_domain);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virDomainFree(domain);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virStoragePoolGetName(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    const char * c_retval;
    virStoragePoolPtr pool;
    PyObject *pyobj_pool;

    if (!PyArg_ParseTuple(args, (char *)"O:virStoragePoolGetName", &pyobj_pool))
        return(NULL);
    pool = (virStoragePoolPtr) PyvirStoragePool_Get(pyobj_pool);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virStoragePoolGetName(pool);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_charPtrConstWrap((const char *) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virConnectGetMaxVcpus(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virConnectPtr conn;
    PyObject *pyobj_conn;
    char * type;

    if (!PyArg_ParseTuple(args, (char *)"Oz:virConnectGetMaxVcpus", &pyobj_conn, &type))
        return(NULL);
    conn = (virConnectPtr) PyvirConnect_Get(pyobj_conn);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virConnectGetMaxVcpus(conn, type);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virStoragePoolDefineXML(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    virStoragePoolPtr c_retval;
    virConnectPtr conn;
    PyObject *pyobj_conn;
    char * xml;
    unsigned int flags;

    if (!PyArg_ParseTuple(args, (char *)"Ozi:virStoragePoolDefineXML", &pyobj_conn, &xml, &flags))
        return(NULL);
    conn = (virConnectPtr) PyvirConnect_Get(pyobj_conn);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virStoragePoolDefineXML(conn, xml, flags);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_virStoragePoolPtrWrap((virStoragePoolPtr) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virStorageVolLookupByPath(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    virStorageVolPtr c_retval;
    virConnectPtr conn;
    PyObject *pyobj_conn;
    char * path;

    if (!PyArg_ParseTuple(args, (char *)"Oz:virStorageVolLookupByPath", &pyobj_conn, &path))
        return(NULL);
    conn = (virConnectPtr) PyvirConnect_Get(pyobj_conn);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virStorageVolLookupByPath(conn, path);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_virStorageVolPtrWrap((virStorageVolPtr) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virStorageVolLookupByName(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    virStorageVolPtr c_retval;
    virStoragePoolPtr pool;
    PyObject *pyobj_pool;
    char * name;

    if (!PyArg_ParseTuple(args, (char *)"Oz:virStorageVolLookupByName", &pyobj_pool, &name))
        return(NULL);
    pool = (virStoragePoolPtr) PyvirStoragePool_Get(pyobj_pool);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virStorageVolLookupByName(pool, name);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_virStorageVolPtrWrap((virStorageVolPtr) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virNetworkCreate(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virNetworkPtr network;
    PyObject *pyobj_network;

    if (!PyArg_ParseTuple(args, (char *)"O:virNetworkCreate", &pyobj_network))
        return(NULL);
    network = (virNetworkPtr) PyvirNetwork_Get(pyobj_network);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virNetworkCreate(network);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virInterfaceDefineXML(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    virInterfacePtr c_retval;
    virConnectPtr conn;
    PyObject *pyobj_conn;
    char * xml;
    unsigned int flags;

    if (!PyArg_ParseTuple(args, (char *)"Ozi:virInterfaceDefineXML", &pyobj_conn, &xml, &flags))
        return(NULL);
    conn = (virConnectPtr) PyvirConnect_Get(pyobj_conn);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virInterfaceDefineXML(conn, xml, flags);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_virInterfacePtrWrap((virInterfacePtr) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virDomainMigrate(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    virDomainPtr c_retval;
    virDomainPtr domain;
    PyObject *pyobj_domain;
    virConnectPtr dconn;
    PyObject *pyobj_dconn;
    unsigned long flags;
    char * dname;
    char * uri;
    unsigned long bandwidth;

    if (!PyArg_ParseTuple(args, (char *)"OOlzzl:virDomainMigrate", &pyobj_domain, &pyobj_dconn, &flags, &dname, &uri, &bandwidth))
        return(NULL);
    domain = (virDomainPtr) PyvirDomain_Get(pyobj_domain);
    dconn = (virConnectPtr) PyvirConnect_Get(pyobj_dconn);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virDomainMigrate(domain, dconn, flags, dname, uri, bandwidth);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_virDomainPtrWrap((virDomainPtr) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virDomainSuspend(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virDomainPtr domain;
    PyObject *pyobj_domain;

    if (!PyArg_ParseTuple(args, (char *)"O:virDomainSuspend", &pyobj_domain))
        return(NULL);
    domain = (virDomainPtr) PyvirDomain_Get(pyobj_domain);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virDomainSuspend(domain);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virDomainCreateLinux(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    virDomainPtr c_retval;
    virConnectPtr conn;
    PyObject *pyobj_conn;
    char * xmlDesc;
    unsigned int flags;

    if (!PyArg_ParseTuple(args, (char *)"Ozi:virDomainCreateLinux", &pyobj_conn, &xmlDesc, &flags))
        return(NULL);
    conn = (virConnectPtr) PyvirConnect_Get(pyobj_conn);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virDomainCreateLinux(conn, xmlDesc, flags);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_virDomainPtrWrap((virDomainPtr) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virStreamEventRemoveCallback(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virStreamPtr stream;
    PyObject *pyobj_stream;

    if (!PyArg_ParseTuple(args, (char *)"O:virStreamEventRemoveCallback", &pyobj_stream))
        return(NULL);
    stream = (virStreamPtr) PyvirStream_Get(pyobj_stream);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virStreamEventRemoveCallback(stream);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virNodeDeviceGetXMLDesc(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    char * c_retval;
    virNodeDevicePtr dev;
    PyObject *pyobj_dev;
    unsigned int flags;

    if (!PyArg_ParseTuple(args, (char *)"Oi:virNodeDeviceGetXMLDesc", &pyobj_dev, &flags))
        return(NULL);
    dev = (virNodeDevicePtr) PyvirNodeDevice_Get(pyobj_dev);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virNodeDeviceGetXMLDesc(dev, flags);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_charPtrWrap((char *) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virDomainDestroy(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virDomainPtr domain;
    PyObject *pyobj_domain;

    if (!PyArg_ParseTuple(args, (char *)"O:virDomainDestroy", &pyobj_domain))
        return(NULL);
    domain = (virDomainPtr) PyvirDomain_Get(pyobj_domain);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virDomainDestroy(domain);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virConnectNumOfNetworks(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virConnectPtr conn;
    PyObject *pyobj_conn;

    if (!PyArg_ParseTuple(args, (char *)"O:virConnectNumOfNetworks", &pyobj_conn))
        return(NULL);
    conn = (virConnectPtr) PyvirConnect_Get(pyobj_conn);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virConnectNumOfNetworks(conn);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virStorageVolGetName(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    const char * c_retval;
    virStorageVolPtr vol;
    PyObject *pyobj_vol;

    if (!PyArg_ParseTuple(args, (char *)"O:virStorageVolGetName", &pyobj_vol))
        return(NULL);
    vol = (virStorageVolPtr) PyvirStorageVol_Get(pyobj_vol);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virStorageVolGetName(vol);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_charPtrConstWrap((const char *) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virStoragePoolLookupByUUIDString(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    virStoragePoolPtr c_retval;
    virConnectPtr conn;
    PyObject *pyobj_conn;
    char * uuidstr;

    if (!PyArg_ParseTuple(args, (char *)"Oz:virStoragePoolLookupByUUIDString", &pyobj_conn, &uuidstr))
        return(NULL);
    conn = (virConnectPtr) PyvirConnect_Get(pyobj_conn);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virStoragePoolLookupByUUIDString(conn, uuidstr);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_virStoragePoolPtrWrap((virStoragePoolPtr) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virInterfaceCreate(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virInterfacePtr iface;
    PyObject *pyobj_iface;
    unsigned int flags;

    if (!PyArg_ParseTuple(args, (char *)"Oi:virInterfaceCreate", &pyobj_iface, &flags))
        return(NULL);
    iface = (virInterfacePtr) PyvirInterface_Get(pyobj_iface);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virInterfaceCreate(iface, flags);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virNWFilterLookupByName(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    virNWFilterPtr c_retval;
    virConnectPtr conn;
    PyObject *pyobj_conn;
    char * name;

    if (!PyArg_ParseTuple(args, (char *)"Oz:virNWFilterLookupByName", &pyobj_conn, &name))
        return(NULL);
    conn = (virConnectPtr) PyvirConnect_Get(pyobj_conn);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virNWFilterLookupByName(conn, name);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_virNWFilterPtrWrap((virNWFilterPtr) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virDomainGetXMLDesc(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    char * c_retval;
    virDomainPtr domain;
    PyObject *pyobj_domain;
    int flags;

    if (!PyArg_ParseTuple(args, (char *)"Oi:virDomainGetXMLDesc", &pyobj_domain, &flags))
        return(NULL);
    domain = (virDomainPtr) PyvirDomain_Get(pyobj_domain);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virDomainGetXMLDesc(domain, flags);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_charPtrWrap((char *) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virSecretLookupByUsage(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    virSecretPtr c_retval;
    virConnectPtr conn;
    PyObject *pyobj_conn;
    int usageType;
    char * usageID;

    if (!PyArg_ParseTuple(args, (char *)"Oiz:virSecretLookupByUsage", &pyobj_conn, &usageType, &usageID))
        return(NULL);
    conn = (virConnectPtr) PyvirConnect_Get(pyobj_conn);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virSecretLookupByUsage(conn, usageType, usageID);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_virSecretPtrWrap((virSecretPtr) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virConnectCompareCPU(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virConnectPtr conn;
    PyObject *pyobj_conn;
    char * xmlDesc;
    unsigned int flags;

    if (!PyArg_ParseTuple(args, (char *)"Ozi:virConnectCompareCPU", &pyobj_conn, &xmlDesc, &flags))
        return(NULL);
    conn = (virConnectPtr) PyvirConnect_Get(pyobj_conn);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virConnectCompareCPU(conn, xmlDesc, flags);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virNetworkDefineXML(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    virNetworkPtr c_retval;
    virConnectPtr conn;
    PyObject *pyobj_conn;
    char * xml;

    if (!PyArg_ParseTuple(args, (char *)"Oz:virNetworkDefineXML", &pyobj_conn, &xml))
        return(NULL);
    conn = (virConnectPtr) PyvirConnect_Get(pyobj_conn);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virNetworkDefineXML(conn, xml);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_virNetworkPtrWrap((virNetworkPtr) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virDomainMigrateSetMaxDowntime(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virDomainPtr domain;
    PyObject *pyobj_domain;
    unsigned long long downtime;
    unsigned int flags;

    if (!PyArg_ParseTuple(args, (char *)"Oli:virDomainMigrateSetMaxDowntime", &pyobj_domain, &downtime, &flags))
        return(NULL);
    domain = (virDomainPtr) PyvirDomain_Get(pyobj_domain);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virDomainMigrateSetMaxDowntime(domain, downtime, flags);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virNodeDeviceFree(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virNodeDevicePtr dev;
    PyObject *pyobj_dev;

    if (!PyArg_ParseTuple(args, (char *)"O:virNodeDeviceFree", &pyobj_dev))
        return(NULL);
    dev = (virNodeDevicePtr) PyvirNodeDevice_Get(pyobj_dev);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virNodeDeviceFree(dev);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virSecretGetUsageType(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virSecretPtr secret;
    PyObject *pyobj_secret;

    if (!PyArg_ParseTuple(args, (char *)"O:virSecretGetUsageType", &pyobj_secret))
        return(NULL);
    secret = (virSecretPtr) PyvirSecret_Get(pyobj_secret);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virSecretGetUsageType(secret);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virDomainCreateWithFlags(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virDomainPtr domain;
    PyObject *pyobj_domain;
    unsigned int flags;

    if (!PyArg_ParseTuple(args, (char *)"Oi:virDomainCreateWithFlags", &pyobj_domain, &flags))
        return(NULL);
    domain = (virDomainPtr) PyvirDomain_Get(pyobj_domain);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virDomainCreateWithFlags(domain, flags);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virNodeDeviceCreateXML(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    virNodeDevicePtr c_retval;
    virConnectPtr conn;
    PyObject *pyobj_conn;
    char * xmlDesc;
    unsigned int flags;

    if (!PyArg_ParseTuple(args, (char *)"Ozi:virNodeDeviceCreateXML", &pyobj_conn, &xmlDesc, &flags))
        return(NULL);
    conn = (virConnectPtr) PyvirConnect_Get(pyobj_conn);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virNodeDeviceCreateXML(conn, xmlDesc, flags);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_virNodeDevicePtrWrap((virNodeDevicePtr) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virStorageVolDelete(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virStorageVolPtr vol;
    PyObject *pyobj_vol;
    unsigned int flags;

    if (!PyArg_ParseTuple(args, (char *)"Oi:virStorageVolDelete", &pyobj_vol, &flags))
        return(NULL);
    vol = (virStorageVolPtr) PyvirStorageVol_Get(pyobj_vol);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virStorageVolDelete(vol, flags);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virSecretDefineXML(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    virSecretPtr c_retval;
    virConnectPtr conn;
    PyObject *pyobj_conn;
    char * xml;
    unsigned int flags;

    if (!PyArg_ParseTuple(args, (char *)"Ozi:virSecretDefineXML", &pyobj_conn, &xml, &flags))
        return(NULL);
    conn = (virConnectPtr) PyvirConnect_Get(pyobj_conn);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virSecretDefineXML(conn, xml, flags);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_virSecretPtrWrap((virSecretPtr) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virNetworkDestroy(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virNetworkPtr network;
    PyObject *pyobj_network;

    if (!PyArg_ParseTuple(args, (char *)"O:virNetworkDestroy", &pyobj_network))
        return(NULL);
    network = (virNetworkPtr) PyvirNetwork_Get(pyobj_network);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virNetworkDestroy(network);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virStoragePoolLookupByName(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    virStoragePoolPtr c_retval;
    virConnectPtr conn;
    PyObject *pyobj_conn;
    char * name;

    if (!PyArg_ParseTuple(args, (char *)"Oz:virStoragePoolLookupByName", &pyobj_conn, &name))
        return(NULL);
    conn = (virConnectPtr) PyvirConnect_Get(pyobj_conn);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virStoragePoolLookupByName(conn, name);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_virStoragePoolPtrWrap((virStoragePoolPtr) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virStoragePoolCreateXML(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    virStoragePoolPtr c_retval;
    virConnectPtr conn;
    PyObject *pyobj_conn;
    char * xmlDesc;
    unsigned int flags;

    if (!PyArg_ParseTuple(args, (char *)"Ozi:virStoragePoolCreateXML", &pyobj_conn, &xmlDesc, &flags))
        return(NULL);
    conn = (virConnectPtr) PyvirConnect_Get(pyobj_conn);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virStoragePoolCreateXML(conn, xmlDesc, flags);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_virStoragePoolPtrWrap((virStoragePoolPtr) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virNWFilterFree(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virNWFilterPtr nwfilter;
    PyObject *pyobj_nwfilter;

    if (!PyArg_ParseTuple(args, (char *)"O:virNWFilterFree", &pyobj_nwfilter))
        return(NULL);
    nwfilter = (virNWFilterPtr) PyvirNWFilter_Get(pyobj_nwfilter);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virNWFilterFree(nwfilter);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virDomainSnapshotGetXMLDesc(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    char * c_retval;
    virDomainSnapshotPtr snapshot;
    PyObject *pyobj_snapshot;
    unsigned int flags;

    if (!PyArg_ParseTuple(args, (char *)"Oi:virDomainSnapshotGetXMLDesc", &pyobj_snapshot, &flags))
        return(NULL);
    snapshot = (virDomainSnapshotPtr) PyvirDomainSnapshot_Get(pyobj_snapshot);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virDomainSnapshotGetXMLDesc(snapshot, flags);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_charPtrWrap((char *) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virStorageVolGetXMLDesc(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    char * c_retval;
    virStorageVolPtr vol;
    PyObject *pyobj_vol;
    unsigned int flags;

    if (!PyArg_ParseTuple(args, (char *)"Oi:virStorageVolGetXMLDesc", &pyobj_vol, &flags))
        return(NULL);
    vol = (virStorageVolPtr) PyvirStorageVol_Get(pyobj_vol);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virStorageVolGetXMLDesc(vol, flags);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_charPtrWrap((char *) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virConnectDomainXMLToNative(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    char * c_retval;
    virConnectPtr conn;
    PyObject *pyobj_conn;
    char * nativeFormat;
    char * domainXml;
    unsigned int flags;

    if (!PyArg_ParseTuple(args, (char *)"Ozzi:virConnectDomainXMLToNative", &pyobj_conn, &nativeFormat, &domainXml, &flags))
        return(NULL);
    conn = (virConnectPtr) PyvirConnect_Get(pyobj_conn);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virConnectDomainXMLToNative(conn, nativeFormat, domainXml, flags);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_charPtrWrap((char *) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virConnectGetType(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    const char * c_retval;
    virConnectPtr conn;
    PyObject *pyobj_conn;

    if (!PyArg_ParseTuple(args, (char *)"O:virConnectGetType", &pyobj_conn))
        return(NULL);
    conn = (virConnectPtr) PyvirConnect_Get(pyobj_conn);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virConnectGetType(conn);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_charPtrConstWrap((const char *) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virStorageVolCreateXML(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    virStorageVolPtr c_retval;
    virStoragePoolPtr pool;
    PyObject *pyobj_pool;
    char * xmldesc;
    unsigned int flags;

    if (!PyArg_ParseTuple(args, (char *)"Ozi:virStorageVolCreateXML", &pyobj_pool, &xmldesc, &flags))
        return(NULL);
    pool = (virStoragePoolPtr) PyvirStoragePool_Get(pyobj_pool);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virStorageVolCreateXML(pool, xmldesc, flags);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_virStorageVolPtrWrap((virStorageVolPtr) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virDomainSnapshotDelete(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virDomainSnapshotPtr snapshot;
    PyObject *pyobj_snapshot;
    unsigned int flags;

    if (!PyArg_ParseTuple(args, (char *)"Oi:virDomainSnapshotDelete", &pyobj_snapshot, &flags))
        return(NULL);
    snapshot = (virDomainSnapshotPtr) PyvirDomainSnapshot_Get(pyobj_snapshot);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virDomainSnapshotDelete(snapshot, flags);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virDomainSave(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virDomainPtr domain;
    PyObject *pyobj_domain;
    char * to;

    if (!PyArg_ParseTuple(args, (char *)"Oz:virDomainSave", &pyobj_domain, &to))
        return(NULL);
    domain = (virDomainPtr) PyvirDomain_Get(pyobj_domain);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virDomainSave(domain, to);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virDomainCreate(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virDomainPtr domain;
    PyObject *pyobj_domain;

    if (!PyArg_ParseTuple(args, (char *)"O:virDomainCreate", &pyobj_domain))
        return(NULL);
    domain = (virDomainPtr) PyvirDomain_Get(pyobj_domain);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virDomainCreate(domain);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virDomainCoreDump(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virDomainPtr domain;
    PyObject *pyobj_domain;
    char * to;
    int flags;

    if (!PyArg_ParseTuple(args, (char *)"Ozi:virDomainCoreDump", &pyobj_domain, &to, &flags))
        return(NULL);
    domain = (virDomainPtr) PyvirDomain_Get(pyobj_domain);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virDomainCoreDump(domain, to, flags);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virDomainSetMemory(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virDomainPtr domain;
    PyObject *pyobj_domain;
    unsigned long memory;

    if (!PyArg_ParseTuple(args, (char *)"Ol:virDomainSetMemory", &pyobj_domain, &memory))
        return(NULL);
    domain = (virDomainPtr) PyvirDomain_Get(pyobj_domain);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virDomainSetMemory(domain, memory);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virConnectIsSecure(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virConnectPtr conn;
    PyObject *pyobj_conn;

    if (!PyArg_ParseTuple(args, (char *)"O:virConnectIsSecure", &pyobj_conn))
        return(NULL);
    conn = (virConnectPtr) PyvirConnect_Get(pyobj_conn);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virConnectIsSecure(conn);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virResetLastError(PyObject *self ATTRIBUTE_UNUSED, PyObject *args ATTRIBUTE_UNUSED) {
LIBVIRT_BEGIN_ALLOW_THREADS;

    virResetLastError();
LIBVIRT_END_ALLOW_THREADS;
    Py_INCREF(Py_None);
    return(Py_None);
}

PyObject *
libvirt_virInterfaceGetName(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    const char * c_retval;
    virInterfacePtr iface;
    PyObject *pyobj_iface;

    if (!PyArg_ParseTuple(args, (char *)"O:virInterfaceGetName", &pyobj_iface))
        return(NULL);
    iface = (virInterfacePtr) PyvirInterface_Get(pyobj_iface);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virInterfaceGetName(iface);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_charPtrConstWrap((const char *) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virDomainOpenConsole(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virDomainPtr dom;
    PyObject *pyobj_dom;
    char * devname;
    virStreamPtr st;
    PyObject *pyobj_st;
    unsigned int flags;

    if (!PyArg_ParseTuple(args, (char *)"OzOi:virDomainOpenConsole", &pyobj_dom, &devname, &pyobj_st, &flags))
        return(NULL);
    dom = (virDomainPtr) PyvirDomain_Get(pyobj_dom);
    st = (virStreamPtr) PyvirStream_Get(pyobj_st);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virDomainOpenConsole(dom, devname, st, flags);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virDomainSetVcpusFlags(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virDomainPtr domain;
    PyObject *pyobj_domain;
    unsigned int nvcpus;
    unsigned int flags;

    if (!PyArg_ParseTuple(args, (char *)"Oii:virDomainSetVcpusFlags", &pyobj_domain, &nvcpus, &flags))
        return(NULL);
    domain = (virDomainPtr) PyvirDomain_Get(pyobj_domain);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virDomainSetVcpusFlags(domain, nvcpus, flags);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virStoragePoolCreate(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virStoragePoolPtr pool;
    PyObject *pyobj_pool;
    unsigned int flags;

    if (!PyArg_ParseTuple(args, (char *)"Oi:virStoragePoolCreate", &pyobj_pool, &flags))
        return(NULL);
    pool = (virStoragePoolPtr) PyvirStoragePool_Get(pyobj_pool);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virStoragePoolCreate(pool, flags);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virStoragePoolDelete(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virStoragePoolPtr pool;
    PyObject *pyobj_pool;
    unsigned int flags;

    if (!PyArg_ParseTuple(args, (char *)"Oi:virStoragePoolDelete", &pyobj_pool, &flags))
        return(NULL);
    pool = (virStoragePoolPtr) PyvirStoragePool_Get(pyobj_pool);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virStoragePoolDelete(pool, flags);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virNetworkSetAutostart(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virNetworkPtr network;
    PyObject *pyobj_network;
    int autostart;

    if (!PyArg_ParseTuple(args, (char *)"Oi:virNetworkSetAutostart", &pyobj_network, &autostart))
        return(NULL);
    network = (virNetworkPtr) PyvirNetwork_Get(pyobj_network);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virNetworkSetAutostart(network, autostart);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virSecretLookupByUUIDString(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    virSecretPtr c_retval;
    virConnectPtr conn;
    PyObject *pyobj_conn;
    char * uuidstr;

    if (!PyArg_ParseTuple(args, (char *)"Oz:virSecretLookupByUUIDString", &pyobj_conn, &uuidstr))
        return(NULL);
    conn = (virConnectPtr) PyvirConnect_Get(pyobj_conn);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virSecretLookupByUUIDString(conn, uuidstr);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_virSecretPtrWrap((virSecretPtr) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virDomainGetMaxMemory(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    unsigned long c_retval;
    virDomainPtr domain;
    PyObject *pyobj_domain;

    if (!PyArg_ParseTuple(args, (char *)"O:virDomainGetMaxMemory", &pyobj_domain))
        return(NULL);
    domain = (virDomainPtr) PyvirDomain_Get(pyobj_domain);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virDomainGetMaxMemory(domain);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_longWrap((long) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virDomainManagedSaveRemove(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virDomainPtr dom;
    PyObject *pyobj_dom;
    unsigned int flags;

    if (!PyArg_ParseTuple(args, (char *)"Oi:virDomainManagedSaveRemove", &pyobj_dom, &flags))
        return(NULL);
    dom = (virDomainPtr) PyvirDomain_Get(pyobj_dom);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virDomainManagedSaveRemove(dom, flags);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virStreamAbort(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virStreamPtr stream;
    PyObject *pyobj_stream;

    if (!PyArg_ParseTuple(args, (char *)"O:virStreamAbort", &pyobj_stream))
        return(NULL);
    stream = (virStreamPtr) PyvirStream_Get(pyobj_stream);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virStreamAbort(stream);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virDomainSetMaxMemory(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virDomainPtr domain;
    PyObject *pyobj_domain;
    unsigned long memory;

    if (!PyArg_ParseTuple(args, (char *)"Ol:virDomainSetMaxMemory", &pyobj_domain, &memory))
        return(NULL);
    domain = (virDomainPtr) PyvirDomain_Get(pyobj_domain);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virDomainSetMaxMemory(domain, memory);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virNetworkIsPersistent(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virNetworkPtr net;
    PyObject *pyobj_net;

    if (!PyArg_ParseTuple(args, (char *)"O:virNetworkIsPersistent", &pyobj_net))
        return(NULL);
    net = (virNetworkPtr) PyvirNetwork_Get(pyobj_net);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virNetworkIsPersistent(net);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virNWFilterGetName(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    const char * c_retval;
    virNWFilterPtr nwfilter;
    PyObject *pyobj_nwfilter;

    if (!PyArg_ParseTuple(args, (char *)"O:virNWFilterGetName", &pyobj_nwfilter))
        return(NULL);
    nwfilter = (virNWFilterPtr) PyvirNWFilter_Get(pyobj_nwfilter);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virNWFilterGetName(nwfilter);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_charPtrConstWrap((const char *) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virNodeDeviceDestroy(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virNodeDevicePtr dev;
    PyObject *pyobj_dev;

    if (!PyArg_ParseTuple(args, (char *)"O:virNodeDeviceDestroy", &pyobj_dev))
        return(NULL);
    dev = (virNodeDevicePtr) PyvirNodeDevice_Get(pyobj_dev);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virNodeDeviceDestroy(dev);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virStoragePoolFree(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virStoragePoolPtr pool;
    PyObject *pyobj_pool;

    if (!PyArg_ParseTuple(args, (char *)"O:virStoragePoolFree", &pyobj_pool))
        return(NULL);
    pool = (virStoragePoolPtr) PyvirStoragePool_Get(pyobj_pool);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virStoragePoolFree(pool);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virConnectNumOfDefinedInterfaces(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virConnectPtr conn;
    PyObject *pyobj_conn;

    if (!PyArg_ParseTuple(args, (char *)"O:virConnectNumOfDefinedInterfaces", &pyobj_conn))
        return(NULL);
    conn = (virConnectPtr) PyvirConnect_Get(pyobj_conn);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virConnectNumOfDefinedInterfaces(conn);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virDomainSnapshotLookupByName(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    virDomainSnapshotPtr c_retval;
    virDomainPtr domain;
    PyObject *pyobj_domain;
    char * name;
    unsigned int flags;

    if (!PyArg_ParseTuple(args, (char *)"Ozi:virDomainSnapshotLookupByName", &pyobj_domain, &name, &flags))
        return(NULL);
    domain = (virDomainPtr) PyvirDomain_Get(pyobj_domain);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virDomainSnapshotLookupByName(domain, name, flags);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_virDomainSnapshotPtrWrap((virDomainSnapshotPtr) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virConnResetLastError(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    virConnectPtr conn;
    PyObject *pyobj_conn;

    if (!PyArg_ParseTuple(args, (char *)"O:virConnResetLastError", &pyobj_conn))
        return(NULL);
    conn = (virConnectPtr) PyvirConnect_Get(pyobj_conn);
LIBVIRT_BEGIN_ALLOW_THREADS;

    virConnResetLastError(conn);
LIBVIRT_END_ALLOW_THREADS;
    Py_INCREF(Py_None);
    return(Py_None);
}

PyObject *
libvirt_virDomainResume(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virDomainPtr domain;
    PyObject *pyobj_domain;

    if (!PyArg_ParseTuple(args, (char *)"O:virDomainResume", &pyobj_domain))
        return(NULL);
    domain = (virDomainPtr) PyvirDomain_Get(pyobj_domain);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virDomainResume(domain);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virStreamNew(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    virStreamPtr c_retval;
    virConnectPtr conn;
    PyObject *pyobj_conn;
    unsigned int flags;

    if (!PyArg_ParseTuple(args, (char *)"Oi:virStreamNew", &pyobj_conn, &flags))
        return(NULL);
    conn = (virConnectPtr) PyvirConnect_Get(pyobj_conn);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virStreamNew(conn, flags);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_virStreamPtrWrap((virStreamPtr) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virConnectGetHostname(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    char * c_retval;
    virConnectPtr conn;
    PyObject *pyobj_conn;

    if (!PyArg_ParseTuple(args, (char *)"O:virConnectGetHostname", &pyobj_conn))
        return(NULL);
    conn = (virConnectPtr) PyvirConnect_Get(pyobj_conn);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virConnectGetHostname(conn);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_charPtrWrap((char *) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virDomainIsActive(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virDomainPtr dom;
    PyObject *pyobj_dom;

    if (!PyArg_ParseTuple(args, (char *)"O:virDomainIsActive", &pyobj_dom))
        return(NULL);
    dom = (virDomainPtr) PyvirDomain_Get(pyobj_dom);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virDomainIsActive(dom);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virNodeDeviceGetParent(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    const char * c_retval;
    virNodeDevicePtr dev;
    PyObject *pyobj_dev;

    if (!PyArg_ParseTuple(args, (char *)"O:virNodeDeviceGetParent", &pyobj_dev))
        return(NULL);
    dev = (virNodeDevicePtr) PyvirNodeDevice_Get(pyobj_dev);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virNodeDeviceGetParent(dev);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_charPtrConstWrap((const char *) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virDomainDetachDeviceFlags(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virDomainPtr domain;
    PyObject *pyobj_domain;
    char * xml;
    unsigned int flags;

    if (!PyArg_ParseTuple(args, (char *)"Ozi:virDomainDetachDeviceFlags", &pyobj_domain, &xml, &flags))
        return(NULL);
    domain = (virDomainPtr) PyvirDomain_Get(pyobj_domain);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virDomainDetachDeviceFlags(domain, xml, flags);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virDomainGetName(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    const char * c_retval;
    virDomainPtr domain;
    PyObject *pyobj_domain;

    if (!PyArg_ParseTuple(args, (char *)"O:virDomainGetName", &pyobj_domain))
        return(NULL);
    domain = (virDomainPtr) PyvirDomain_Get(pyobj_domain);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virDomainGetName(domain);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_charPtrConstWrap((const char *) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virNWFilterGetXMLDesc(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    char * c_retval;
    virNWFilterPtr nwfilter;
    PyObject *pyobj_nwfilter;
    int flags;

    if (!PyArg_ParseTuple(args, (char *)"Oi:virNWFilterGetXMLDesc", &pyobj_nwfilter, &flags))
        return(NULL);
    nwfilter = (virNWFilterPtr) PyvirNWFilter_Get(pyobj_nwfilter);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virNWFilterGetXMLDesc(nwfilter, flags);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_charPtrWrap((char *) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virNetworkGetXMLDesc(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    char * c_retval;
    virNetworkPtr network;
    PyObject *pyobj_network;
    int flags;

    if (!PyArg_ParseTuple(args, (char *)"Oi:virNetworkGetXMLDesc", &pyobj_network, &flags))
        return(NULL);
    network = (virNetworkPtr) PyvirNetwork_Get(pyobj_network);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virNetworkGetXMLDesc(network, flags);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_charPtrWrap((char *) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virConnectNumOfStoragePools(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virConnectPtr conn;
    PyObject *pyobj_conn;

    if (!PyArg_ParseTuple(args, (char *)"O:virConnectNumOfStoragePools", &pyobj_conn))
        return(NULL);
    conn = (virConnectPtr) PyvirConnect_Get(pyobj_conn);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virConnectNumOfStoragePools(conn);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virStoragePoolIsPersistent(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virStoragePoolPtr pool;
    PyObject *pyobj_pool;

    if (!PyArg_ParseTuple(args, (char *)"O:virStoragePoolIsPersistent", &pyobj_pool))
        return(NULL);
    pool = (virStoragePoolPtr) PyvirStoragePool_Get(pyobj_pool);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virStoragePoolIsPersistent(pool);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virStoragePoolNumOfVolumes(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virStoragePoolPtr pool;
    PyObject *pyobj_pool;

    if (!PyArg_ParseTuple(args, (char *)"O:virStoragePoolNumOfVolumes", &pyobj_pool))
        return(NULL);
    pool = (virStoragePoolPtr) PyvirStoragePool_Get(pyobj_pool);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virStoragePoolNumOfVolumes(pool);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virNetworkGetName(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    const char * c_retval;
    virNetworkPtr network;
    PyObject *pyobj_network;

    if (!PyArg_ParseTuple(args, (char *)"O:virNetworkGetName", &pyobj_network))
        return(NULL);
    network = (virNetworkPtr) PyvirNetwork_Get(pyobj_network);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virNetworkGetName(network);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_charPtrConstWrap((const char *) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virSecretGetXMLDesc(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    char * c_retval;
    virSecretPtr secret;
    PyObject *pyobj_secret;
    unsigned int flags;

    if (!PyArg_ParseTuple(args, (char *)"Oi:virSecretGetXMLDesc", &pyobj_secret, &flags))
        return(NULL);
    secret = (virSecretPtr) PyvirSecret_Get(pyobj_secret);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virSecretGetXMLDesc(secret, flags);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_charPtrWrap((char *) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virConnectGetCapabilities(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    char * c_retval;
    virConnectPtr conn;
    PyObject *pyobj_conn;

    if (!PyArg_ParseTuple(args, (char *)"O:virConnectGetCapabilities", &pyobj_conn))
        return(NULL);
    conn = (virConnectPtr) PyvirConnect_Get(pyobj_conn);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virConnectGetCapabilities(conn);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_charPtrWrap((char *) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virDomainLookupByName(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    virDomainPtr c_retval;
    virConnectPtr conn;
    PyObject *pyobj_conn;
    char * name;

    if (!PyArg_ParseTuple(args, (char *)"Oz:virDomainLookupByName", &pyobj_conn, &name))
        return(NULL);
    conn = (virConnectPtr) PyvirConnect_Get(pyobj_conn);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virDomainLookupByName(conn, name);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_virDomainPtrWrap((virDomainPtr) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virConnectFindStoragePoolSources(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    char * c_retval;
    virConnectPtr conn;
    PyObject *pyobj_conn;
    char * type;
    char * srcSpec;
    unsigned int flags;

    if (!PyArg_ParseTuple(args, (char *)"Ozzi:virConnectFindStoragePoolSources", &pyobj_conn, &type, &srcSpec, &flags))
        return(NULL);
    conn = (virConnectPtr) PyvirConnect_Get(pyobj_conn);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virConnectFindStoragePoolSources(conn, type, srcSpec, flags);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_charPtrWrap((char *) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virStorageVolFree(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virStorageVolPtr vol;
    PyObject *pyobj_vol;

    if (!PyArg_ParseTuple(args, (char *)"O:virStorageVolFree", &pyobj_vol))
        return(NULL);
    vol = (virStorageVolPtr) PyvirStorageVol_Get(pyobj_vol);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virStorageVolFree(vol);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virDomainRestore(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virConnectPtr conn;
    PyObject *pyobj_conn;
    char * frm;

    if (!PyArg_ParseTuple(args, (char *)"Oz:virDomainRestore", &pyobj_conn, &frm))
        return(NULL);
    conn = (virConnectPtr) PyvirConnect_Get(pyobj_conn);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virDomainRestore(conn, frm);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virStorageVolGetPath(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    char * c_retval;
    virStorageVolPtr vol;
    PyObject *pyobj_vol;

    if (!PyArg_ParseTuple(args, (char *)"O:virStorageVolGetPath", &pyobj_vol))
        return(NULL);
    vol = (virStorageVolPtr) PyvirStorageVol_Get(pyobj_vol);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virStorageVolGetPath(vol);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_charPtrWrap((char *) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virNetworkLookupByUUIDString(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    virNetworkPtr c_retval;
    virConnectPtr conn;
    PyObject *pyobj_conn;
    char * uuidstr;

    if (!PyArg_ParseTuple(args, (char *)"Oz:virNetworkLookupByUUIDString", &pyobj_conn, &uuidstr))
        return(NULL);
    conn = (virConnectPtr) PyvirConnect_Get(pyobj_conn);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virNetworkLookupByUUIDString(conn, uuidstr);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_virNetworkPtrWrap((virNetworkPtr) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virDomainLookupByID(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    virDomainPtr c_retval;
    virConnectPtr conn;
    PyObject *pyobj_conn;
    int id;

    if (!PyArg_ParseTuple(args, (char *)"Oi:virDomainLookupByID", &pyobj_conn, &id))
        return(NULL);
    conn = (virConnectPtr) PyvirConnect_Get(pyobj_conn);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virDomainLookupByID(conn, id);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_virDomainPtrWrap((virDomainPtr) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virNWFilterUndefine(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virNWFilterPtr nwfilter;
    PyObject *pyobj_nwfilter;

    if (!PyArg_ParseTuple(args, (char *)"O:virNWFilterUndefine", &pyobj_nwfilter))
        return(NULL);
    nwfilter = (virNWFilterPtr) PyvirNWFilter_Get(pyobj_nwfilter);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virNWFilterUndefine(nwfilter);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virSecretUndefine(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virSecretPtr secret;
    PyObject *pyobj_secret;

    if (!PyArg_ParseTuple(args, (char *)"O:virSecretUndefine", &pyobj_secret))
        return(NULL);
    secret = (virSecretPtr) PyvirSecret_Get(pyobj_secret);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virSecretUndefine(secret);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virConnectGetSysinfo(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    char * c_retval;
    virConnectPtr conn;
    PyObject *pyobj_conn;
    unsigned int flags;

    if (!PyArg_ParseTuple(args, (char *)"Oi:virConnectGetSysinfo", &pyobj_conn, &flags))
        return(NULL);
    conn = (virConnectPtr) PyvirConnect_Get(pyobj_conn);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virConnectGetSysinfo(conn, flags);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_charPtrWrap((char *) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virNetworkCreateXML(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    virNetworkPtr c_retval;
    virConnectPtr conn;
    PyObject *pyobj_conn;
    char * xmlDesc;

    if (!PyArg_ParseTuple(args, (char *)"Oz:virNetworkCreateXML", &pyobj_conn, &xmlDesc))
        return(NULL);
    conn = (virConnectPtr) PyvirConnect_Get(pyobj_conn);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virNetworkCreateXML(conn, xmlDesc);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_virNetworkPtrWrap((virNetworkPtr) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virNetworkUndefine(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virNetworkPtr network;
    PyObject *pyobj_network;

    if (!PyArg_ParseTuple(args, (char *)"O:virNetworkUndefine", &pyobj_network))
        return(NULL);
    network = (virNetworkPtr) PyvirNetwork_Get(pyobj_network);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virNetworkUndefine(network);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virDomainMigrateToURI(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virDomainPtr domain;
    PyObject *pyobj_domain;
    char * duri;
    unsigned long flags;
    char * dname;
    unsigned long bandwidth;

    if (!PyArg_ParseTuple(args, (char *)"Ozlzl:virDomainMigrateToURI", &pyobj_domain, &duri, &flags, &dname, &bandwidth))
        return(NULL);
    domain = (virDomainPtr) PyvirDomain_Get(pyobj_domain);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virDomainMigrateToURI(domain, duri, flags, dname, bandwidth);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virNodeDeviceLookupByName(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    virNodeDevicePtr c_retval;
    virConnectPtr conn;
    PyObject *pyobj_conn;
    char * name;

    if (!PyArg_ParseTuple(args, (char *)"Oz:virNodeDeviceLookupByName", &pyobj_conn, &name))
        return(NULL);
    conn = (virConnectPtr) PyvirConnect_Get(pyobj_conn);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virNodeDeviceLookupByName(conn, name);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_virNodeDevicePtrWrap((virNodeDevicePtr) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virDomainGetVcpusFlags(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virDomainPtr domain;
    PyObject *pyobj_domain;
    unsigned int flags;

    if (!PyArg_ParseTuple(args, (char *)"Oi:virDomainGetVcpusFlags", &pyobj_domain, &flags))
        return(NULL);
    domain = (virDomainPtr) PyvirDomain_Get(pyobj_domain);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virDomainGetVcpusFlags(domain, flags);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virInterfaceGetMACString(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    const char * c_retval;
    virInterfacePtr iface;
    PyObject *pyobj_iface;

    if (!PyArg_ParseTuple(args, (char *)"O:virInterfaceGetMACString", &pyobj_iface))
        return(NULL);
    iface = (virInterfacePtr) PyvirInterface_Get(pyobj_iface);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virInterfaceGetMACString(iface);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_charPtrConstWrap((const char *) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virConnectNumOfDomains(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virConnectPtr conn;
    PyObject *pyobj_conn;

    if (!PyArg_ParseTuple(args, (char *)"O:virConnectNumOfDomains", &pyobj_conn))
        return(NULL);
    conn = (virConnectPtr) PyvirConnect_Get(pyobj_conn);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virConnectNumOfDomains(conn);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virConnectNumOfNWFilters(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virConnectPtr conn;
    PyObject *pyobj_conn;

    if (!PyArg_ParseTuple(args, (char *)"O:virConnectNumOfNWFilters", &pyobj_conn))
        return(NULL);
    conn = (virConnectPtr) PyvirConnect_Get(pyobj_conn);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virConnectNumOfNWFilters(conn);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virConnectNumOfDefinedDomains(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virConnectPtr conn;
    PyObject *pyobj_conn;

    if (!PyArg_ParseTuple(args, (char *)"O:virConnectNumOfDefinedDomains", &pyobj_conn))
        return(NULL);
    conn = (virConnectPtr) PyvirConnect_Get(pyobj_conn);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virConnectNumOfDefinedDomains(conn);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virInitialize(PyObject *self ATTRIBUTE_UNUSED, PyObject *args ATTRIBUTE_UNUSED) {
    PyObject *py_retval;
    int c_retval;
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virInitialize();
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virDomainAttachDeviceFlags(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virDomainPtr domain;
    PyObject *pyobj_domain;
    char * xml;
    unsigned int flags;

    if (!PyArg_ParseTuple(args, (char *)"Ozi:virDomainAttachDeviceFlags", &pyobj_domain, &xml, &flags))
        return(NULL);
    domain = (virDomainPtr) PyvirDomain_Get(pyobj_domain);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virDomainAttachDeviceFlags(domain, xml, flags);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virStorageVolCreateXMLFrom(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    virStorageVolPtr c_retval;
    virStoragePoolPtr pool;
    PyObject *pyobj_pool;
    char * xmldesc;
    virStorageVolPtr clonevol;
    PyObject *pyobj_clonevol;
    unsigned int flags;

    if (!PyArg_ParseTuple(args, (char *)"OzOi:virStorageVolCreateXMLFrom", &pyobj_pool, &xmldesc, &pyobj_clonevol, &flags))
        return(NULL);
    pool = (virStoragePoolPtr) PyvirStoragePool_Get(pyobj_pool);
    clonevol = (virStorageVolPtr) PyvirStorageVol_Get(pyobj_clonevol);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virStorageVolCreateXMLFrom(pool, xmldesc, clonevol, flags);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_virStorageVolPtrWrap((virStorageVolPtr) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virConnectClose(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virConnectPtr conn;
    PyObject *pyobj_conn;

    if (!PyArg_ParseTuple(args, (char *)"O:virConnectClose", &pyobj_conn))
        return(NULL);
    conn = (virConnectPtr) PyvirConnect_Get(pyobj_conn);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virConnectClose(conn);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virDomainReboot(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virDomainPtr domain;
    PyObject *pyobj_domain;
    unsigned int flags;

    if (!PyArg_ParseTuple(args, (char *)"Oi:virDomainReboot", &pyobj_domain, &flags))
        return(NULL);
    domain = (virDomainPtr) PyvirDomain_Get(pyobj_domain);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virDomainReboot(domain, flags);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virNetworkLookupByName(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    virNetworkPtr c_retval;
    virConnectPtr conn;
    PyObject *pyobj_conn;
    char * name;

    if (!PyArg_ParseTuple(args, (char *)"Oz:virNetworkLookupByName", &pyobj_conn, &name))
        return(NULL);
    conn = (virConnectPtr) PyvirConnect_Get(pyobj_conn);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virNetworkLookupByName(conn, name);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_virNetworkPtrWrap((virNetworkPtr) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virDomainGetMaxVcpus(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virDomainPtr domain;
    PyObject *pyobj_domain;

    if (!PyArg_ParseTuple(args, (char *)"O:virDomainGetMaxVcpus", &pyobj_domain))
        return(NULL);
    domain = (virDomainPtr) PyvirDomain_Get(pyobj_domain);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virDomainGetMaxVcpus(domain);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virDomainIsPersistent(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virDomainPtr dom;
    PyObject *pyobj_dom;

    if (!PyArg_ParseTuple(args, (char *)"O:virDomainIsPersistent", &pyobj_dom))
        return(NULL);
    dom = (virDomainPtr) PyvirDomain_Get(pyobj_dom);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virDomainIsPersistent(dom);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virStreamEventUpdateCallback(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virStreamPtr stream;
    PyObject *pyobj_stream;
    int events;

    if (!PyArg_ParseTuple(args, (char *)"Oi:virStreamEventUpdateCallback", &pyobj_stream, &events))
        return(NULL);
    stream = (virStreamPtr) PyvirStream_Get(pyobj_stream);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virStreamEventUpdateCallback(stream, events);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virNWFilterDefineXML(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    virNWFilterPtr c_retval;
    virConnectPtr conn;
    PyObject *pyobj_conn;
    char * xmlDesc;

    if (!PyArg_ParseTuple(args, (char *)"Oz:virNWFilterDefineXML", &pyobj_conn, &xmlDesc))
        return(NULL);
    conn = (virConnectPtr) PyvirConnect_Get(pyobj_conn);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virNWFilterDefineXML(conn, xmlDesc);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_virNWFilterPtrWrap((virNWFilterPtr) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virNodeDeviceReset(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virNodeDevicePtr dev;
    PyObject *pyobj_dev;

    if (!PyArg_ParseTuple(args, (char *)"O:virNodeDeviceReset", &pyobj_dev))
        return(NULL);
    dev = (virNodeDevicePtr) PyvirNodeDevice_Get(pyobj_dev);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virNodeDeviceReset(dev);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virDomainDetachDevice(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virDomainPtr domain;
    PyObject *pyobj_domain;
    char * xml;

    if (!PyArg_ParseTuple(args, (char *)"Oz:virDomainDetachDevice", &pyobj_domain, &xml))
        return(NULL);
    domain = (virDomainPtr) PyvirDomain_Get(pyobj_domain);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virDomainDetachDevice(domain, xml);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virSecretGetUsageID(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    const char * c_retval;
    virSecretPtr secret;
    PyObject *pyobj_secret;

    if (!PyArg_ParseTuple(args, (char *)"O:virSecretGetUsageID", &pyobj_secret))
        return(NULL);
    secret = (virSecretPtr) PyvirSecret_Get(pyobj_secret);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virSecretGetUsageID(secret);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_charPtrConstWrap((const char *) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virNodeDeviceReAttach(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virNodeDevicePtr dev;
    PyObject *pyobj_dev;

    if (!PyArg_ParseTuple(args, (char *)"O:virNodeDeviceReAttach", &pyobj_dev))
        return(NULL);
    dev = (virNodeDevicePtr) PyvirNodeDevice_Get(pyobj_dev);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virNodeDeviceReAttach(dev);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virDomainSnapshotFree(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virDomainSnapshotPtr snapshot;
    PyObject *pyobj_snapshot;

    if (!PyArg_ParseTuple(args, (char *)"O:virDomainSnapshotFree", &pyobj_snapshot))
        return(NULL);
    snapshot = (virDomainSnapshotPtr) PyvirDomainSnapshot_Get(pyobj_snapshot);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virDomainSnapshotFree(snapshot);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virStoragePoolUndefine(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virStoragePoolPtr pool;
    PyObject *pyobj_pool;

    if (!PyArg_ParseTuple(args, (char *)"O:virStoragePoolUndefine", &pyobj_pool))
        return(NULL);
    pool = (virStoragePoolPtr) PyvirStoragePool_Get(pyobj_pool);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virStoragePoolUndefine(pool);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virNetworkGetBridgeName(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    char * c_retval;
    virNetworkPtr network;
    PyObject *pyobj_network;

    if (!PyArg_ParseTuple(args, (char *)"O:virNetworkGetBridgeName", &pyobj_network))
        return(NULL);
    network = (virNetworkPtr) PyvirNetwork_Get(pyobj_network);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virNetworkGetBridgeName(network);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_charPtrWrap((char *) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virDomainAttachDevice(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virDomainPtr domain;
    PyObject *pyobj_domain;
    char * xml;

    if (!PyArg_ParseTuple(args, (char *)"Oz:virDomainAttachDevice", &pyobj_domain, &xml))
        return(NULL);
    domain = (virDomainPtr) PyvirDomain_Get(pyobj_domain);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virDomainAttachDevice(domain, xml);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virConnectGetURI(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    char * c_retval;
    virConnectPtr conn;
    PyObject *pyobj_conn;

    if (!PyArg_ParseTuple(args, (char *)"O:virConnectGetURI", &pyobj_conn))
        return(NULL);
    conn = (virConnectPtr) PyvirConnect_Get(pyobj_conn);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virConnectGetURI(conn);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_charPtrWrap((char *) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virConnectOpenReadOnly(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    virConnectPtr c_retval;
    char * name;

    if (!PyArg_ParseTuple(args, (char *)"z:virConnectOpenReadOnly", &name))
        return(NULL);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virConnectOpenReadOnly(name);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_virConnectPtrWrap((virConnectPtr) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virNetworkFree(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virNetworkPtr network;
    PyObject *pyobj_network;

    if (!PyArg_ParseTuple(args, (char *)"O:virNetworkFree", &pyobj_network))
        return(NULL);
    network = (virNetworkPtr) PyvirNetwork_Get(pyobj_network);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virNetworkFree(network);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virInterfaceFree(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virInterfacePtr iface;
    PyObject *pyobj_iface;

    if (!PyArg_ParseTuple(args, (char *)"O:virInterfaceFree", &pyobj_iface))
        return(NULL);
    iface = (virInterfacePtr) PyvirInterface_Get(pyobj_iface);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virInterfaceFree(iface);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virNodeDeviceNumOfCaps(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virNodeDevicePtr dev;
    PyObject *pyobj_dev;

    if (!PyArg_ParseTuple(args, (char *)"O:virNodeDeviceNumOfCaps", &pyobj_dev))
        return(NULL);
    dev = (virNodeDevicePtr) PyvirNodeDevice_Get(pyobj_dev);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virNodeDeviceNumOfCaps(dev);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virInterfaceLookupByMACString(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    virInterfacePtr c_retval;
    virConnectPtr conn;
    PyObject *pyobj_conn;
    char * macstr;

    if (!PyArg_ParseTuple(args, (char *)"Oz:virInterfaceLookupByMACString", &pyobj_conn, &macstr))
        return(NULL);
    conn = (virConnectPtr) PyvirConnect_Get(pyobj_conn);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virInterfaceLookupByMACString(conn, macstr);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_virInterfacePtrWrap((virInterfacePtr) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virNodeGetFreeMemory(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    unsigned long long c_retval;
    virConnectPtr conn;
    PyObject *pyobj_conn;

    if (!PyArg_ParseTuple(args, (char *)"O:virNodeGetFreeMemory", &pyobj_conn))
        return(NULL);
    conn = (virConnectPtr) PyvirConnect_Get(pyobj_conn);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virNodeGetFreeMemory(conn);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_longlongWrap((long long) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virDomainHasCurrentSnapshot(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virDomainPtr domain;
    PyObject *pyobj_domain;
    unsigned int flags;

    if (!PyArg_ParseTuple(args, (char *)"Oi:virDomainHasCurrentSnapshot", &pyobj_domain, &flags))
        return(NULL);
    domain = (virDomainPtr) PyvirDomain_Get(pyobj_domain);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virDomainHasCurrentSnapshot(domain, flags);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virInterfaceDestroy(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virInterfacePtr iface;
    PyObject *pyobj_iface;
    unsigned int flags;

    if (!PyArg_ParseTuple(args, (char *)"Oi:virInterfaceDestroy", &pyobj_iface, &flags))
        return(NULL);
    iface = (virInterfacePtr) PyvirInterface_Get(pyobj_iface);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virInterfaceDestroy(iface, flags);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virNodeNumOfDevices(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virConnectPtr conn;
    PyObject *pyobj_conn;
    char * cap;
    unsigned int flags;

    if (!PyArg_ParseTuple(args, (char *)"Ozi:virNodeNumOfDevices", &pyobj_conn, &cap, &flags))
        return(NULL);
    conn = (virConnectPtr) PyvirConnect_Get(pyobj_conn);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virNodeNumOfDevices(conn, cap, flags);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virStoragePoolDestroy(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virStoragePoolPtr pool;
    PyObject *pyobj_pool;

    if (!PyArg_ParseTuple(args, (char *)"O:virStoragePoolDestroy", &pyobj_pool))
        return(NULL);
    pool = (virStoragePoolPtr) PyvirStoragePool_Get(pyobj_pool);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virStoragePoolDestroy(pool);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virStoragePoolLookupByVolume(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    virStoragePoolPtr c_retval;
    virStorageVolPtr vol;
    PyObject *pyobj_vol;

    if (!PyArg_ParseTuple(args, (char *)"O:virStoragePoolLookupByVolume", &pyobj_vol))
        return(NULL);
    vol = (virStorageVolPtr) PyvirStorageVol_Get(pyobj_vol);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virStoragePoolLookupByVolume(vol);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_virStoragePoolPtrWrap((virStoragePoolPtr) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virDomainGetOSType(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    char * c_retval;
    virDomainPtr domain;
    PyObject *pyobj_domain;

    if (!PyArg_ParseTuple(args, (char *)"O:virDomainGetOSType", &pyobj_domain))
        return(NULL);
    domain = (virDomainPtr) PyvirDomain_Get(pyobj_domain);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virDomainGetOSType(domain);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_charPtrWrap((char *) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virDomainSetAutostart(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virDomainPtr domain;
    PyObject *pyobj_domain;
    int autostart;

    if (!PyArg_ParseTuple(args, (char *)"Oi:virDomainSetAutostart", &pyobj_domain, &autostart))
        return(NULL);
    domain = (virDomainPtr) PyvirDomain_Get(pyobj_domain);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virDomainSetAutostart(domain, autostart);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virConnectNumOfDefinedStoragePools(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virConnectPtr conn;
    PyObject *pyobj_conn;

    if (!PyArg_ParseTuple(args, (char *)"O:virConnectNumOfDefinedStoragePools", &pyobj_conn))
        return(NULL);
    conn = (virConnectPtr) PyvirConnect_Get(pyobj_conn);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virConnectNumOfDefinedStoragePools(conn);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virSecretFree(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virSecretPtr secret;
    PyObject *pyobj_secret;

    if (!PyArg_ParseTuple(args, (char *)"O:virSecretFree", &pyobj_secret))
        return(NULL);
    secret = (virSecretPtr) PyvirSecret_Get(pyobj_secret);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virSecretFree(secret);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virStreamFinish(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virStreamPtr stream;
    PyObject *pyobj_stream;

    if (!PyArg_ParseTuple(args, (char *)"O:virStreamFinish", &pyobj_stream))
        return(NULL);
    stream = (virStreamPtr) PyvirStream_Get(pyobj_stream);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virStreamFinish(stream);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virNetworkIsActive(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virNetworkPtr net;
    PyObject *pyobj_net;

    if (!PyArg_ParseTuple(args, (char *)"O:virNetworkIsActive", &pyobj_net))
        return(NULL);
    net = (virNetworkPtr) PyvirNetwork_Get(pyobj_net);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virNetworkIsActive(net);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virConnectOpen(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    virConnectPtr c_retval;
    char * name;

    if (!PyArg_ParseTuple(args, (char *)"z:virConnectOpen", &name))
        return(NULL);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virConnectOpen(name);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_virConnectPtrWrap((virConnectPtr) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virDomainCreateXML(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    virDomainPtr c_retval;
    virConnectPtr conn;
    PyObject *pyobj_conn;
    char * xmlDesc;
    unsigned int flags;

    if (!PyArg_ParseTuple(args, (char *)"Ozi:virDomainCreateXML", &pyobj_conn, &xmlDesc, &flags))
        return(NULL);
    conn = (virConnectPtr) PyvirConnect_Get(pyobj_conn);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virDomainCreateXML(conn, xmlDesc, flags);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_virDomainPtrWrap((virDomainPtr) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virStoragePoolRefresh(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virStoragePoolPtr pool;
    PyObject *pyobj_pool;
    unsigned int flags;

    if (!PyArg_ParseTuple(args, (char *)"Oi:virStoragePoolRefresh", &pyobj_pool, &flags))
        return(NULL);
    pool = (virStoragePoolPtr) PyvirStoragePool_Get(pyobj_pool);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virStoragePoolRefresh(pool, flags);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virInterfaceIsActive(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virInterfacePtr iface;
    PyObject *pyobj_iface;

    if (!PyArg_ParseTuple(args, (char *)"O:virInterfaceIsActive", &pyobj_iface))
        return(NULL);
    iface = (virInterfacePtr) PyvirInterface_Get(pyobj_iface);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virInterfaceIsActive(iface);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virStoragePoolBuild(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virStoragePoolPtr pool;
    PyObject *pyobj_pool;
    unsigned int flags;

    if (!PyArg_ParseTuple(args, (char *)"Oi:virStoragePoolBuild", &pyobj_pool, &flags))
        return(NULL);
    pool = (virStoragePoolPtr) PyvirStoragePool_Get(pyobj_pool);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virStoragePoolBuild(pool, flags);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virInterfaceUndefine(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virInterfacePtr iface;
    PyObject *pyobj_iface;

    if (!PyArg_ParseTuple(args, (char *)"O:virInterfaceUndefine", &pyobj_iface))
        return(NULL);
    iface = (virInterfacePtr) PyvirInterface_Get(pyobj_iface);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virInterfaceUndefine(iface);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virDomainSetVcpus(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virDomainPtr domain;
    PyObject *pyobj_domain;
    unsigned int nvcpus;

    if (!PyArg_ParseTuple(args, (char *)"Oi:virDomainSetVcpus", &pyobj_domain, &nvcpus))
        return(NULL);
    domain = (virDomainPtr) PyvirDomain_Get(pyobj_domain);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virDomainSetVcpus(domain, nvcpus);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virConnectNumOfInterfaces(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virConnectPtr conn;
    PyObject *pyobj_conn;

    if (!PyArg_ParseTuple(args, (char *)"O:virConnectNumOfInterfaces", &pyobj_conn))
        return(NULL);
    conn = (virConnectPtr) PyvirConnect_Get(pyobj_conn);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virConnectNumOfInterfaces(conn);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virDomainGetID(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    unsigned int c_retval;
    virDomainPtr domain;
    PyObject *pyobj_domain;

    if (!PyArg_ParseTuple(args, (char *)"O:virDomainGetID", &pyobj_domain))
        return(NULL);
    domain = (virDomainPtr) PyvirDomain_Get(pyobj_domain);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virDomainGetID(domain);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virNWFilterLookupByUUIDString(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    virNWFilterPtr c_retval;
    virConnectPtr conn;
    PyObject *pyobj_conn;
    char * uuidstr;

    if (!PyArg_ParseTuple(args, (char *)"Oz:virNWFilterLookupByUUIDString", &pyobj_conn, &uuidstr))
        return(NULL);
    conn = (virConnectPtr) PyvirConnect_Get(pyobj_conn);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virNWFilterLookupByUUIDString(conn, uuidstr);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_virNWFilterPtrWrap((virNWFilterPtr) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virDomainSnapshotCreateXML(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    virDomainSnapshotPtr c_retval;
    virDomainPtr domain;
    PyObject *pyobj_domain;
    char * xmlDesc;
    unsigned int flags;

    if (!PyArg_ParseTuple(args, (char *)"Ozi:virDomainSnapshotCreateXML", &pyobj_domain, &xmlDesc, &flags))
        return(NULL);
    domain = (virDomainPtr) PyvirDomain_Get(pyobj_domain);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virDomainSnapshotCreateXML(domain, xmlDesc, flags);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_virDomainSnapshotPtrWrap((virDomainSnapshotPtr) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virConnectIsEncrypted(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virConnectPtr conn;
    PyObject *pyobj_conn;

    if (!PyArg_ParseTuple(args, (char *)"O:virConnectIsEncrypted", &pyobj_conn))
        return(NULL);
    conn = (virConnectPtr) PyvirConnect_Get(pyobj_conn);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virConnectIsEncrypted(conn);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virInterfaceLookupByName(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    virInterfacePtr c_retval;
    virConnectPtr conn;
    PyObject *pyobj_conn;
    char * name;

    if (!PyArg_ParseTuple(args, (char *)"Oz:virInterfaceLookupByName", &pyobj_conn, &name))
        return(NULL);
    conn = (virConnectPtr) PyvirConnect_Get(pyobj_conn);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virInterfaceLookupByName(conn, name);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_virInterfacePtrWrap((virInterfacePtr) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virDomainHasManagedSaveImage(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virDomainPtr dom;
    PyObject *pyobj_dom;
    unsigned int flags;

    if (!PyArg_ParseTuple(args, (char *)"Oi:virDomainHasManagedSaveImage", &pyobj_dom, &flags))
        return(NULL);
    dom = (virDomainPtr) PyvirDomain_Get(pyobj_dom);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virDomainHasManagedSaveImage(dom, flags);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virDomainSnapshotNum(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    virDomainPtr domain;
    PyObject *pyobj_domain;
    unsigned int flags;

    if (!PyArg_ParseTuple(args, (char *)"Oi:virDomainSnapshotNum", &pyobj_domain, &flags))
        return(NULL);
    domain = (virDomainPtr) PyvirDomain_Get(pyobj_domain);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virDomainSnapshotNum(domain, flags);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_intWrap((int) c_retval);
    return(py_retval);
}

PyObject *
libvirt_virStorageVolLookupByKey(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    virStorageVolPtr c_retval;
    virConnectPtr conn;
    PyObject *pyobj_conn;
    char * key;

    if (!PyArg_ParseTuple(args, (char *)"Oz:virStorageVolLookupByKey", &pyobj_conn, &key))
        return(NULL);
    conn = (virConnectPtr) PyvirConnect_Get(pyobj_conn);
LIBVIRT_BEGIN_ALLOW_THREADS;

    c_retval = virStorageVolLookupByKey(conn, key);
LIBVIRT_END_ALLOW_THREADS;
    py_retval = libvirt_virStorageVolPtrWrap((virStorageVolPtr) c_retval);
    return(py_retval);
}

