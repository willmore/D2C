#!/usr/bin/env python
#
# WARNING WARNING WARNING WARNING
#
# This file is automatically written by generator.py. Any changes
# made here will be lost.
#
# To change the manually written methods edit libvirt-override.py
# To change the automatically written methods edit generator.py
#
# WARNING WARNING WARNING WARNING
#
#
# Manually written part of python bindings for libvirt
#

# Specify -i commandline option after python was started
if __name__ == "__main__":
    import os
    os.environ["PYTHONINSPECT"] = "1"

# On cygwin, the DLL is called cygvirtmod.dll
try:
    import libvirtmod
except ImportError, lib_e:
    try:
        import cygvirtmod as libvirtmod
    except ImportError, cyg_e:
        if str(cyg_e).count("No module named"):
            raise lib_e

import types

# The root of all libvirt errors.
class libvirtError(Exception):
    def __init__(self, defmsg, conn=None, dom=None, net=None, pool=None, vol=None):

        # Never call virConnGetLastError().
        # virGetLastError() is now thread local
        err = virGetLastError()
        if err is None:
            msg = defmsg
        else:
            msg = err[2]

        Exception.__init__(self, msg)

        self.err = err

    def get_error_code(self):
        if self.err is None:
            return None
        return self.err[0]

    def get_error_domain(self):
        if self.err is None:
            return None
        return self.err[1]

    def get_error_message(self):
        if self.err is None:
            return None
        return self.err[2]

    def get_error_level(self):
        if self.err is None:
            return None
        return self.err[3]

    def get_str1(self):
        if self.err is None:
            return None
        return self.err[4]

    def get_str2(self):
        if self.err is None:
            return None
        return self.err[5]

    def get_str3(self):
        if self.err is None:
            return None
        return self.err[6]

    def get_int1(self):
        if self.err is None:
            return None
        return self.err[7]

    def get_int2(self):
        if self.err is None:
            return None
        return self.err[8]

#
# register the libvirt global error handler
#
def registerErrorHandler(f, ctx):
    """Register a Python written function to for error reporting.
       The function is called back as f(ctx, error), with error
       being a list of information about the error being raised.
       Returns 1 in case of success."""
    return libvirtmod.virRegisterErrorHandler(f,ctx)

def openAuth(uri, auth, flags):
    ret = libvirtmod.virConnectOpenAuth(uri, auth, flags)
    if ret is None:raise libvirtError('virConnectOpenAuth() failed')
    return virConnect(_obj=ret)


#
# Return library version.
#
def getVersion (name = None):
    """If no name parameter is passed (or name is None) then the
    version of the libvirt library is returned as an integer.

    If a name is passed and it refers to a driver linked to the
    libvirt library, then this returns a tuple of (library version,
    driver version).

    If the name passed refers to a non-existent driver, then you
    will get the exception 'no support for hypervisor'.

    Versions numbers are integers: 1000000*major + 1000*minor + release."""
    if name is None:
        ret = libvirtmod.virGetVersion ();
    else:
        ret = libvirtmod.virGetVersion (name);
    if ret is None: raise libvirtError ("virGetVersion() failed")
    return ret


#
# Invoke an EventHandle callback
#
def eventInvokeHandleCallback (watch, fd, event, callback, opaque):
    """
    Invoke the Event Impl Handle Callback in C
    """
    libvirtmod.virEventInvokeHandleCallback(watch, fd, event, callback, opaque);

#
# Invoke an EventTimeout callback
#
def eventInvokeTimeoutCallback (timer, callback, opaque):
    """
    Invoke the Event Impl Timeout Callback in C
    """
    libvirtmod.virEventInvokeTimeoutCallback(timer, callback, opaque);
#
# WARNING WARNING WARNING WARNING
#
# Automatically written part of python bindings for libvirt
#
# WARNING WARNING WARNING WARNING
#
# Functions from module libvirt
#

def open(name):
    """This function should be called first to get a connection to the
    Hypervisor and xen store """
    ret = libvirtmod.virConnectOpen(name)
    if ret is None:raise libvirtError('virConnectOpen() failed')
    return virConnect(_obj=ret)

def openReadOnly(name):
    """This function should be called first to get a restricted connection to the
    library functionalities. The set of APIs usable are then restricted
    on the available methods to control the domains.
    
    See virConnectOpen for notes about environment variables which can
    have an effect on opening drivers """
    ret = libvirtmod.virConnectOpenReadOnly(name)
    if ret is None:raise libvirtError('virConnectOpenReadOnly() failed')
    return virConnect(_obj=ret)

def virEventRegisterImpl(addHandle, updateHandle, removeHandle, addTimeout, updateTimeout, removeTimeout):
    """Registers an event implementation """
    libvirtmod.virEventRegisterImpl(addHandle, updateHandle, removeHandle, addTimeout, updateTimeout, removeTimeout)

def virInitialize():
    """Initialize the library. It's better to call this routine at startup
    in multithreaded applications to avoid potential race when initializing
    the library. """
    ret = libvirtmod.virInitialize()
    if ret == -1: raise libvirtError ('virInitialize() failed')
    return ret

#
# Functions from module virterror
#

def virGetLastError():
    """Provide a pointer to the last error caught at the library level
    
    The error object is kept in thread local storage, so separate
    threads can safely access this concurrently. """
    ret = libvirtmod.virGetLastError()
    return ret

def virResetLastError():
    """Reset the last error caught at the library level.
    
    The error object is kept in thread local storage, so separate
    threads can safely access this concurrently, only resetting
    their own error object. """
    libvirtmod.virResetLastError()

