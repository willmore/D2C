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