class virDomain:
    def __init__(self, conn, _obj=None):
        self._conn = conn
        if _obj != None:self._o = _obj;return
        self._o = None

    def __del__(self):
        if self._o != None:
            libvirtmod.virDomainFree(self._o)
        self._o = None

    def connect(self):
        return self._conn

    #
    # virDomain functions from module libvirt
    #

    def ID(self):
        """Get the hypervisor ID number for the domain """
        ret = libvirtmod.virDomainGetID(self._o)
        return ret

    def OSType(self):
        """Get the type of domain operation system. """
        ret = libvirtmod.virDomainGetOSType(self._o)
        if ret is None: raise libvirtError ('virDomainGetOSType() failed', dom=self)
        return ret

    def XMLDesc(self, flags):
        """Provide an XML description of the domain. The description may be reused
        later to relaunch the domain with virDomainCreateXML(). """
        ret = libvirtmod.virDomainGetXMLDesc(self._o, flags)
        if ret is None: raise libvirtError ('virDomainGetXMLDesc() failed', dom=self)
        return ret

    def abortJob(self):
        """Requests that the current background job be aborted at the
        soonest opportunity. This will block until the job has
        either completed, or aborted. """
        ret = libvirtmod.virDomainAbortJob(self._o)
        if ret == -1: raise libvirtError ('virDomainAbortJob() failed', dom=self)
        return ret

    def attachDevice(self, xml):
        """Create a virtual device attachment to backend.  This function,
        having hotplug semantics, is only allowed on an active domain.
        
        For compatibility, this method can also be used to change the media
        in an existing CDROM/Floppy device, however, applications are
        recommended to use the virDomainUpdateDeviceFlag method instead. """
        ret = libvirtmod.virDomainAttachDevice(self._o, xml)
        if ret == -1: raise libvirtError ('virDomainAttachDevice() failed', dom=self)
        return ret

    def attachDeviceFlags(self, xml, flags):
        """Attach a virtual device to a domain, using the flags parameter
        to control how the device is attached.  VIR_DOMAIN_DEVICE_MODIFY_CURRENT
        specifies that the device allocation is made based on current domain
        state.  VIR_DOMAIN_DEVICE_MODIFY_LIVE specifies that the device shall be
        allocated to the active domain instance only and is not added to the
        persisted domain configuration.  VIR_DOMAIN_DEVICE_MODIFY_CONFIG
        specifies that the device shall be allocated to the persisted domain
        configuration only.  Note that the target hypervisor must return an
        error if unable to satisfy flags.  E.g. the hypervisor driver will
        return failure if LIVE is specified but it only supports modifying the
        persisted device allocation.
        
        For compatibility, this method can also be used to change the media
        in an existing CDROM/Floppy device, however, applications are
        recommended to use the virDomainUpdateDeviceFlag method instead. """
        ret = libvirtmod.virDomainAttachDeviceFlags(self._o, xml, flags)
        if ret == -1: raise libvirtError ('virDomainAttachDeviceFlags() failed', dom=self)
        return ret

    def blockPeek(self, path, offset, size, buffer, flags):
        """This function allows you to read the contents of a domain's
        disk device.
        
        Typical uses for this are to determine if the domain has
        written a Master Boot Record (indicating that the domain
        has completed installation), or to try to work out the state
        of the domain's filesystems.
        
        (Note that in the local case you might try to open the
        block device or file directly, but that won't work in the
        remote case, nor if you don't have sufficient permission.
        Hence the need for this call).
        
        'path' must be a device or file corresponding to the domain.
        In other words it must be the precise string returned in
        a <disk><source dev='...'/></disk> from
        virDomainGetXMLDesc.
        
        'offset' and 'size' represent an area which must lie entirely
        within the device or file.  'size' may be 0 to test if the
        call would succeed.
        
        'buffer' is the return buffer and must be at least 'size' bytes.
        
        NB. The remote driver imposes a 64K byte limit on 'size'.
        For your program to be able to work reliably over a remote
        connection you should split large requests to <= 65536 bytes. """
        ret = libvirtmod.virDomainBlockPeek(self._o, path, offset, size, buffer, flags)
        if ret == -1: raise libvirtError ('virDomainBlockPeek() failed', dom=self)
        return ret

    def coreDump(self, to, flags):
        """This method will dump the core of a domain on a given file for analysis.
        Note that for remote Xen Daemon the file path will be interpreted in
        the remote host. """
        ret = libvirtmod.virDomainCoreDump(self._o, to, flags)
        if ret == -1: raise libvirtError ('virDomainCoreDump() failed', dom=self)
        return ret

    def create(self):
        """Launch a defined domain. If the call succeeds the domain moves from the
        defined to the running domains pools. """
        ret = libvirtmod.virDomainCreate(self._o)
        if ret == -1: raise libvirtError ('virDomainCreate() failed', dom=self)
        return ret

    def createWithFlags(self, flags):
        """Launch a defined domain. If the call succeeds the domain moves from the
        defined to the running domains pools. """
        ret = libvirtmod.virDomainCreateWithFlags(self._o, flags)
        if ret == -1: raise libvirtError ('virDomainCreateWithFlags() failed', dom=self)
        return ret

    def destroy(self):
        """Destroy the domain object. The running instance is shutdown if not down
        already and all resources used by it are given back to the hypervisor. This
        does not free the associated virDomainPtr object.
        This function may require privileged access """
        ret = libvirtmod.virDomainDestroy(self._o)
        if ret == -1: raise libvirtError ('virDomainDestroy() failed', dom=self)
        return ret

    def detachDevice(self, xml):
        """Destroy a virtual device attachment to backend.  This function,
        having hot-unplug semantics, is only allowed on an active domain. """
        ret = libvirtmod.virDomainDetachDevice(self._o, xml)
        if ret == -1: raise libvirtError ('virDomainDetachDevice() failed', dom=self)
        return ret

    def detachDeviceFlags(self, xml, flags):
        """Detach a virtual device from a domain, using the flags parameter
        to control how the device is detached.  VIR_DOMAIN_DEVICE_MODIFY_CURRENT
        specifies that the device allocation is removed based on current domain
        state.  VIR_DOMAIN_DEVICE_MODIFY_LIVE specifies that the device shall be
        deallocated from the active domain instance only and is not from the
        persisted domain configuration.  VIR_DOMAIN_DEVICE_MODIFY_CONFIG
        specifies that the device shall be deallocated from the persisted domain
        configuration only.  Note that the target hypervisor must return an
        error if unable to satisfy flags.  E.g. the hypervisor driver will
        return failure if LIVE is specified but it only supports removing the
        persisted device allocation. """
        ret = libvirtmod.virDomainDetachDeviceFlags(self._o, xml, flags)
        if ret == -1: raise libvirtError ('virDomainDetachDeviceFlags() failed', dom=self)
        return ret

    def hasCurrentSnapshot(self, flags):
        """Determine if the domain has a current snapshot. """
        ret = libvirtmod.virDomainHasCurrentSnapshot(self._o, flags)
        if ret == -1: raise libvirtError ('virDomainHasCurrentSnapshot() failed', dom=self)
        return ret

    def hasManagedSaveImage(self, flags):
        """Check if a domain has a managed save image as created by
        virDomainManagedSave(). Note that any running domain should not have
        such an image, as it should have been removed on restart. """
        ret = libvirtmod.virDomainHasManagedSaveImage(self._o, flags)
        if ret == -1: raise libvirtError ('virDomainHasManagedSaveImage() failed', dom=self)
        return ret

    def isActive(self):
        """Determine if the domain is currently running """
        ret = libvirtmod.virDomainIsActive(self._o)
        if ret == -1: raise libvirtError ('virDomainIsActive() failed', dom=self)
        return ret

    def isPersistent(self):
        """Determine if the domain has a persistent configuration
        which means it will still exist after shutting down """
        ret = libvirtmod.virDomainIsPersistent(self._o)
        if ret == -1: raise libvirtError ('virDomainIsPersistent() failed', dom=self)
        return ret

    def isUpdated(self):
        """Determine if the domain has been updated. """
        ret = libvirtmod.virDomainIsUpdated(self._o)
        if ret == -1: raise libvirtError ('virDomainIsUpdated() failed', dom=self)
        return ret

    def managedSave(self, flags):
        """This method will suspend a domain and save its memory contents to
        a file on disk. After the call, if successful, the domain is not
        listed as running anymore.
        The difference from virDomainSave() is that libvirt is keeping track of
        the saved state itself, and will reuse it once the domain is being
        restarted (automatically or via an explicit libvirt call).
        As a result any running domain is sure to not have a managed saved image. """
        ret = libvirtmod.virDomainManagedSave(self._o, flags)
        if ret == -1: raise libvirtError ('virDomainManagedSave() failed', dom=self)
        return ret

    def managedSaveRemove(self, flags):
        """Remove any managed save image for this domain. """
        ret = libvirtmod.virDomainManagedSaveRemove(self._o, flags)
        if ret == -1: raise libvirtError ('virDomainManagedSaveRemove() failed', dom=self)
        return ret

    def maxMemory(self):
        """Retrieve the maximum amount of physical memory allocated to a
        domain. If domain is None, then this get the amount of memory reserved
        to Domain0 i.e. the domain where the application runs. """
        ret = libvirtmod.virDomainGetMaxMemory(self._o)
        if ret == 0: raise libvirtError ('virDomainGetMaxMemory() failed', dom=self)
        return ret

    def maxVcpus(self):
        """Provides the maximum number of virtual CPUs supported for
        the guest VM. If the guest is inactive, this is basically
        the same as virConnectGetMaxVcpus(). If the guest is running
        this will reflect the maximum number of virtual CPUs the
        guest was booted with.  For more details, see virDomainGetVcpusFlags(). """
        ret = libvirtmod.virDomainGetMaxVcpus(self._o)
        if ret == -1: raise libvirtError ('virDomainGetMaxVcpus() failed', dom=self)
        return ret

    def memoryPeek(self, start, size, buffer, flags):
        """This function allows you to read the contents of a domain's
        memory.
        
        The memory which is read is controlled by the 'start', 'size'
        and 'flags' parameters.
        
        If 'flags' is VIR_MEMORY_VIRTUAL then the 'start' and 'size'
        parameters are interpreted as virtual memory addresses for
        whichever task happens to be running on the domain at the
        moment.  Although this sounds haphazard it is in fact what
        you want in order to read Linux kernel state, because it
        ensures that pointers in the kernel image can be interpreted
        coherently.
        
        'buffer' is the return buffer and must be at least 'size' bytes.
        'size' may be 0 to test if the call would succeed.
        
        NB. The remote driver imposes a 64K byte limit on 'size'.
        For your program to be able to work reliably over a remote
        connection you should split large requests to <= 65536 bytes. """
        ret = libvirtmod.virDomainMemoryPeek(self._o, start, size, buffer, flags)
        if ret == -1: raise libvirtError ('virDomainMemoryPeek() failed', dom=self)
        return ret

    def migrate(self, dconn, flags, dname, uri, bandwidth):
        """Migrate the domain object from its current host to the destination
        host given by dconn (a connection to the destination host).
        
        Flags may be one of more of the following:
          VIR_MIGRATE_LIVE      Do not pause the VM during migration
          VIR_MIGRATE_PEER2PEER Direct connection between source & destination hosts
          VIR_MIGRATE_TUNNELLED Tunnel migration data over the libvirt RPC channel
          VIR_MIGRATE_PERSIST_DEST If the migration is successful, persist the domain
                                   on the destination host.
          VIR_MIGRATE_UNDEFINE_SOURCE If the migration is successful, undefine the
                                      domain on the source host.
          VIR_MIGRATE_PAUSED    Leave the domain suspended on the remote side.
        
        VIR_MIGRATE_TUNNELLED requires that VIR_MIGRATE_PEER2PEER be set.
        Applications using the VIR_MIGRATE_PEER2PEER flag will probably
        prefer to invoke virDomainMigrateToURI, avoiding the need to
        open connection to the destination host themselves.
        
        If a hypervisor supports renaming domains during migration,
        then you may set the dname parameter to the new name (otherwise
        it keeps the same name).  If this is not supported by the
        hypervisor, dname must be None or else you will get an error.
        
        If the VIR_MIGRATE_PEER2PEER flag is set, the uri parameter
        must be a valid libvirt connection URI, by which the source
        libvirt driver can connect to the destination libvirt. If
        omitted, the dconn connection object will be queried for its
        current URI.
        
        If the VIR_MIGRATE_PEER2PEER flag is NOT set, the URI parameter
        takes a hypervisor specific format. The hypervisor capabilities
        XML includes details of the support URI schemes. If omitted
        the dconn will be asked for a default URI.
        
        In either case it is typically only necessary to specify a
        URI if the destination host has multiple interfaces and a
        specific interface is required to transmit migration data.
        
        The maximum bandwidth (in Mbps) that will be used to do migration
        can be specified with the bandwidth parameter.  If set to 0,
        libvirt will choose a suitable default.  Some hypervisors do
        not support this feature and will return an error if bandwidth
        is not 0.
        
        To see which features are supported by the current hypervisor,
        see virConnectGetCapabilities, /capabilities/host/migration_features.
        
        There are many limitations on migration imposed by the underlying
        technology - for example it may not be possible to migrate between
        different processors even with the same architecture, or between
        different types of hypervisor. """
        if dconn is None: dconn__o = None
        else: dconn__o = dconn._o
        ret = libvirtmod.virDomainMigrate(self._o, dconn__o, flags, dname, uri, bandwidth)
        if ret is None:raise libvirtError('virDomainMigrate() failed', dom=self)
        __tmp = virDomain(self,_obj=ret)
        return __tmp

    def migrateSetMaxDowntime(self, downtime, flags):
        """Sets maximum tolerable time for which the domain is allowed to be paused
        at the end of live migration. It's supposed to be called while the domain is
        being live-migrated as a reaction to migration progress. """
        ret = libvirtmod.virDomainMigrateSetMaxDowntime(self._o, downtime, flags)
        if ret == -1: raise libvirtError ('virDomainMigrateSetMaxDowntime() failed', dom=self)
        return ret

    def migrateToURI(self, duri, flags, dname, bandwidth):
        """Migrate the domain object from its current host to the destination
        host given by duri.
        
        Flags may be one of more of the following:
          VIR_MIGRATE_LIVE      Do not pause the VM during migration
          VIR_MIGRATE_PEER2PEER Direct connection between source & destination hosts
          VIR_MIGRATE_TUNNELLED Tunnel migration data over the libvirt RPC channel
          VIR_MIGRATE_PERSIST_DEST If the migration is successful, persist the domain
                                   on the destination host.
          VIR_MIGRATE_UNDEFINE_SOURCE If the migration is successful, undefine the
                                      domain on the source host.
        
        The operation of this API hinges on the VIR_MIGRATE_PEER2PEER flag.
        If the VIR_MIGRATE_PEER2PEER flag is NOT set, the duri parameter
        takes a hypervisor specific format. The uri_transports element of the
        hypervisor capabilities XML includes details of the supported URI
        schemes. Not all hypervisors will support this mode of migration, so
        if the VIR_MIGRATE_PEER2PEER flag is not set, then it may be necessary
        to use the alternative virDomainMigrate API providing and explicit
        virConnectPtr for the destination host.
        
        If the VIR_MIGRATE_PEER2PEER flag IS set, the duri parameter
        must be a valid libvirt connection URI, by which the source
        libvirt driver can connect to the destination libvirt.
        
        VIR_MIGRATE_TUNNELLED requires that VIR_MIGRATE_PEER2PEER be set.
        
        If a hypervisor supports renaming domains during migration,
        the dname parameter specifies the new name for the domain.
        Setting dname to None keeps the domain name the same.  If domain
        renaming is not supported by the hypervisor, dname must be None or
        else an error will be returned.
        
        The maximum bandwidth (in Mbps) that will be used to do migration
        can be specified with the bandwidth parameter.  If set to 0,
        libvirt will choose a suitable default.  Some hypervisors do
        not support this feature and will return an error if bandwidth
        is not 0.
        
        To see which features are supported by the current hypervisor,
        see virConnectGetCapabilities, /capabilities/host/migration_features.
        
        There are many limitations on migration imposed by the underlying
        technology - for example it may not be possible to migrate between
        different processors even with the same architecture, or between
        different types of hypervisor. """
        ret = libvirtmod.virDomainMigrateToURI(self._o, duri, flags, dname, bandwidth)
        if ret == -1: raise libvirtError ('virDomainMigrateToURI() failed', dom=self)
        return ret

    def name(self):
        """Get the public name for that domain """
        ret = libvirtmod.virDomainGetName(self._o)
        return ret

    def openConsole(self, devname, st, flags):
        """This opens the backend associated with a console, serial or
        parallel port device on a guest, if the backend is supported.
        If the @devname is omitted, then the first console or serial
        device is opened. The console is associated with the passed
        in @st stream, which should have been opened in non-blocking
        mode for bi-directional I/O. """
        if st is None: st__o = None
        else: st__o = st._o
        ret = libvirtmod.virDomainOpenConsole(self._o, devname, st__o, flags)
        if ret == -1: raise libvirtError ('virDomainOpenConsole() failed', dom=self)
        return ret

    def reboot(self, flags):
        """Reboot a domain, the domain object is still usable there after but
        the domain OS is being stopped for a restart.
        Note that the guest OS may ignore the request. """
        ret = libvirtmod.virDomainReboot(self._o, flags)
        if ret == -1: raise libvirtError ('virDomainReboot() failed', dom=self)
        return ret

    def resume(self):
        """Resume a suspended domain, the process is restarted from the state where
        it was frozen by calling virSuspendDomain().
        This function may requires privileged access """
        ret = libvirtmod.virDomainResume(self._o)
        if ret == -1: raise libvirtError ('virDomainResume() failed', dom=self)
        return ret

    def save(self, to):
        """This method will suspend a domain and save its memory contents to
        a file on disk. After the call, if successful, the domain is not
        listed as running anymore (this may be a problem).
        Use virDomainRestore() to restore a domain after saving. """
        ret = libvirtmod.virDomainSave(self._o, to)
        if ret == -1: raise libvirtError ('virDomainSave() failed', dom=self)
        return ret

    def setAutostart(self, autostart):
        """Configure the domain to be automatically started
        when the host machine boots. """
        ret = libvirtmod.virDomainSetAutostart(self._o, autostart)
        if ret == -1: raise libvirtError ('virDomainSetAutostart() failed', dom=self)
        return ret

    def setMaxMemory(self, memory):
        """Dynamically change the maximum amount of physical memory allocated to a
        domain. If domain is None, then this change the amount of memory reserved
        to Domain0 i.e. the domain where the application runs.
        This function requires privileged access to the hypervisor.
        
        This command only changes the runtime configuration of the domain,
        so can only be called on an active domain. """
        ret = libvirtmod.virDomainSetMaxMemory(self._o, memory)
        if ret == -1: raise libvirtError ('virDomainSetMaxMemory() failed', dom=self)
        return ret

    def setMemory(self, memory):
        """Dynamically change the target amount of physical memory allocated to a
        domain. If domain is None, then this change the amount of memory reserved
        to Domain0 i.e. the domain where the application runs.
        This function may requires privileged access to the hypervisor.
        
        This command only changes the runtime configuration of the domain,
        so can only be called on an active domain. """
        ret = libvirtmod.virDomainSetMemory(self._o, memory)
        if ret == -1: raise libvirtError ('virDomainSetMemory() failed', dom=self)
        return ret

    def setVcpus(self, nvcpus):
        """Dynamically change the number of virtual CPUs used by the domain.
        Note that this call may fail if the underlying virtualization hypervisor
        does not support it or if growing the number is arbitrary limited.
        This function requires privileged access to the hypervisor.
        
        This command only changes the runtime configuration of the domain,
        so can only be called on an active domain.  It is hypervisor-dependent
        whether it also affects persistent configuration; for more control,
        use virDomainSetVcpusFlags(). """
        ret = libvirtmod.virDomainSetVcpus(self._o, nvcpus)
        if ret == -1: raise libvirtError ('virDomainSetVcpus() failed', dom=self)
        return ret

    def setVcpusFlags(self, nvcpus, flags):
        """Dynamically change the number of virtual CPUs used by the domain.
        Note that this call may fail if the underlying virtualization hypervisor
        does not support it or if growing the number is arbitrary limited.
        This function requires privileged access to the hypervisor.
        
        @flags must include VIR_DOMAIN_VCPU_LIVE to affect a running
        domain (which may fail if domain is not active), or
        VIR_DOMAIN_VCPU_CONFIG to affect the next boot via the XML
        description of the domain.  Both flags may be set.
        
        If @flags includes VIR_DOMAIN_VCPU_MAXIMUM, then
        VIR_DOMAIN_VCPU_LIVE must be clear, and only the maximum virtual
        CPU limit is altered; generally, this value must be less than or
        equal to virConnectGetMaxVcpus().  Otherwise, this call affects the
        current virtual CPU limit, which must be less than or equal to the
        maximum limit. """
        ret = libvirtmod.virDomainSetVcpusFlags(self._o, nvcpus, flags)
        if ret == -1: raise libvirtError ('virDomainSetVcpusFlags() failed', dom=self)
        return ret

    def shutdown(self):
        """Shutdown a domain, the domain object is still usable there after but
        the domain OS is being stopped. Note that the guest OS may ignore the
        request.
        
        TODO: should we add an option for reboot, knowing it may not be doable
              in the general case ? """
        ret = libvirtmod.virDomainShutdown(self._o)
        if ret == -1: raise libvirtError ('virDomainShutdown() failed', dom=self)
        return ret

    def snapshotCreateXML(self, xmlDesc, flags):
        """Creates a new snapshot of a domain based on the snapshot xml
        contained in xmlDesc. """
        ret = libvirtmod.virDomainSnapshotCreateXML(self._o, xmlDesc, flags)
        if ret is None:raise libvirtError('virDomainSnapshotCreateXML() failed', dom=self)
        __tmp = virDomainSnapshot(self,_obj=ret)
        return __tmp

    def snapshotCurrent(self, flags):
        """Get the current snapshot for a domain, if any. """
        ret = libvirtmod.virDomainSnapshotCurrent(self._o, flags)
        if ret is None:raise libvirtError('virDomainSnapshotCurrent() failed', dom=self)
        __tmp = virDomainSnapshot(self,_obj=ret)
        return __tmp

    def snapshotLookupByName(self, name, flags):
        """Try to lookup a domain snapshot based on its name. """
        ret = libvirtmod.virDomainSnapshotLookupByName(self._o, name, flags)
        if ret is None:raise libvirtError('virDomainSnapshotLookupByName() failed', dom=self)
        __tmp = virDomainSnapshot(self,_obj=ret)
        return __tmp

    def snapshotNum(self, flags):
        """Provides the number of domain snapshots for this domain.. """
        ret = libvirtmod.virDomainSnapshotNum(self._o, flags)
        if ret == -1: raise libvirtError ('virDomainSnapshotNum() failed', dom=self)
        return ret

    def suspend(self):
        """Suspends an active domain, the process is frozen without further access
        to CPU resources and I/O but the memory used by the domain at the
        hypervisor level will stay allocated. Use virDomainResume() to reactivate
        the domain.
        This function may requires privileged access. """
        ret = libvirtmod.virDomainSuspend(self._o)
        if ret == -1: raise libvirtError ('virDomainSuspend() failed', dom=self)
        return ret

    def undefine(self):
        """Undefine a domain but does not stop it if it is running """
        ret = libvirtmod.virDomainUndefine(self._o)
        if ret == -1: raise libvirtError ('virDomainUndefine() failed', dom=self)
        return ret

    def updateDeviceFlags(self, xml, flags):
        """Change a virtual device on a domain, using the flags parameter
        to control how the device is changed.  VIR_DOMAIN_DEVICE_MODIFY_CURRENT
        specifies that the device change is made based on current domain
        state.  VIR_DOMAIN_DEVICE_MODIFY_LIVE specifies that the device shall be
        changed on the active domain instance only and is not added to the
        persisted domain configuration. VIR_DOMAIN_DEVICE_MODIFY_CONFIG
        specifies that the device shall be changed on the persisted domain
        configuration only.  Note that the target hypervisor must return an
        error if unable to satisfy flags.  E.g. the hypervisor driver will
        return failure if LIVE is specified but it only supports modifying the
        persisted device allocation.
        
        This method is used for actions such changing CDROM/Floppy device
        media, altering the graphics configuration such as password,
        reconfiguring the NIC device backend connectivity, etc. """
        ret = libvirtmod.virDomainUpdateDeviceFlags(self._o, xml, flags)
        if ret == -1: raise libvirtError ('virDomainUpdateDeviceFlags() failed', dom=self)
        return ret

    def vcpusFlags(self, flags):
        """Query the number of virtual CPUs used by the domain.  Note that
        this call may fail if the underlying virtualization hypervisor does
        not support it.  This function requires privileged access to the
        hypervisor.
        
        @flags must include either VIR_DOMAIN_VCPU_ACTIVE to query a
        running domain (which will fail if domain is not active), or
        VIR_DOMAIN_VCPU_PERSISTENT to query the XML description of the
        domain.  It is an error to set both flags.
        
        If @flags includes VIR_DOMAIN_VCPU_MAXIMUM, then the maximum
        virtual CPU limit is queried.  Otherwise, this call queries the
        current virtual CPU limit. """
        ret = libvirtmod.virDomainGetVcpusFlags(self._o, flags)
        if ret == -1: raise libvirtError ('virDomainGetVcpusFlags() failed', dom=self)
        return ret

    #
    # virDomain functions from module python
    #

    def UUID(self):
        """Extract the UUID unique Identifier of a domain. """
        ret = libvirtmod.virDomainGetUUID(self._o)
        if ret is None: raise libvirtError ('virDomainGetUUID() failed', dom=self)
        return ret

    def UUIDString(self):
        """Fetch globally unique ID of the domain as a string. """
        ret = libvirtmod.virDomainGetUUIDString(self._o)
        if ret is None: raise libvirtError ('virDomainGetUUIDString() failed', dom=self)
        return ret

    def autostart(self):
        """Extract the autostart flag for a domain """
        ret = libvirtmod.virDomainGetAutostart(self._o)
        if ret == -1: raise libvirtError ('virDomainGetAutostart() failed', dom=self)
        return ret

    def blockInfo(self, path, flags):
        """Extract information about a domain block device size """
        ret = libvirtmod.virDomainGetBlockInfo(self._o, path, flags)
        if ret is None: raise libvirtError ('virDomainGetBlockInfo() failed', dom=self)
        return ret

    def blockStats(self, path):
        """Extracts block device statistics for a domain """
        ret = libvirtmod.virDomainBlockStats(self._o, path)
        if ret is None: raise libvirtError ('virDomainBlockStats() failed', dom=self)
        return ret

    def info(self):
        """Extract information about a domain. Note that if the connection used to get the domain is limited only a partial set of the information can be extracted. """
        ret = libvirtmod.virDomainGetInfo(self._o)
        if ret is None: raise libvirtError ('virDomainGetInfo() failed', dom=self)
        return ret

    def interfaceStats(self, path):
        """Extracts interface device statistics for a domain """
        ret = libvirtmod.virDomainInterfaceStats(self._o, path)
        if ret is None: raise libvirtError ('virDomainInterfaceStats() failed', dom=self)
        return ret

    def jobInfo(self):
        """Extract information about an active job being processed for a domain. """
        ret = libvirtmod.virDomainGetJobInfo(self._o)
        if ret is None: raise libvirtError ('virDomainGetJobInfo() failed', dom=self)
        return ret

    def memoryParameters(self, params):
        """Get the memory parameters, the @params array will be filled with the values. """
        ret = libvirtmod.virDomainGetMemoryParameters(self._o, params)
        if ret == -1: raise libvirtError ('virDomainGetMemoryParameters() failed', dom=self)
        return ret

    def memoryStats(self):
        """Extracts memory statistics for a domain """
        ret = libvirtmod.virDomainMemoryStats(self._o)
        return ret

    def pinVcpu(self, vcpu, cpumap):
        """Dynamically change the real CPUs which can be allocated to a virtual CPU. This function requires privileged access to the hypervisor. """
        ret = libvirtmod.virDomainPinVcpu(self._o, vcpu, cpumap)
        if ret == -1: raise libvirtError ('virDomainPinVcpu() failed', dom=self)
        return ret

    def revertToSnapshot(self, snap, flags):
        """revert the domain to the given snapshot """
        if snap is None: snap__o = None
        else: snap__o = snap._o
        ret = libvirtmod.virDomainRevertToSnapshot(self._o, snap__o, flags)
        if ret == -1: raise libvirtError ('virDomainRevertToSnapshot() failed', dom=self)
        return ret

    def schedulerParameters(self):
        """Get the scheduler parameters, the @params array will be filled with the values. """
        ret = libvirtmod.virDomainGetSchedulerParameters(self._o)
        if ret == -1: raise libvirtError ('virDomainGetSchedulerParameters() failed', dom=self)
        return ret

    def schedulerType(self):
        """Get the scheduler type. """
        ret = libvirtmod.virDomainGetSchedulerType(self._o)
        if ret is None: raise libvirtError ('virDomainGetSchedulerType() failed', dom=self)
        return ret

    def setMemoryParameters(self, params):
        """Change the memory tunables """
        ret = libvirtmod.virDomainSetMemoryParameters(self._o, params)
        if ret == -1: raise libvirtError ('virDomainSetMemoryParameters() failed', dom=self)
        return ret

    def setSchedulerParameters(self, params):
        """Change the scheduler parameters """
        ret = libvirtmod.virDomainSetSchedulerParameters(self._o, params)
        if ret == -1: raise libvirtError ('virDomainSetSchedulerParameters() failed', dom=self)
        return ret

    def snapshotListNames(self, flags):
        """collect the list of snapshots for the given domain """
        ret = libvirtmod.virDomainSnapshotListNames(self._o, flags)
        if ret is None: raise libvirtError ('virDomainSnapshotListNames() failed', dom=self)
        return ret

    def vcpus(self):
        """Extract information about virtual CPUs of domain, store it in info array and also in cpumaps if this pointer is'nt None. """
        ret = libvirtmod.virDomainGetVcpus(self._o)
        if ret == -1: raise libvirtError ('virDomainGetVcpus() failed', dom=self)
        return ret

class virNetwork:
    def __init__(self, conn, _obj=None):
        self._conn = conn
        if _obj != None:self._o = _obj;return
        self._o = None

    def __del__(self):
        if self._o != None:
            libvirtmod.virNetworkFree(self._o)
        self._o = None

    def connect(self):
        return self._conn

    #
    # virNetwork functions from module libvirt
    #

    def XMLDesc(self, flags):
        """Provide an XML description of the network. The description may be reused
        later to relaunch the network with virNetworkCreateXML(). """
        ret = libvirtmod.virNetworkGetXMLDesc(self._o, flags)
        if ret is None: raise libvirtError ('virNetworkGetXMLDesc() failed', net=self)
        return ret

    def bridgeName(self):
        """Provides a bridge interface name to which a domain may connect
        a network interface in order to join the network. """
        ret = libvirtmod.virNetworkGetBridgeName(self._o)
        if ret is None: raise libvirtError ('virNetworkGetBridgeName() failed', net=self)
        return ret

    def create(self):
        """Create and start a defined network. If the call succeed the network
        moves from the defined to the running networks pools. """
        ret = libvirtmod.virNetworkCreate(self._o)
        if ret == -1: raise libvirtError ('virNetworkCreate() failed', net=self)
        return ret

    def destroy(self):
        """Destroy the network object. The running instance is shutdown if not down
        already and all resources used by it are given back to the hypervisor. This
        does not free the associated virNetworkPtr object.
        This function may require privileged access """
        ret = libvirtmod.virNetworkDestroy(self._o)
        if ret == -1: raise libvirtError ('virNetworkDestroy() failed', net=self)
        return ret

    def isActive(self):
        """Determine if the network is currently running """
        ret = libvirtmod.virNetworkIsActive(self._o)
        if ret == -1: raise libvirtError ('virNetworkIsActive() failed', net=self)
        return ret

    def isPersistent(self):
        """Determine if the network has a persistent configuration
        which means it will still exist after shutting down """
        ret = libvirtmod.virNetworkIsPersistent(self._o)
        if ret == -1: raise libvirtError ('virNetworkIsPersistent() failed', net=self)
        return ret

    def name(self):
        """Get the public name for that network """
        ret = libvirtmod.virNetworkGetName(self._o)
        return ret

    def setAutostart(self, autostart):
        """Configure the network to be automatically started
        when the host machine boots. """
        ret = libvirtmod.virNetworkSetAutostart(self._o, autostart)
        if ret == -1: raise libvirtError ('virNetworkSetAutostart() failed', net=self)
        return ret

    def undefine(self):
        """Undefine a network but does not stop it if it is running """
        ret = libvirtmod.virNetworkUndefine(self._o)
        if ret == -1: raise libvirtError ('virNetworkUndefine() failed', net=self)
        return ret

    #
    # virNetwork functions from module python
    #

    def UUID(self):
        """Extract the UUID unique Identifier of a network. """
        ret = libvirtmod.virNetworkGetUUID(self._o)
        if ret is None: raise libvirtError ('virNetworkGetUUID() failed', net=self)
        return ret

    def UUIDString(self):
        """Fetch globally unique ID of the network as a string. """
        ret = libvirtmod.virNetworkGetUUIDString(self._o)
        if ret is None: raise libvirtError ('virNetworkGetUUIDString() failed', net=self)
        return ret

    def autostart(self):
        """Extract the autostart flag for a network. """
        ret = libvirtmod.virNetworkGetAutostart(self._o)
        if ret == -1: raise libvirtError ('virNetworkGetAutostart() failed', net=self)
        return ret

class virInterface:
    def __init__(self, conn, _obj=None):
        self._conn = conn
        if _obj != None:self._o = _obj;return
        self._o = None

    def __del__(self):
        if self._o != None:
            libvirtmod.virInterfaceFree(self._o)
        self._o = None

    def connect(self):
        return self._conn

    #
    # virInterface functions from module libvirt
    #

    def MACString(self):
        """Get the MAC for an interface as string. For more information about
        MAC see RFC4122. """
        ret = libvirtmod.virInterfaceGetMACString(self._o)
        if ret is None: raise libvirtError ('virInterfaceGetMACString() failed', net=self)
        return ret

    def XMLDesc(self, flags):
        """VIR_INTERFACE_XML_INACTIVE - return the static configuration,
                                          suitable for use redefining the
                                          interface via virInterfaceDefineXML()
        
        Provide an XML description of the interface. If
        VIR_INTERFACE_XML_INACTIVE is set, the description may be reused
        later to redefine the interface with virInterfaceDefineXML(). If it
        is not set, the ip address and netmask will be the current live
        setting of the interface, not the settings from the config files. """
        ret = libvirtmod.virInterfaceGetXMLDesc(self._o, flags)
        if ret is None: raise libvirtError ('virInterfaceGetXMLDesc() failed', net=self)
        return ret

    def create(self, flags):
        """Activate an interface (ie call "ifup") """
        ret = libvirtmod.virInterfaceCreate(self._o, flags)
        if ret == -1: raise libvirtError ('virInterfaceCreate() failed', net=self)
        return ret

    def destroy(self, flags):
        """deactivate an interface (ie call "ifdown")
        This does not remove the interface from the config, and
        does not free the associated virInterfacePtr object. """
        ret = libvirtmod.virInterfaceDestroy(self._o, flags)
        if ret == -1: raise libvirtError ('virInterfaceDestroy() failed', net=self)
        return ret

    def isActive(self):
        """Determine if the interface is currently running """
        ret = libvirtmod.virInterfaceIsActive(self._o)
        if ret == -1: raise libvirtError ('virInterfaceIsActive() failed', net=self)
        return ret

    def name(self):
        """Get the public name for that interface """
        ret = libvirtmod.virInterfaceGetName(self._o)
        return ret

    def undefine(self):
        """Undefine an interface, ie remove it from the config.
        This does not free the associated virInterfacePtr object. """
        ret = libvirtmod.virInterfaceUndefine(self._o)
        if ret == -1: raise libvirtError ('virInterfaceUndefine() failed', net=self)
        return ret

class virStoragePool:
    def __init__(self, conn, _obj=None):
        self._conn = conn
        if not isinstance(conn, virConnect):
            self._conn = conn._conn
        if _obj != None:self._o = _obj;return
        self._o = None

    def __del__(self):
        if self._o != None:
            libvirtmod.virStoragePoolFree(self._o)
        self._o = None

    def connect(self):
        return self._conn

    #
    # virStoragePool functions from module libvirt
    #

    def XMLDesc(self, flags):
        """Fetch an XML document describing all aspects of the
        storage pool. This is suitable for later feeding back
        into the virStoragePoolCreateXML method. """
        ret = libvirtmod.virStoragePoolGetXMLDesc(self._o, flags)
        if ret is None: raise libvirtError ('virStoragePoolGetXMLDesc() failed', pool=self)
        return ret

    def build(self, flags):
        """Build the underlying storage pool """
        ret = libvirtmod.virStoragePoolBuild(self._o, flags)
        if ret == -1: raise libvirtError ('virStoragePoolBuild() failed', pool=self)
        return ret

    def create(self, flags):
        """Starts an inactive storage pool """
        ret = libvirtmod.virStoragePoolCreate(self._o, flags)
        if ret == -1: raise libvirtError ('virStoragePoolCreate() failed', pool=self)
        return ret

    def createXML(self, xmldesc, flags):
        """Create a storage volume within a pool based
        on an XML description. Not all pools support
        creation of volumes """
        ret = libvirtmod.virStorageVolCreateXML(self._o, xmldesc, flags)
        if ret is None:raise libvirtError('virStorageVolCreateXML() failed', pool=self)
        __tmp = virStorageVol(self, _obj=ret)
        return __tmp

    def createXMLFrom(self, xmldesc, clonevol, flags):
        """Create a storage volume in the parent pool, using the
        'clonevol' volume as input. Information for the new
        volume (name, perms)  are passed via a typical volume
        XML description. """
        if clonevol is None: clonevol__o = None
        else: clonevol__o = clonevol._o
        ret = libvirtmod.virStorageVolCreateXMLFrom(self._o, xmldesc, clonevol__o, flags)
        if ret is None:raise libvirtError('virStorageVolCreateXMLFrom() failed', pool=self)
        __tmp = virStorageVol(self, _obj=ret)
        return __tmp

    def delete(self, flags):
        """Delete the underlying pool resources. This is
        a non-recoverable operation. The virStoragePoolPtr object
        itself is not free'd. """
        ret = libvirtmod.virStoragePoolDelete(self._o, flags)
        if ret == -1: raise libvirtError ('virStoragePoolDelete() failed', pool=self)
        return ret

    def destroy(self):
        """Destroy an active storage pool. This will deactivate the
        pool on the host, but keep any persistent config associated
        with it. If it has a persistent config it can later be
        restarted with virStoragePoolCreate(). This does not free
        the associated virStoragePoolPtr object. """
        ret = libvirtmod.virStoragePoolDestroy(self._o)
        if ret == -1: raise libvirtError ('virStoragePoolDestroy() failed', pool=self)
        return ret

    def isActive(self):
        """Determine if the storage pool is currently running """
        ret = libvirtmod.virStoragePoolIsActive(self._o)
        if ret == -1: raise libvirtError ('virStoragePoolIsActive() failed', pool=self)
        return ret

    def isPersistent(self):
        """Determine if the storage pool has a persistent configuration
        which means it will still exist after shutting down """
        ret = libvirtmod.virStoragePoolIsPersistent(self._o)
        if ret == -1: raise libvirtError ('virStoragePoolIsPersistent() failed', pool=self)
        return ret

    def name(self):
        """Fetch the locally unique name of the storage pool """
        ret = libvirtmod.virStoragePoolGetName(self._o)
        return ret

    def numOfVolumes(self):
        """Fetch the number of storage volumes within a pool """
        ret = libvirtmod.virStoragePoolNumOfVolumes(self._o)
        if ret == -1: raise libvirtError ('virStoragePoolNumOfVolumes() failed', pool=self)
        return ret

    def refresh(self, flags):
        """Request that the pool refresh its list of volumes. This may
        involve communicating with a remote server, and/or initializing
        new devices at the OS layer """
        ret = libvirtmod.virStoragePoolRefresh(self._o, flags)
        if ret == -1: raise libvirtError ('virStoragePoolRefresh() failed', pool=self)
        return ret

    def setAutostart(self, autostart):
        """Sets the autostart flag """
        ret = libvirtmod.virStoragePoolSetAutostart(self._o, autostart)
        if ret == -1: raise libvirtError ('virStoragePoolSetAutostart() failed', pool=self)
        return ret

    def storageVolLookupByName(self, name):
        """Fetch a pointer to a storage volume based on its name
        within a pool """
        ret = libvirtmod.virStorageVolLookupByName(self._o, name)
        if ret is None:raise libvirtError('virStorageVolLookupByName() failed', pool=self)
        __tmp = virStorageVol(self, _obj=ret)
        return __tmp

    def undefine(self):
        """Undefine an inactive storage pool """
        ret = libvirtmod.virStoragePoolUndefine(self._o)
        if ret == -1: raise libvirtError ('virStoragePoolUndefine() failed', pool=self)
        return ret

    #
    # virStoragePool functions from module python
    #

    def UUID(self):
        """Extract the UUID unique Identifier of a storage pool. """
        ret = libvirtmod.virStoragePoolGetUUID(self._o)
        if ret is None: raise libvirtError ('virStoragePoolGetUUID() failed', pool=self)
        return ret

    def UUIDString(self):
        """Fetch globally unique ID of the storage pool as a string. """
        ret = libvirtmod.virStoragePoolGetUUIDString(self._o)
        if ret is None: raise libvirtError ('virStoragePoolGetUUIDString() failed', pool=self)
        return ret

    def autostart(self):
        """Extract the autostart flag for a storage pool """
        ret = libvirtmod.virStoragePoolGetAutostart(self._o)
        if ret == -1: raise libvirtError ('virStoragePoolGetAutostart() failed', pool=self)
        return ret

    def info(self):
        """Extract information about a storage pool. Note that if the connection used to get the domain is limited only a partial set of the information can be extracted. """
        ret = libvirtmod.virStoragePoolGetInfo(self._o)
        if ret is None: raise libvirtError ('virStoragePoolGetInfo() failed', pool=self)
        return ret

    def listVolumes(self):
        """list the storage volumes, stores the pointers to the names in @names """
        ret = libvirtmod.virStoragePoolListVolumes(self._o)
        if ret is None: raise libvirtError ('virStoragePoolListVolumes() failed', pool=self)
        return ret

class virStorageVol:
    def __init__(self, conn, _obj=None):
        self._conn = conn
        if not isinstance(conn, virConnect):
            self._conn = conn._conn
        if _obj != None:self._o = _obj;return
        self._o = None

    def __del__(self):
        if self._o != None:
            libvirtmod.virStorageVolFree(self._o)
        self._o = None

    def connect(self):
        return self._conn

    #
    # virStorageVol functions from module libvirt
    #

    def XMLDesc(self, flags):
        """Fetch an XML document describing all aspects of
        the storage volume """
        ret = libvirtmod.virStorageVolGetXMLDesc(self._o, flags)
        if ret is None: raise libvirtError ('virStorageVolGetXMLDesc() failed', vol=self)
        return ret

    def delete(self, flags):
        """Delete the storage volume from the pool """
        ret = libvirtmod.virStorageVolDelete(self._o, flags)
        if ret == -1: raise libvirtError ('virStorageVolDelete() failed', vol=self)
        return ret

    def key(self):
        """Fetch the storage volume key. This is globally
        unique, so the same volume will have the same
        key no matter what host it is accessed from """
        ret = libvirtmod.virStorageVolGetKey(self._o)
        if ret is None: raise libvirtError ('virStorageVolGetKey() failed', vol=self)
        return ret

    def name(self):
        """Fetch the storage volume name. This is unique
        within the scope of a pool """
        ret = libvirtmod.virStorageVolGetName(self._o)
        return ret

    def path(self):
        """Fetch the storage volume path. Depending on the pool
        configuration this is either persistent across hosts,
        or dynamically assigned at pool startup. Consult
        pool documentation for information on getting the
        persistent naming """
        ret = libvirtmod.virStorageVolGetPath(self._o)
        if ret is None: raise libvirtError ('virStorageVolGetPath() failed', vol=self)
        return ret

    def storagePoolLookupByVolume(self):
        """Fetch a storage pool which contains a particular volume """
        ret = libvirtmod.virStoragePoolLookupByVolume(self._o)
        if ret is None:raise libvirtError('virStoragePoolLookupByVolume() failed', vol=self)
        __tmp = virStoragePool(self, _obj=ret)
        return __tmp

    def wipe(self, flags):
        """Ensure data previously on a volume is not accessible to future reads """
        ret = libvirtmod.virStorageVolWipe(self._o, flags)
        if ret == -1: raise libvirtError ('virStorageVolWipe() failed', vol=self)
        return ret

    #
    # virStorageVol functions from module python
    #

    def info(self):
        """Extract information about a storage volume. Note that if the connection used to get the domain is limited only a partial set of the information can be extracted. """
        ret = libvirtmod.virStorageVolGetInfo(self._o)
        if ret is None: raise libvirtError ('virStorageVolGetInfo() failed', vol=self)
        return ret

class virConnect:
    def __init__(self, _obj=None):
        if _obj != None:self._o = _obj;return
        self._o = None

    #
    # virConnect functions from module libvirt
    #

    def close(self):
        """This function closes the connection to the Hypervisor. This should
        not be called if further interaction with the Hypervisor are needed
        especially if there is running domain which need further monitoring by
        the application. """
        ret = libvirtmod.virConnectClose(self._o)
        self._o = None
        if ret == -1: raise libvirtError ('virConnectClose() failed', conn=self)
        return ret

    def compareCPU(self, xmlDesc, flags):
        """Compares the given CPU description with the host CPU """
        ret = libvirtmod.virConnectCompareCPU(self._o, xmlDesc, flags)
        if ret == -1: raise libvirtError ('virConnectCompareCPU() failed', conn=self)
        return ret

    def createLinux(self, xmlDesc, flags):
        """Deprecated after 0.4.6.
        Renamed to virDomainCreateXML() providing identical functionality.
        This existing name will left indefinitely for API compatibility. """
        ret = libvirtmod.virDomainCreateLinux(self._o, xmlDesc, flags)
        if ret is None:raise libvirtError('virDomainCreateLinux() failed', conn=self)
        __tmp = virDomain(self,_obj=ret)
        return __tmp

    def createXML(self, xmlDesc, flags):
        """Launch a new guest domain, based on an XML description similar
        to the one returned by virDomainGetXMLDesc()
        This function may requires privileged access to the hypervisor.
        The domain is not persistent, so its definition will disappear when it
        is destroyed, or if the host is restarted (see virDomainDefineXML() to
        define persistent domains). """
        ret = libvirtmod.virDomainCreateXML(self._o, xmlDesc, flags)
        if ret is None:raise libvirtError('virDomainCreateXML() failed', conn=self)
        __tmp = virDomain(self,_obj=ret)
        return __tmp

    def defineXML(self, xml):
        """Define a domain, but does not start it.
        This definition is persistent, until explicitly undefined with
        virDomainUndefine(). A previous definition for this domain would be
        overriden if it already exists. """
        ret = libvirtmod.virDomainDefineXML(self._o, xml)
        if ret is None:raise libvirtError('virDomainDefineXML() failed', conn=self)
        __tmp = virDomain(self,_obj=ret)
        return __tmp

    def domainXMLFromNative(self, nativeFormat, nativeConfig, flags):
        """Reads native configuration data  describing a domain, and
        generates libvirt domain XML. The format of the native
        data is hypervisor dependant. """
        ret = libvirtmod.virConnectDomainXMLFromNative(self._o, nativeFormat, nativeConfig, flags)
        if ret is None: raise libvirtError ('virConnectDomainXMLFromNative() failed', conn=self)
        return ret

    def domainXMLToNative(self, nativeFormat, domainXml, flags):
        """Reads a domain XML configuration document, and generates
        a native configuration file describing the domain.
        The format of the native data is hypervisor dependant. """
        ret = libvirtmod.virConnectDomainXMLToNative(self._o, nativeFormat, domainXml, flags)
        if ret is None: raise libvirtError ('virConnectDomainXMLToNative() failed', conn=self)
        return ret

    def findStoragePoolSources(self, type, srcSpec, flags):
        """Talks to a storage backend and attempts to auto-discover the set of
        available storage pool sources. e.g. For iSCSI this would be a set of
        iSCSI targets. For NFS this would be a list of exported paths.  The
        srcSpec (optional for some storage pool types, e.g. local ones) is
        an instance of the storage pool's source element specifying where
        to look for the pools.
        
        srcSpec is not required for some types (e.g., those querying
        local storage resources only) """
        ret = libvirtmod.virConnectFindStoragePoolSources(self._o, type, srcSpec, flags)
        if ret is None: raise libvirtError ('virConnectFindStoragePoolSources() failed', conn=self)
        return ret

    def getCapabilities(self):
        """Provides capabilities of the hypervisor / driver. """
        ret = libvirtmod.virConnectGetCapabilities(self._o)
        if ret is None: raise libvirtError ('virConnectGetCapabilities() failed', conn=self)
        return ret

    def getFreeMemory(self):
        """provides the free memory available on the Node
        Note: most libvirt APIs provide memory sizes in kilobytes, but in this
        function the returned value is in bytes. Divide by 1024 as necessary. """
        ret = libvirtmod.virNodeGetFreeMemory(self._o)
        return ret

    def getHostname(self):
        """This returns the system hostname on which the hypervisor is
        running (the result of the gethostname system call).  If
        we are connected to a remote system, then this returns the
        hostname of the remote system. """
        ret = libvirtmod.virConnectGetHostname(self._o)
        if ret is None: raise libvirtError ('virConnectGetHostname() failed', conn=self)
        return ret

    def getMaxVcpus(self, type):
        """Provides the maximum number of virtual CPUs supported for a guest VM of a
        specific type. The 'type' parameter here corresponds to the 'type'
        attribute in the <domain> element of the XML. """
        ret = libvirtmod.virConnectGetMaxVcpus(self._o, type)
        if ret == -1: raise libvirtError ('virConnectGetMaxVcpus() failed', conn=self)
        return ret

    def getSysinfo(self, flags):
        """This returns the XML description of the sysinfo details for the
        host on which the hypervisor is running, in the same format as the
        <sysinfo> element of a domain XML.  This information is generally
        available only for hypervisors running with root privileges. """
        ret = libvirtmod.virConnectGetSysinfo(self._o, flags)
        if ret is None: raise libvirtError ('virConnectGetSysinfo() failed', conn=self)
        return ret

    def getType(self):
        """Get the name of the Hypervisor software used. """
        ret = libvirtmod.virConnectGetType(self._o)
        if ret is None: raise libvirtError ('virConnectGetType() failed', conn=self)
        return ret

    def getURI(self):
        """This returns the URI (name) of the hypervisor connection.
        Normally this is the same as or similar to the string passed
        to the virConnectOpen/virConnectOpenReadOnly call, but
        the driver may make the URI canonical.  If name == None
        was passed to virConnectOpen, then the driver will return
        a non-None URI which can be used to connect to the same
        hypervisor later. """
        ret = libvirtmod.virConnectGetURI(self._o)
        if ret is None: raise libvirtError ('virConnectGetURI() failed', conn=self)
        return ret

    def interfaceDefineXML(self, xml, flags):
        """Define an interface (or modify existing interface configuration) """
        ret = libvirtmod.virInterfaceDefineXML(self._o, xml, flags)
        if ret is None:raise libvirtError('virInterfaceDefineXML() failed', conn=self)
        __tmp = virInterface(self, _obj=ret)
        return __tmp

    def interfaceLookupByMACString(self, macstr):
        """Try to lookup an interface on the given hypervisor based on its MAC. """
        ret = libvirtmod.virInterfaceLookupByMACString(self._o, macstr)
        if ret is None:raise libvirtError('virInterfaceLookupByMACString() failed', conn=self)
        __tmp = virInterface(self, _obj=ret)
        return __tmp

    def interfaceLookupByName(self, name):
        """Try to lookup an interface on the given hypervisor based on its name. """
        ret = libvirtmod.virInterfaceLookupByName(self._o, name)
        if ret is None:raise libvirtError('virInterfaceLookupByName() failed', conn=self)
        __tmp = virInterface(self, _obj=ret)
        return __tmp

    def isEncrypted(self):
        """Determine if the connection to the hypervisor is encrypted """
        ret = libvirtmod.virConnectIsEncrypted(self._o)
        if ret == -1: raise libvirtError ('virConnectIsEncrypted() failed', conn=self)
        return ret

    def isSecure(self):
        """Determine if the connection to the hypervisor is secure
        
        A connection will be classed as secure if it is either
        encrypted, or running over a channel which is not exposed
        to eavesdropping (eg a UNIX domain socket, or pipe) """
        ret = libvirtmod.virConnectIsSecure(self._o)
        if ret == -1: raise libvirtError ('virConnectIsSecure() failed', conn=self)
        return ret

    def listNWFilters(self):
        """List the defined network filters """
        ret = libvirtmod.virConnectListNWFilters(self._o)
        if ret is None: raise libvirtError ('virConnectListNWFilters() failed', conn=self)
        return ret

    def listSecrets(self):
        """List the defined secret IDs """
        ret = libvirtmod.virConnectListSecrets(self._o)
        if ret is None: raise libvirtError ('virConnectListSecrets() failed', conn=self)
        return ret

    def lookupByID(self, id):
        """Try to find a domain based on the hypervisor ID number
        Note that this won't work for inactive domains which have an ID of -1,
        in that case a lookup based on the Name or UUId need to be done instead. """
        ret = libvirtmod.virDomainLookupByID(self._o, id)
        if ret is None:raise libvirtError('virDomainLookupByID() failed', conn=self)
        __tmp = virDomain(self,_obj=ret)
        return __tmp

    def lookupByName(self, name):
        """Try to lookup a domain on the given hypervisor based on its name. """
        ret = libvirtmod.virDomainLookupByName(self._o, name)
        if ret is None:raise libvirtError('virDomainLookupByName() failed', conn=self)
        __tmp = virDomain(self,_obj=ret)
        return __tmp

    def lookupByUUIDString(self, uuidstr):
        """Try to lookup a domain on the given hypervisor based on its UUID. """
        ret = libvirtmod.virDomainLookupByUUIDString(self._o, uuidstr)
        if ret is None:raise libvirtError('virDomainLookupByUUIDString() failed', conn=self)
        __tmp = virDomain(self,_obj=ret)
        return __tmp

    def migrate(self, domain, flags, dname, uri, bandwidth):
        """Migrate the domain object from its current host to the destination
        host given by dconn (a connection to the destination host).
        
        Flags may be one of more of the following:
          VIR_MIGRATE_LIVE      Do not pause the VM during migration
          VIR_MIGRATE_PEER2PEER Direct connection between source & destination hosts
          VIR_MIGRATE_TUNNELLED Tunnel migration data over the libvirt RPC channel
          VIR_MIGRATE_PERSIST_DEST If the migration is successful, persist the domain
                                   on the destination host.
          VIR_MIGRATE_UNDEFINE_SOURCE If the migration is successful, undefine the
                                      domain on the source host.
          VIR_MIGRATE_PAUSED    Leave the domain suspended on the remote side.
        
        VIR_MIGRATE_TUNNELLED requires that VIR_MIGRATE_PEER2PEER be set.
        Applications using the VIR_MIGRATE_PEER2PEER flag will probably
        prefer to invoke virDomainMigrateToURI, avoiding the need to
        open connection to the destination host themselves.
        
        If a hypervisor supports renaming domains during migration,
        then you may set the dname parameter to the new name (otherwise
        it keeps the same name).  If this is not supported by the
        hypervisor, dname must be None or else you will get an error.
        
        If the VIR_MIGRATE_PEER2PEER flag is set, the uri parameter
        must be a valid libvirt connection URI, by which the source
        libvirt driver can connect to the destination libvirt. If
        omitted, the dconn connection object will be queried for its
        current URI.
        
        If the VIR_MIGRATE_PEER2PEER flag is NOT set, the URI parameter
        takes a hypervisor specific format. The hypervisor capabilities
        XML includes details of the support URI schemes. If omitted
        the dconn will be asked for a default URI.
        
        In either case it is typically only necessary to specify a
        URI if the destination host has multiple interfaces and a
        specific interface is required to transmit migration data.
        
        The maximum bandwidth (in Mbps) that will be used to do migration
        can be specified with the bandwidth parameter.  If set to 0,
        libvirt will choose a suitable default.  Some hypervisors do
        not support this feature and will return an error if bandwidth
        is not 0.
        
        To see which features are supported by the current hypervisor,
        see virConnectGetCapabilities, /capabilities/host/migration_features.
        
        There are many limitations on migration imposed by the underlying
        technology - for example it may not be possible to migrate between
        different processors even with the same architecture, or between
        different types of hypervisor. """
        if domain is None: domain__o = None
        else: domain__o = domain._o
        ret = libvirtmod.virDomainMigrate(domain__o, self._o, flags, dname, uri, bandwidth)
        if ret is None:raise libvirtError('virDomainMigrate() failed', conn=self)
        __tmp = virDomain(self,_obj=ret)
        return __tmp

    def networkCreateXML(self, xmlDesc):
        """Create and start a new virtual network, based on an XML description
        similar to the one returned by virNetworkGetXMLDesc() """
        ret = libvirtmod.virNetworkCreateXML(self._o, xmlDesc)
        if ret is None:raise libvirtError('virNetworkCreateXML() failed', conn=self)
        __tmp = virNetwork(self, _obj=ret)
        return __tmp

    def networkDefineXML(self, xml):
        """Define a network, but does not create it """
        ret = libvirtmod.virNetworkDefineXML(self._o, xml)
        if ret is None:raise libvirtError('virNetworkDefineXML() failed', conn=self)
        __tmp = virNetwork(self, _obj=ret)
        return __tmp

    def networkLookupByName(self, name):
        """Try to lookup a network on the given hypervisor based on its name. """
        ret = libvirtmod.virNetworkLookupByName(self._o, name)
        if ret is None:raise libvirtError('virNetworkLookupByName() failed', conn=self)
        __tmp = virNetwork(self, _obj=ret)
        return __tmp

    def networkLookupByUUIDString(self, uuidstr):
        """Try to lookup a network on the given hypervisor based on its UUID. """
        ret = libvirtmod.virNetworkLookupByUUIDString(self._o, uuidstr)
        if ret is None:raise libvirtError('virNetworkLookupByUUIDString() failed', conn=self)
        __tmp = virNetwork(self, _obj=ret)
        return __tmp

    def newStream(self, flags):
        """Creates a new stream object which can be used to perform
        streamed I/O with other public API function.
        
        When no longer needed, a stream object must be released
        with virStreamFree. If a data stream has been used,
        then the application must call virStreamFinish or
        virStreamAbort before free'ing to, in order to notify
        the driver of termination.
        
        If a non-blocking data stream is required passed
        VIR_STREAM_NONBLOCK for flags, otherwise pass 0. """
        ret = libvirtmod.virStreamNew(self._o, flags)
        if ret is None:raise libvirtError('virStreamNew() failed', conn=self)
        __tmp = virStream(self, _obj=ret)
        return __tmp

    def nodeDeviceCreateXML(self, xmlDesc, flags):
        """Create a new device on the VM host machine, for example, virtual
        HBAs created using vport_create. """
        ret = libvirtmod.virNodeDeviceCreateXML(self._o, xmlDesc, flags)
        if ret is None:raise libvirtError('virNodeDeviceCreateXML() failed', conn=self)
        __tmp = virNodeDevice(self, _obj=ret)
        return __tmp

    def nodeDeviceLookupByName(self, name):
        """Lookup a node device by its name. """
        ret = libvirtmod.virNodeDeviceLookupByName(self._o, name)
        if ret is None:raise libvirtError('virNodeDeviceLookupByName() failed', conn=self)
        __tmp = virNodeDevice(self, _obj=ret)
        return __tmp

    def numOfDefinedDomains(self):
        """Provides the number of defined but inactive domains. """
        ret = libvirtmod.virConnectNumOfDefinedDomains(self._o)
        if ret == -1: raise libvirtError ('virConnectNumOfDefinedDomains() failed', conn=self)
        return ret

    def numOfDefinedInterfaces(self):
        """Provides the number of defined (inactive) interfaces on the physical host. """
        ret = libvirtmod.virConnectNumOfDefinedInterfaces(self._o)
        if ret == -1: raise libvirtError ('virConnectNumOfDefinedInterfaces() failed', conn=self)
        return ret

    def numOfDefinedNetworks(self):
        """Provides the number of inactive networks. """
        ret = libvirtmod.virConnectNumOfDefinedNetworks(self._o)
        if ret == -1: raise libvirtError ('virConnectNumOfDefinedNetworks() failed', conn=self)
        return ret

    def numOfDefinedStoragePools(self):
        """Provides the number of inactive storage pools """
        ret = libvirtmod.virConnectNumOfDefinedStoragePools(self._o)
        if ret == -1: raise libvirtError ('virConnectNumOfDefinedStoragePools() failed', conn=self)
        return ret

    def numOfDevices(self, cap, flags):
        """Provides the number of node devices.
        
        If the optional 'cap'  argument is non-None, then the count
        will be restricted to devices with the specified capability """
        ret = libvirtmod.virNodeNumOfDevices(self._o, cap, flags)
        if ret == -1: raise libvirtError ('virNodeNumOfDevices() failed', conn=self)
        return ret

    def numOfDomains(self):
        """Provides the number of active domains. """
        ret = libvirtmod.virConnectNumOfDomains(self._o)
        if ret == -1: raise libvirtError ('virConnectNumOfDomains() failed', conn=self)
        return ret

    def numOfInterfaces(self):
        """Provides the number of active interfaces on the physical host. """
        ret = libvirtmod.virConnectNumOfInterfaces(self._o)
        if ret == -1: raise libvirtError ('virConnectNumOfInterfaces() failed', conn=self)
        return ret

    def numOfNWFilters(self):
        """Provides the number of nwfilters. """
        ret = libvirtmod.virConnectNumOfNWFilters(self._o)
        if ret == -1: raise libvirtError ('virConnectNumOfNWFilters() failed', conn=self)
        return ret

    def numOfNetworks(self):
        """Provides the number of active networks. """
        ret = libvirtmod.virConnectNumOfNetworks(self._o)
        if ret == -1: raise libvirtError ('virConnectNumOfNetworks() failed', conn=self)
        return ret

    def numOfSecrets(self):
        """Fetch number of currently defined secrets. """
        ret = libvirtmod.virConnectNumOfSecrets(self._o)
        if ret == -1: raise libvirtError ('virConnectNumOfSecrets() failed', conn=self)
        return ret

    def numOfStoragePools(self):
        """Provides the number of active storage pools """
        ret = libvirtmod.virConnectNumOfStoragePools(self._o)
        if ret == -1: raise libvirtError ('virConnectNumOfStoragePools() failed', conn=self)
        return ret

    def nwfilterDefineXML(self, xmlDesc):
        """Define a new network filter, based on an XML description
        similar to the one returned by virNWFilterGetXMLDesc() """
        ret = libvirtmod.virNWFilterDefineXML(self._o, xmlDesc)
        if ret is None:raise libvirtError('virNWFilterDefineXML() failed', conn=self)
        __tmp = virNWFilter(self, _obj=ret)
        return __tmp

    def nwfilterLookupByName(self, name):
        """Try to lookup a network filter on the given hypervisor based on its name. """
        ret = libvirtmod.virNWFilterLookupByName(self._o, name)
        if ret is None:raise libvirtError('virNWFilterLookupByName() failed', conn=self)
        __tmp = virNWFilter(self, _obj=ret)
        return __tmp

    def nwfilterLookupByUUIDString(self, uuidstr):
        """Try to lookup an nwfilter on the given hypervisor based on its UUID. """
        ret = libvirtmod.virNWFilterLookupByUUIDString(self._o, uuidstr)
        if ret is None:raise libvirtError('virNWFilterLookupByUUIDString() failed', conn=self)
        __tmp = virNWFilter(self, _obj=ret)
        return __tmp

    def restore(self, frm):
        """This method will restore a domain saved to disk by virDomainSave(). """
        ret = libvirtmod.virDomainRestore(self._o, frm)
        if ret == -1: raise libvirtError ('virDomainRestore() failed', conn=self)
        return ret

    def secretDefineXML(self, xml, flags):
        """If XML specifies a UUID, locates the specified secret and replaces all
        attributes of the secret specified by UUID by attributes specified in xml
        (any attributes not specified in xml are discarded).
        
        Otherwise, creates a new secret with an automatically chosen UUID, and
        initializes its attributes from xml. """
        ret = libvirtmod.virSecretDefineXML(self._o, xml, flags)
        if ret is None:raise libvirtError('virSecretDefineXML() failed', conn=self)
        __tmp = virSecret(self, _obj=ret)
        return __tmp

    def secretLookupByUUIDString(self, uuidstr):
        """Try to lookup a secret on the given hypervisor based on its UUID.
        Uses the printable string value to describe the UUID """
        ret = libvirtmod.virSecretLookupByUUIDString(self._o, uuidstr)
        if ret is None:raise libvirtError('virSecretLookupByUUIDString() failed', conn=self)
        __tmp = virSecret(self, _obj=ret)
        return __tmp

    def secretLookupByUsage(self, usageType, usageID):
        """Try to lookup a secret on the given hypervisor based on its usage
        The usageID is unique within the set of secrets sharing the
        same usageType value. """
        ret = libvirtmod.virSecretLookupByUsage(self._o, usageType, usageID)
        if ret is None:raise libvirtError('virSecretLookupByUsage() failed', conn=self)
        __tmp = virSecret(self, _obj=ret)
        return __tmp

    def storagePoolCreateXML(self, xmlDesc, flags):
        """Create a new storage based on its XML description. The
        pool is not persistent, so its definition will disappear
        when it is destroyed, or if the host is restarted """
        ret = libvirtmod.virStoragePoolCreateXML(self._o, xmlDesc, flags)
        if ret is None:raise libvirtError('virStoragePoolCreateXML() failed', conn=self)
        __tmp = virStoragePool(self, _obj=ret)
        return __tmp

    def storagePoolDefineXML(self, xml, flags):
        """Define a new inactive storage pool based on its XML description. The
        pool is persistent, until explicitly undefined. """
        ret = libvirtmod.virStoragePoolDefineXML(self._o, xml, flags)
        if ret is None:raise libvirtError('virStoragePoolDefineXML() failed', conn=self)
        __tmp = virStoragePool(self, _obj=ret)
        return __tmp

    def storagePoolLookupByName(self, name):
        """Fetch a storage pool based on its unique name """
        ret = libvirtmod.virStoragePoolLookupByName(self._o, name)
        if ret is None:raise libvirtError('virStoragePoolLookupByName() failed', conn=self)
        __tmp = virStoragePool(self, _obj=ret)
        return __tmp

    def storagePoolLookupByUUID(self, uuid):
        """Fetch a storage pool based on its globally unique id """
        ret = libvirtmod.virStoragePoolLookupByUUID(self._o, uuid)
        if ret is None:raise libvirtError('virStoragePoolLookupByUUID() failed', conn=self)
        __tmp = virStoragePool(self, _obj=ret)
        return __tmp

    def storagePoolLookupByUUIDString(self, uuidstr):
        """Fetch a storage pool based on its globally unique id """
        ret = libvirtmod.virStoragePoolLookupByUUIDString(self._o, uuidstr)
        if ret is None:raise libvirtError('virStoragePoolLookupByUUIDString() failed', conn=self)
        __tmp = virStoragePool(self, _obj=ret)
        return __tmp

    def storageVolLookupByKey(self, key):
        """Fetch a pointer to a storage volume based on its
        globally unique key """
        ret = libvirtmod.virStorageVolLookupByKey(self._o, key)
        if ret is None:raise libvirtError('virStorageVolLookupByKey() failed', conn=self)
        __tmp = virStorageVol(self, _obj=ret)
        return __tmp

    def storageVolLookupByPath(self, path):
        """Fetch a pointer to a storage volume based on its
        locally (host) unique path """
        ret = libvirtmod.virStorageVolLookupByPath(self._o, path)
        if ret is None:raise libvirtError('virStorageVolLookupByPath() failed', conn=self)
        __tmp = virStorageVol(self, _obj=ret)
        return __tmp

    #
    # virConnect functions from module python
    #

    def baselineCPU(self, xmlCPUs, flags):
        """Computes the most feature-rich CPU which is compatible with all given host CPUs. """
        ret = libvirtmod.virConnectBaselineCPU(self._o, xmlCPUs, flags)
        if ret is None: raise libvirtError ('virConnectBaselineCPU() failed', conn=self)
        return ret

    def getCellsFreeMemory(self, startCell, maxCells):
        """Returns the available memory for a list of cells """
        ret = libvirtmod.virNodeGetCellsFreeMemory(self._o, startCell, maxCells)
        if ret is None: raise libvirtError ('virNodeGetCellsFreeMemory() failed', conn=self)
        return ret

    def getInfo(self):
        """Extract hardware information about the Node. """
        ret = libvirtmod.virNodeGetInfo(self._o)
        if ret is None: raise libvirtError ('virNodeGetInfo() failed', conn=self)
        return ret

    def getLibVersion(self):
        """Returns the libvirt version of the connection host """
        ret = libvirtmod.virConnectGetLibVersion(self._o)
        if ret == -1: raise libvirtError ('virConnectGetLibVersion() failed', conn=self)
        return ret

    def getVersion(self):
        """Returns the running hypervisor version of the connection host """
        ret = libvirtmod.virConnectGetVersion(self._o)
        if ret == -1: raise libvirtError ('virConnectGetVersion() failed', conn=self)
        return ret

    def listDefinedDomains(self):
        """list the defined domains, stores the pointers to the names in @names """
        ret = libvirtmod.virConnectListDefinedDomains(self._o)
        if ret is None: raise libvirtError ('virConnectListDefinedDomains() failed', conn=self)
        return ret

    def listDefinedInterfaces(self):
        """list the defined interfaces, stores the pointers to the names in @names """
        ret = libvirtmod.virConnectListDefinedInterfaces(self._o)
        if ret is None: raise libvirtError ('virConnectListDefinedInterfaces() failed', conn=self)
        return ret

    def listDefinedNetworks(self):
        """list the defined networks, stores the pointers to the names in @names """
        ret = libvirtmod.virConnectListDefinedNetworks(self._o)
        if ret is None: raise libvirtError ('virConnectListDefinedNetworks() failed', conn=self)
        return ret

    def listDefinedStoragePools(self):
        """list the defined storage pool, stores the pointers to the names in @names """
        ret = libvirtmod.virConnectListDefinedStoragePools(self._o)
        if ret is None: raise libvirtError ('virConnectListDefinedStoragePools() failed', conn=self)
        return ret

    def listDevices(self, cap, flags):
        """list the node devices """
        ret = libvirtmod.virNodeListDevices(self._o, cap, flags)
        if ret is None: raise libvirtError ('virNodeListDevices() failed', conn=self)
        return ret

    def listDomainsID(self):
        """Returns the list of the ID of the domains on the hypervisor """
        ret = libvirtmod.virConnectListDomainsID(self._o)
        if ret is None: raise libvirtError ('virConnectListDomainsID() failed', conn=self)
        return ret

    def listInterfaces(self):
        """list the running interfaces, stores the pointers to the names in @names """
        ret = libvirtmod.virConnectListInterfaces(self._o)
        if ret is None: raise libvirtError ('virConnectListInterfaces() failed', conn=self)
        return ret

    def listNetworks(self):
        """list the networks, stores the pointers to the names in @names """
        ret = libvirtmod.virConnectListNetworks(self._o)
        if ret is None: raise libvirtError ('virConnectListNetworks() failed', conn=self)
        return ret

    def listStoragePools(self):
        """list the storage pools, stores the pointers to the names in @names """
        ret = libvirtmod.virConnectListStoragePools(self._o)
        if ret is None: raise libvirtError ('virConnectListStoragePools() failed', conn=self)
        return ret

    def lookupByUUID(self, uuid):
        """Try to lookup a domain on the given hypervisor based on its UUID. """
        ret = libvirtmod.virDomainLookupByUUID(self._o, uuid)
        if ret is None:raise libvirtError('virDomainLookupByUUID() failed', conn=self)
        __tmp = virDomain(self,_obj=ret)
        return __tmp

    def networkLookupByUUID(self, uuid):
        """Try to lookup a network on the given hypervisor based on its UUID. """
        ret = libvirtmod.virNetworkLookupByUUID(self._o, uuid)
        if ret is None:raise libvirtError('virNetworkLookupByUUID() failed', conn=self)
        __tmp = virNetwork(self, _obj=ret)
        return __tmp

    def nwfilterLookupByUUID(self, uuid):
        """Try to lookup a network filter on the given hypervisor based on its UUID. """
        ret = libvirtmod.virNWFilterLookupByUUID(self._o, uuid)
        if ret is None:raise libvirtError('virNWFilterLookupByUUID() failed', conn=self)
        __tmp = virNWFilter(self, _obj=ret)
        return __tmp

    def secretLookupByUUID(self, uuid):
        """Try to lookup a secret on the given hypervisor based on its UUID. """
        ret = libvirtmod.virSecretLookupByUUID(self._o, uuid)
        if ret is None:raise libvirtError('virSecretLookupByUUID() failed', conn=self)
        __tmp = virSecret(self, _obj=ret)
        return __tmp

    #
    # virConnect functions from module virterror
    #

    def virConnGetLastError(self):
        """Provide a pointer to the last error caught on that connection
        
        This method is not protected against access from multiple
        threads. In a multi-threaded application, always use the
        global virGetLastError() API which is backed by thread
        local storage.
        
        If the connection object was discovered to be invalid by
        an API call, then the error will be reported against the
        global error object.
        
        Since 0.6.0, all errors reported in the per-connection object
        are also duplicated in the global error object. As such an
        application can always use virGetLastError(). This method
        remains for backwards compatability. """
        ret = libvirtmod.virConnGetLastError(self._o)
        return ret

    def virConnResetLastError(self):
        """The error object is kept in thread local storage, so separate
        threads can safely access this concurrently.
        
        Reset the last error caught on that connection """
        libvirtmod.virConnResetLastError(self._o)

    #
    # virConnect methods from virConnect.py (hand coded)
    #
    def __del__(self):
        try:
           for cb,opaque in self.domainEventCallbacks.items():
               del self.domainEventCallbacks[cb]
           del self.domainEventCallbacks
           libvirtmod.virConnectDomainEventDeregister(self._o, self)
        except AttributeError:
           pass

        if self._o != None:
            libvirtmod.virConnectClose(self._o)
        self._o = None

    def domainEventDeregister(self, cb):
        """Removes a Domain Event Callback. De-registering for a
           domain callback will disable delivery of this event type """
        try:
            del self.domainEventCallbacks[cb]
            if len(self.domainEventCallbacks) == 0:
                del self.domainEventCallbacks
                ret = libvirtmod.virConnectDomainEventDeregister(self._o, self)
                if ret == -1: raise libvirtError ('virConnectDomainEventDeregister() failed', conn=self)
        except AttributeError:
            pass

    def domainEventRegister(self, cb, opaque):
        """Adds a Domain Event Callback. Registering for a domain
           callback will enable delivery of the events """
        try:
            self.domainEventCallbacks[cb] = opaque
        except AttributeError:
            self.domainEventCallbacks = {cb:opaque}
            ret = libvirtmod.virConnectDomainEventRegister(self._o, self)
            if ret == -1: raise libvirtError ('virConnectDomainEventRegister() failed', conn=self)

    def dispatchDomainEventCallbacks(self, dom, event, detail):
        """Dispatches events to python user domain event callbacks
        """
        try:
            for cb,opaque in self.domainEventCallbacks.items():
                cb(self,dom,event,detail,opaque)
            return 0
        except AttributeError:
            pass

    def dispatchDomainEventLifecycleCallback(self, dom, event, detail, cbData):
        """Dispatches events to python user domain lifecycle event callbacks
        """
        cb = cbData["cb"]
        opaque = cbData["opaque"]

        cb(self, virDomain(self, _obj=dom), event, detail, opaque)
        return 0

    def dispatchDomainEventGenericCallback(self, dom, cbData):
        """Dispatches events to python user domain generic event callbacks
        """
        try:
            cb = cbData["cb"]
            opaque = cbData["opaque"]

            cb(self, virDomain(self, _obj=dom), opaque)
            return 0
        except AttributeError:
            pass

    def dispatchDomainEventRTCChangeCallback(self, dom, offset, cbData):
        """Dispatches events to python user domain RTC change event callbacks
        """
        try:
            cb = cbData["cb"]
            opaque = cbData["opaque"]

            cb(self, virDomain(self, _obj=dom), offset ,opaque)
            return 0
        except AttributeError:
            pass

    def dispatchDomainEventWatchdogCallback(self, dom, action, cbData):
        """Dispatches events to python user domain watchdog event callbacks
        """
        try:
            cb = cbData["cb"]
            opaque = cbData["opaque"]

            cb(self, virDomain(self, _obj=dom), action, opaque)
            return 0
        except AttributeError:
            pass

    def dispatchDomainEventIOErrorCallback(self, dom, srcPath, devAlias, action, cbData):
        """Dispatches events to python user domain IO error event callbacks
        """
        try:
            cb = cbData["cb"]
            opaque = cbData["opaque"]

            cb(self, virDomain(self, _obj=dom), srcPath, devAlias, action, opaque)
            return 0
        except AttributeError:
            pass

    def dispatchDomainEventIOErrorReasonCallback(self, dom, srcPath, devAlias, action, reason, cbData):
        """Dispatches events to python user domain IO error event callbacks
        """
        try:
            cb = cbData["cb"]
            opaque = cbData["opaque"]

            cb(self, virDomain(self, _obj=dom), srcPath, devAlias, action, reason, opaque)
            return 0
        except AttributeError:
            pass

    def dispatchDomainEventGraphicsCallback(self, dom, phase, localAddr, remoteAddr, authScheme, subject, cbData):
        """Dispatches events to python user domain graphics event callbacks
        """
        try:
            cb = cbData["cb"]
            opaque = cbData["opaque"]

            cb(self, virDomain(self, _obj=dom), phase, localAddr, remoteAddr, authScheme, subject, opaque)
            return 0
        except AttributeError:
            pass

    def domainEventDeregisterAny(self, callbackID):
        """Removes a Domain Event Callback. De-registering for a
           domain callback will disable delivery of this event type """
        try:
            ret = libvirtmod.virConnectDomainEventDeregisterAny(self._o, callbackID)
            if ret == -1: raise libvirtError ('virConnectDomainEventDeregisterAny() failed', conn=self)
            del self.domainEventCallbackID[callbackID]
        except AttributeError:
            pass

    def domainEventRegisterAny(self, dom, eventID, cb, opaque):
        """Adds a Domain Event Callback. Registering for a domain
           callback will enable delivery of the events """
        if not hasattr(self, 'domainEventCallbackID'):
            self.domainEventCallbackID = {}
        cbData = { "cb": cb, "conn": self, "opaque": opaque }
        if dom is None:
            ret = libvirtmod.virConnectDomainEventRegisterAny(self._o, None, eventID, cbData)
        else:
            ret = libvirtmod.virConnectDomainEventRegisterAny(self._o, dom._o, eventID, cbData)
        if ret == -1:
            raise libvirtError ('virConnectDomainEventRegisterAny() failed', conn=self)
        self.domainEventCallbackID[ret] = opaque
        return ret

class virNodeDevice:
    def __init__(self, conn, _obj=None):
        self._conn = conn
        if _obj != None:self._o = _obj;return
        self._o = None

    def __del__(self):
        if self._o != None:
            libvirtmod.virNodeDeviceFree(self._o)
        self._o = None

    def connect(self):
        return self._conn

    #
    # virNodeDevice functions from module libvirt
    #

    def XMLDesc(self, flags):
        """Fetch an XML document describing all aspects of
        the device. """
        ret = libvirtmod.virNodeDeviceGetXMLDesc(self._o, flags)
        if ret is None: raise libvirtError ('virNodeDeviceGetXMLDesc() failed')
        return ret

    def destroy(self):
        """Destroy the device object. The virtual device is removed from the host operating system.
        This function may require privileged access """
        ret = libvirtmod.virNodeDeviceDestroy(self._o)
        if ret == -1: raise libvirtError ('virNodeDeviceDestroy() failed')
        return ret

    def dettach(self):
        """Dettach the node device from the node itself so that it may be
        assigned to a guest domain.
        
        Depending on the hypervisor, this may involve operations such
        as unbinding any device drivers from the device, binding the
        device to a dummy device driver and resetting the device.
        
        If the device is currently in use by the node, this method may
        fail.
        
        Once the device is not assigned to any guest, it may be re-attached
        to the node using the virNodeDeviceReattach() method. """
        ret = libvirtmod.virNodeDeviceDettach(self._o)
        if ret == -1: raise libvirtError ('virNodeDeviceDettach() failed')
        return ret

    def name(self):
        """Just return the device name """
        ret = libvirtmod.virNodeDeviceGetName(self._o)
        return ret

    def numOfCaps(self):
        """Accessor for the number of capabilities supported by the device. """
        ret = libvirtmod.virNodeDeviceNumOfCaps(self._o)
        if ret == -1: raise libvirtError ('virNodeDeviceNumOfCaps() failed')
        return ret

    def parent(self):
        """Accessor for the parent of the device """
        ret = libvirtmod.virNodeDeviceGetParent(self._o)
        return ret

    def reAttach(self):
        """Re-attach a previously dettached node device to the node so that it
        may be used by the node again.
        
        Depending on the hypervisor, this may involve operations such
        as resetting the device, unbinding it from a dummy device driver
        and binding it to its appropriate driver.
        
        If the device is currently in use by a guest, this method may fail. """
        ret = libvirtmod.virNodeDeviceReAttach(self._o)
        if ret == -1: raise libvirtError ('virNodeDeviceReAttach() failed')
        return ret

    def reset(self):
        """Reset a previously dettached node device to the node before or
        after assigning it to a guest.
        
        The exact reset semantics depends on the hypervisor and device
        type but, for example, KVM will attempt to reset PCI devices with
        a Function Level Reset, Secondary Bus Reset or a Power Management
        D-State reset.
        
        If the reset will affect other devices which are currently in use,
        this function may fail. """
        ret = libvirtmod.virNodeDeviceReset(self._o)
        if ret == -1: raise libvirtError ('virNodeDeviceReset() failed')
        return ret

    #
    # virNodeDevice functions from module python
    #

    def listCaps(self):
        """list the node device's capabilities """
        ret = libvirtmod.virNodeDeviceListCaps(self._o)
        if ret is None: raise libvirtError ('virNodeDeviceListCaps() failed')
        return ret

class virSecret:
    def __init__(self, conn, _obj=None):
        self._conn = conn
        if _obj != None:self._o = _obj;return
        self._o = None

    def __del__(self):
        if self._o != None:
            libvirtmod.virSecretFree(self._o)
        self._o = None

    def connect(self):
        return self._conn

    #
    # virSecret functions from module libvirt
    #

    def XMLDesc(self, flags):
        """Fetches an XML document describing attributes of the secret. """
        ret = libvirtmod.virSecretGetXMLDesc(self._o, flags)
        if ret is None: raise libvirtError ('virSecretGetXMLDesc() failed')
        return ret

    def setValue(self, value, flags):
        """Associates a value with a secret. """
        ret = libvirtmod.virSecretSetValue(self._o, value, flags)
        if ret == -1: raise libvirtError ('virSecretSetValue() failed')
        return ret

    def undefine(self):
        """Deletes the specified secret.  This does not free the associated
        virSecretPtr object. """
        ret = libvirtmod.virSecretUndefine(self._o)
        if ret == -1: raise libvirtError ('virSecretUndefine() failed')
        return ret

    def usageID(self):
        """Get the unique identifier of the object with which this
        secret is to be used. The format of the identifier is
        dependant on the usage type of the secret. For a secret
        with a usage type of VIR_SECRET_USAGE_TYPE_VOLUME the
        identifier will be a fully qualfied path name. The
        identifiers are intended to be unique within the set of
        all secrets sharing the same usage type. ie, there shall
        only ever be one secret for each volume path. """
        ret = libvirtmod.virSecretGetUsageID(self._o)
        return ret

    def usageType(self):
        """Get the type of object which uses this secret. The returned
        value is one of the constants defined in the virSecretUsageType
        enumeration. More values may be added to this enumeration in
        the future, so callers should expect to see usage types they
        do not explicitly know about. """
        ret = libvirtmod.virSecretGetUsageType(self._o)
        return ret

    def value(self, flags):
        """Fetches the value associated with a secret. """
        ret = libvirtmod.virSecretGetValue(self._o, flags)
        if ret is None: raise libvirtError ('virSecretGetValue() failed')
        return ret

    #
    # virSecret functions from module python
    #

    def UUID(self):
        """Extract the UUID unique Identifier of a secret. """
        ret = libvirtmod.virSecretGetUUID(self._o)
        if ret is None: raise libvirtError ('virSecretGetUUID() failed')
        return ret

    def UUIDString(self):
        """Fetch globally unique ID of the secret as a string. """
        ret = libvirtmod.virSecretGetUUIDString(self._o)
        if ret is None: raise libvirtError ('virSecretGetUUIDString() failed')
        return ret

class virNWFilter:
    def __init__(self, conn, _obj=None):
        self._conn = conn
        if _obj != None:self._o = _obj;return
        self._o = None

    def __del__(self):
        if self._o != None:
            libvirtmod.virNWFilterFree(self._o)
        self._o = None

    def connect(self):
        return self._conn

    #
    # virNWFilter functions from module libvirt
    #

    def XMLDesc(self, flags):
        """Provide an XML description of the network filter. The description may be
        reused later to redefine the network filter with virNWFilterCreateXML(). """
        ret = libvirtmod.virNWFilterGetXMLDesc(self._o, flags)
        if ret is None: raise libvirtError ('virNWFilterGetXMLDesc() failed')
        return ret

    def name(self):
        """Get the public name for the network filter """
        ret = libvirtmod.virNWFilterGetName(self._o)
        return ret

    def undefine(self):
        """Undefine the nwfilter object. This call will not succeed if
        a running VM is referencing the filter. This does not free the
        associated virNWFilterPtr object. """
        ret = libvirtmod.virNWFilterUndefine(self._o)
        if ret == -1: raise libvirtError ('virNWFilterUndefine() failed')
        return ret

    #
    # virNWFilter functions from module python
    #

    def UUID(self):
        """Extract the UUID unique Identifier of a network filter. """
        ret = libvirtmod.virNWFilterGetUUID(self._o)
        if ret is None: raise libvirtError ('virNWFilterGetUUID() failed')
        return ret

    def UUIDString(self):
        """Fetch globally unique ID of the network filter as a string. """
        ret = libvirtmod.virNWFilterGetUUIDString(self._o)
        if ret is None: raise libvirtError ('virNWFilterGetUUIDString() failed')
        return ret

class virStream:
    def __init__(self, conn, _obj=None):
        self._conn = conn
        if _obj != None:self._o = _obj;return
        self._o = None

    def connect(self):
        return self._conn

    #
    # virStream functions from module libvirt
    #

    def abort(self):
        """Request that the in progress data transfer be cancelled
        abnormally before the end of the stream has been reached.
        For output streams this can be used to inform the driver
        that the stream is being terminated early. For input
        streams this can be used to inform the driver that it
        should stop sending data. """
        ret = libvirtmod.virStreamAbort(self._o)
        if ret == -1: raise libvirtError ('virStreamAbort() failed')
        return ret

    def eventRemoveCallback(self):
        """Remove an event callback from the stream """
        ret = libvirtmod.virStreamEventRemoveCallback(self._o)
        if ret == -1: raise libvirtError ('virStreamEventRemoveCallback() failed')
        return ret

    def eventUpdateCallback(self, events):
        """Changes the set of events to monitor for a stream. This allows
        for event notification to be changed without having to
        unregister & register the callback completely. This method
        is guarenteed to succeed if a callback is already registered """
        ret = libvirtmod.virStreamEventUpdateCallback(self._o, events)
        if ret == -1: raise libvirtError ('virStreamEventUpdateCallback() failed')
        return ret

    def finish(self):
        """Indicate that there is no further data is to be transmitted
        on the stream. For output streams this should be called once
        all data has been written. For input streams this should be
        called once virStreamRecv returns end-of-file.
        
        This method is a synchronization point for all asynchronous
        errors, so if this returns a success code the application can
        be sure that all data has been successfully processed. """
        ret = libvirtmod.virStreamFinish(self._o)
        if ret == -1: raise libvirtError ('virStreamFinish() failed')
        return ret

    def recv(self, data, nbytes):
        """Write a series of bytes to the stream. This method may
        block the calling application for an arbitrary amount
        of time.
        
        Errors are not guaranteed to be reported synchronously
        with the call, but may instead be delayed until a
        subsequent call.
        
        An example using this with a hypothetical file download
        API looks like
        
          virStreamPtr st = virStreamNew(conn, 0);
          int fd = open("demo.iso", O_WRONLY, 0600)
        
          virConnectDownloadFile(conn, "demo.iso", st);
        
          while (1) {
              char buf[1024];
              int got = virStreamRecv(st, buf, 1024);
              if (got < 0)
                 break;
              if (got == 0) {
                 virStreamFinish(st);
                 break;
              }
              int offset = 0;
              while (offset < got) {
                 int sent = write(fd, buf+offset, got-offset)
                 if (sent < 0) {
                    virStreamAbort(st);
                    goto done;
                 }
                 offset += sent;
              }
          }
          if (virStreamFinish(st) < 0)
             ... report an error ....
        done:
          virStreamFree(st);
          close(fd); """
        ret = libvirtmod.virStreamRecv(self._o, data, nbytes)
        if ret == -1: raise libvirtError ('virStreamRecv() failed')
        return ret

    def send(self, data, nbytes):
        """Write a series of bytes to the stream. This method may
        block the calling application for an arbitrary amount
        of time. Once an application has finished sending data
        it should call virStreamFinish to wait for successful
        confirmation from the driver, or detect any error
        
        This method may not be used if a stream source has been
        registered
        
        Errors are not guaranteed to be reported synchronously
        with the call, but may instead be delayed until a
        subsequent call.
        
        An example using this with a hypothetical file upload
        API looks like
        
          virStreamPtr st = virStreamNew(conn, 0);
          int fd = open("demo.iso", O_RDONLY)
        
          virConnectUploadFile(conn, "demo.iso", st);
        
          while (1) {
              char buf[1024];
              int got = read(fd, buf, 1024);
              if (got < 0) {
                 virStreamAbort(st);
                 break;
              }
              if (got == 0) {
                 virStreamFinish(st);
                 break;
              }
              int offset = 0;
              while (offset < got) {
                 int sent = virStreamSend(st, buf+offset, got-offset)
                 if (sent < 0) {
                    virStreamAbort(st);
                    goto done;
                 }
                 offset += sent;
              }
          }
          if (virStreamFinish(st) < 0)
             ... report an error ....
        done:
          virStreamFree(st);
          close(fd); """
        ret = libvirtmod.virStreamSend(self._o, data, nbytes)
        if ret == -1: raise libvirtError ('virStreamSend() failed')
        return ret

    #
    # virStream methods from virStream.py (hand coded)
    #
    def __del__(self):
        try:
            if self.cb:
                libvirtmod.virStreamEventRemoveCallback(self._o)
        except AttributeError:
           pass

        if self._o != None:
            libvirtmod.virStreamFree(self._o)
        self._o = None

    def eventAddCallback(self, cb, opaque):
        """ """
        try:
            self.cb = cb
            self.opaque = opaque
            ret = libvirtmod.virStreamEventAddCallback(self._o, self)
            if ret == -1: raise libvirtError ('virStreamEventAddCallback() failed', conn=self._conn)
        except AttributeError:
            pass

class virDomainSnapshot:
    def __init__(self, dom, _obj=None):
        self._dom = dom
        if _obj != None:self._o = _obj;return
        self._o = None

    def __del__(self):
        if self._o != None:
            libvirtmod.virDomainSnapshotFree(self._o)
        self._o = None

    def domain(self):
        return self._dom

    #
    # virDomainSnapshot functions from module libvirt
    #

    def delete(self, flags):
        """Delete the snapshot.
        
        If @flags is 0, then just this snapshot is deleted, and changes from
        this snapshot are automatically merged into children snapshots.  If
        flags is VIR_DOMAIN_SNAPSHOT_DELETE_CHILDREN, then this snapshot
        and any children snapshots are deleted. """
        ret = libvirtmod.virDomainSnapshotDelete(self._o, flags)
        if ret == -1: raise libvirtError ('virDomainSnapshotDelete() failed')
        return ret

    def getXMLDesc(self, flags):
        """Provide an XML description of the domain snapshot. """
        ret = libvirtmod.virDomainSnapshotGetXMLDesc(self._o, flags)
        if ret is None: raise libvirtError ('virDomainSnapshotGetXMLDesc() failed')
        return ret

# virStreamFlags
VIR_STREAM_NONBLOCK = 1

# virCPUCompareResult
VIR_CPU_COMPARE_ERROR = -1
VIR_CPU_COMPARE_INCOMPATIBLE = 0
VIR_CPU_COMPARE_IDENTICAL = 1
VIR_CPU_COMPARE_SUPERSET = 2

# virErrorNumber
VIR_ERR_OK = 0
VIR_ERR_INTERNAL_ERROR = 1
VIR_ERR_NO_MEMORY = 2
VIR_ERR_NO_SUPPORT = 3
VIR_ERR_UNKNOWN_HOST = 4
VIR_ERR_NO_CONNECT = 5
VIR_ERR_INVALID_CONN = 6
VIR_ERR_INVALID_DOMAIN = 7
VIR_ERR_INVALID_ARG = 8
VIR_ERR_OPERATION_FAILED = 9
VIR_ERR_GET_FAILED = 10
VIR_ERR_POST_FAILED = 11
VIR_ERR_HTTP_ERROR = 12
VIR_ERR_SEXPR_SERIAL = 13
VIR_ERR_NO_XEN = 14
VIR_ERR_XEN_CALL = 15
VIR_ERR_OS_TYPE = 16
VIR_ERR_NO_KERNEL = 17
VIR_ERR_NO_ROOT = 18
VIR_ERR_NO_SOURCE = 19
VIR_ERR_NO_TARGET = 20
VIR_ERR_NO_NAME = 21
VIR_ERR_NO_OS = 22
VIR_ERR_NO_DEVICE = 23
VIR_ERR_NO_XENSTORE = 24
VIR_ERR_DRIVER_FULL = 25
VIR_ERR_CALL_FAILED = 26
VIR_ERR_XML_ERROR = 27
VIR_ERR_DOM_EXIST = 28
VIR_ERR_OPERATION_DENIED = 29
VIR_ERR_OPEN_FAILED = 30
VIR_ERR_READ_FAILED = 31
VIR_ERR_PARSE_FAILED = 32
VIR_ERR_CONF_SYNTAX = 33
VIR_ERR_WRITE_FAILED = 34
VIR_ERR_XML_DETAIL = 35
VIR_ERR_INVALID_NETWORK = 36
VIR_ERR_NETWORK_EXIST = 37
VIR_ERR_SYSTEM_ERROR = 38
VIR_ERR_RPC = 39
VIR_ERR_GNUTLS_ERROR = 40
VIR_WAR_NO_NETWORK = 41
VIR_ERR_NO_DOMAIN = 42
VIR_ERR_NO_NETWORK = 43
VIR_ERR_INVALID_MAC = 44
VIR_ERR_AUTH_FAILED = 45
VIR_ERR_INVALID_STORAGE_POOL = 46
VIR_ERR_INVALID_STORAGE_VOL = 47
VIR_WAR_NO_STORAGE = 48
VIR_ERR_NO_STORAGE_POOL = 49
VIR_ERR_NO_STORAGE_VOL = 50
VIR_WAR_NO_NODE = 51
VIR_ERR_INVALID_NODE_DEVICE = 52
VIR_ERR_NO_NODE_DEVICE = 53
VIR_ERR_NO_SECURITY_MODEL = 54
VIR_ERR_OPERATION_INVALID = 55
VIR_WAR_NO_INTERFACE = 56
VIR_ERR_NO_INTERFACE = 57
VIR_ERR_INVALID_INTERFACE = 58
VIR_ERR_MULTIPLE_INTERFACES = 59
VIR_WAR_NO_NWFILTER = 60
VIR_ERR_INVALID_NWFILTER = 61
VIR_ERR_NO_NWFILTER = 62
VIR_ERR_BUILD_FIREWALL = 63
VIR_WAR_NO_SECRET = 64
VIR_ERR_INVALID_SECRET = 65
VIR_ERR_NO_SECRET = 66
VIR_ERR_CONFIG_UNSUPPORTED = 67
VIR_ERR_OPERATION_TIMEOUT = 68
VIR_ERR_MIGRATE_PERSIST_FAILED = 69
VIR_ERR_HOOK_SCRIPT_FAILED = 70
VIR_ERR_INVALID_DOMAIN_SNAPSHOT = 71
VIR_ERR_NO_DOMAIN_SNAPSHOT = 72

# virDomainMigrateFlags
VIR_MIGRATE_LIVE = 1
VIR_MIGRATE_PEER2PEER = 2
VIR_MIGRATE_TUNNELLED = 4
VIR_MIGRATE_PERSIST_DEST = 8
VIR_MIGRATE_UNDEFINE_SOURCE = 16
VIR_MIGRATE_PAUSED = 32
VIR_MIGRATE_NON_SHARED_DISK = 64
VIR_MIGRATE_NON_SHARED_INC = 128

# virStorageVolType
VIR_STORAGE_VOL_FILE = 0
VIR_STORAGE_VOL_BLOCK = 1

# virErrorLevel
VIR_ERR_NONE = 0
VIR_ERR_WARNING = 1
VIR_ERR_ERROR = 2

# virDomainEventResumedDetailType
VIR_DOMAIN_EVENT_RESUMED_UNPAUSED = 0
VIR_DOMAIN_EVENT_RESUMED_MIGRATED = 1

# virErrorDomain
VIR_FROM_NONE = 0
VIR_FROM_XEN = 1
VIR_FROM_XEND = 2
VIR_FROM_XENSTORE = 3
VIR_FROM_SEXPR = 4
VIR_FROM_XML = 5
VIR_FROM_DOM = 6
VIR_FROM_RPC = 7
VIR_FROM_PROXY = 8
VIR_FROM_CONF = 9
VIR_FROM_QEMU = 10
VIR_FROM_NET = 11
VIR_FROM_TEST = 12
VIR_FROM_REMOTE = 13
VIR_FROM_OPENVZ = 14
VIR_FROM_XENXM = 15
VIR_FROM_STATS_LINUX = 16
VIR_FROM_LXC = 17
VIR_FROM_STORAGE = 18
VIR_FROM_NETWORK = 19
VIR_FROM_DOMAIN = 20
VIR_FROM_UML = 21
VIR_FROM_NODEDEV = 22
VIR_FROM_XEN_INOTIFY = 23
VIR_FROM_SECURITY = 24
VIR_FROM_VBOX = 25
VIR_FROM_INTERFACE = 26
VIR_FROM_ONE = 27
VIR_FROM_ESX = 28
VIR_FROM_PHYP = 29
VIR_FROM_SECRET = 30
VIR_FROM_CPU = 31
VIR_FROM_XENAPI = 32
VIR_FROM_NWFILTER = 33
VIR_FROM_HOOK = 34
VIR_FROM_DOMAIN_SNAPSHOT = 35
VIR_FROM_AUDIT = 36
VIR_FROM_SYSINFO = 37
VIR_FROM_STREAMS = 38
VIR_FROM_VMWARE = 39

# virDomainEventStartedDetailType
VIR_DOMAIN_EVENT_STARTED_BOOTED = 0
VIR_DOMAIN_EVENT_STARTED_MIGRATED = 1
VIR_DOMAIN_EVENT_STARTED_RESTORED = 2
VIR_DOMAIN_EVENT_STARTED_FROM_SNAPSHOT = 3

# virDomainMemoryStatTags
VIR_DOMAIN_MEMORY_STAT_SWAP_IN = 0
VIR_DOMAIN_MEMORY_STAT_SWAP_OUT = 1
VIR_DOMAIN_MEMORY_STAT_MAJOR_FAULT = 2
VIR_DOMAIN_MEMORY_STAT_MINOR_FAULT = 3
VIR_DOMAIN_MEMORY_STAT_UNUSED = 4
VIR_DOMAIN_MEMORY_STAT_AVAILABLE = 5
VIR_DOMAIN_MEMORY_STAT_NR = 6

# virEventHandleType
VIR_EVENT_HANDLE_READABLE = 1
VIR_EVENT_HANDLE_WRITABLE = 2
VIR_EVENT_HANDLE_ERROR = 4
VIR_EVENT_HANDLE_HANGUP = 8

# virDomainEventType
VIR_DOMAIN_EVENT_DEFINED = 0
VIR_DOMAIN_EVENT_UNDEFINED = 1
VIR_DOMAIN_EVENT_STARTED = 2
VIR_DOMAIN_EVENT_SUSPENDED = 3
VIR_DOMAIN_EVENT_RESUMED = 4
VIR_DOMAIN_EVENT_STOPPED = 5

# virStreamEventType
VIR_STREAM_EVENT_READABLE = 1
VIR_STREAM_EVENT_WRITABLE = 2
VIR_STREAM_EVENT_ERROR = 4
VIR_STREAM_EVENT_HANGUP = 8

# virDomainState
VIR_DOMAIN_NOSTATE = 0
VIR_DOMAIN_RUNNING = 1
VIR_DOMAIN_BLOCKED = 2
VIR_DOMAIN_PAUSED = 3
VIR_DOMAIN_SHUTDOWN = 4
VIR_DOMAIN_SHUTOFF = 5
VIR_DOMAIN_CRASHED = 6

# virDomainVcpuFlags
VIR_DOMAIN_VCPU_LIVE = 1
VIR_DOMAIN_VCPU_CONFIG = 2
VIR_DOMAIN_VCPU_MAXIMUM = 4

# virStoragePoolDeleteFlags
VIR_STORAGE_POOL_DELETE_NORMAL = 0
VIR_STORAGE_POOL_DELETE_ZEROED = 1

# virDomainCoreDumpFlags
VIR_DUMP_CRASH = 1
VIR_DUMP_LIVE = 2

# virDomainEventIOErrorAction
VIR_DOMAIN_EVENT_IO_ERROR_NONE = 0
VIR_DOMAIN_EVENT_IO_ERROR_PAUSE = 1
VIR_DOMAIN_EVENT_IO_ERROR_REPORT = 2

# virSchedParameterType
VIR_DOMAIN_SCHED_FIELD_INT = 1
VIR_DOMAIN_SCHED_FIELD_UINT = 2
VIR_DOMAIN_SCHED_FIELD_LLONG = 3
VIR_DOMAIN_SCHED_FIELD_ULLONG = 4
VIR_DOMAIN_SCHED_FIELD_DOUBLE = 5
VIR_DOMAIN_SCHED_FIELD_BOOLEAN = 6

# virDomainDeviceModifyFlags
VIR_DOMAIN_DEVICE_MODIFY_CURRENT = 0
VIR_DOMAIN_DEVICE_MODIFY_LIVE = 1
VIR_DOMAIN_DEVICE_MODIFY_CONFIG = 2
VIR_DOMAIN_DEVICE_MODIFY_FORCE = 4

# virInterfaceXMLFlags
VIR_INTERFACE_XML_INACTIVE = 1

# virDomainSnapshotDeleteFlags
VIR_DOMAIN_SNAPSHOT_DELETE_CHILDREN = 1

# virConnectFlags
VIR_CONNECT_RO = 1

# virDomainEventDefinedDetailType
VIR_DOMAIN_EVENT_DEFINED_ADDED = 0
VIR_DOMAIN_EVENT_DEFINED_UPDATED = 1

# virDomainEventID
VIR_DOMAIN_EVENT_ID_LIFECYCLE = 0
VIR_DOMAIN_EVENT_ID_REBOOT = 1
VIR_DOMAIN_EVENT_ID_RTC_CHANGE = 2
VIR_DOMAIN_EVENT_ID_WATCHDOG = 3
VIR_DOMAIN_EVENT_ID_IO_ERROR = 4
VIR_DOMAIN_EVENT_ID_GRAPHICS = 5
VIR_DOMAIN_EVENT_ID_IO_ERROR_REASON = 6
VIR_DOMAIN_EVENT_ID_LAST = 7

# virDomainMemoryFlags
VIR_MEMORY_VIRTUAL = 1
VIR_MEMORY_PHYSICAL = 2

# virDomainEventStoppedDetailType
VIR_DOMAIN_EVENT_STOPPED_SHUTDOWN = 0
VIR_DOMAIN_EVENT_STOPPED_DESTROYED = 1
VIR_DOMAIN_EVENT_STOPPED_CRASHED = 2
VIR_DOMAIN_EVENT_STOPPED_MIGRATED = 3
VIR_DOMAIN_EVENT_STOPPED_SAVED = 4
VIR_DOMAIN_EVENT_STOPPED_FAILED = 5
VIR_DOMAIN_EVENT_STOPPED_FROM_SNAPSHOT = 6

# virVcpuState
VIR_VCPU_OFFLINE = 0
VIR_VCPU_RUNNING = 1
VIR_VCPU_BLOCKED = 2

# virStorageVolDeleteFlags
VIR_STORAGE_VOL_DELETE_NORMAL = 0
VIR_STORAGE_VOL_DELETE_ZEROED = 1

# virDomainEventGraphicsPhase
VIR_DOMAIN_EVENT_GRAPHICS_CONNECT = 0
VIR_DOMAIN_EVENT_GRAPHICS_INITIALIZE = 1
VIR_DOMAIN_EVENT_GRAPHICS_DISCONNECT = 2

# virDomainEventSuspendedDetailType
VIR_DOMAIN_EVENT_SUSPENDED_PAUSED = 0
VIR_DOMAIN_EVENT_SUSPENDED_MIGRATED = 1
VIR_DOMAIN_EVENT_SUSPENDED_IOERROR = 2
VIR_DOMAIN_EVENT_SUSPENDED_WATCHDOG = 3

# virStoragePoolBuildFlags
VIR_STORAGE_POOL_BUILD_NEW = 0
VIR_STORAGE_POOL_BUILD_REPAIR = 1
VIR_STORAGE_POOL_BUILD_RESIZE = 2

# virDomainXMLFlags
VIR_DOMAIN_XML_SECURE = 1
VIR_DOMAIN_XML_INACTIVE = 2
VIR_DOMAIN_XML_UPDATE_CPU = 4

# virDomainJobType
VIR_DOMAIN_JOB_NONE = 0
VIR_DOMAIN_JOB_BOUNDED = 1
VIR_DOMAIN_JOB_UNBOUNDED = 2
VIR_DOMAIN_JOB_COMPLETED = 3
VIR_DOMAIN_JOB_FAILED = 4
VIR_DOMAIN_JOB_CANCELLED = 5

# virDomainEventUndefinedDetailType
VIR_DOMAIN_EVENT_UNDEFINED_REMOVED = 0

# virConnectCredentialType
VIR_CRED_USERNAME = 1
VIR_CRED_AUTHNAME = 2
VIR_CRED_LANGUAGE = 3
VIR_CRED_CNONCE = 4
VIR_CRED_PASSPHRASE = 5
VIR_CRED_ECHOPROMPT = 6
VIR_CRED_NOECHOPROMPT = 7
VIR_CRED_REALM = 8
VIR_CRED_EXTERNAL = 9

# virStoragePoolState
VIR_STORAGE_POOL_INACTIVE = 0
VIR_STORAGE_POOL_BUILDING = 1
VIR_STORAGE_POOL_RUNNING = 2
VIR_STORAGE_POOL_DEGRADED = 3
VIR_STORAGE_POOL_INACCESSIBLE = 4

# virDomainEventGraphicsAddressType
VIR_DOMAIN_EVENT_GRAPHICS_ADDRESS_IPV4 = 1
VIR_DOMAIN_EVENT_GRAPHICS_ADDRESS_IPV6 = 2

# virDomainCreateFlags
VIR_DOMAIN_NONE = 0
VIR_DOMAIN_START_PAUSED = 1

# virMemoryParameterType
VIR_DOMAIN_MEMORY_PARAM_INT = 1
VIR_DOMAIN_MEMORY_PARAM_UINT = 2
VIR_DOMAIN_MEMORY_PARAM_LLONG = 3
VIR_DOMAIN_MEMORY_PARAM_ULLONG = 4
VIR_DOMAIN_MEMORY_PARAM_DOUBLE = 5
VIR_DOMAIN_MEMORY_PARAM_BOOLEAN = 6

# virSecretUsageType
VIR_SECRET_USAGE_TYPE_NONE = 0
VIR_SECRET_USAGE_TYPE_VOLUME = 1

# virDomainEventWatchdogAction
VIR_DOMAIN_EVENT_WATCHDOG_NONE = 0
VIR_DOMAIN_EVENT_WATCHDOG_PAUSE = 1
VIR_DOMAIN_EVENT_WATCHDOG_RESET = 2
VIR_DOMAIN_EVENT_WATCHDOG_POWEROFF = 3
VIR_DOMAIN_EVENT_WATCHDOG_SHUTDOWN = 4
VIR_DOMAIN_EVENT_WATCHDOG_DEBUG = 5

