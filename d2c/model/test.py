import threading
import types
import libvirt

class Bob(object):
    
    def __init__(self):
        self.baz = "Baz value"
    
    def hah(self, v1, v2):
        print "self = %s" % type(self) 
        print "Har har %s %s" % (v1, v2) 

class Foo(object):
    
    def __init__(self, proxy):
        self.proxy = proxy
        self.lock = threading.RLock()
    
    def __getattribute__(self, attrName):
        if attrName is "proxy" or attrName is "lock":
            return object.__getattribute__(self, attrName)
            
        #print "Type = %s" % type(self.proxy.__getattribute__(attrName))
        attr = getattr(self.proxy, attrName)
        if isinstance(attr, types.MethodType):
            def lockedFunc(*args, **kwargs):
                self.lock.acquire()
                v = attr(*args, **kwargs)
                self.lock.release()
                return v
            return lockedFunc
        return attr
    
    def __setattr__(self, name, value):
        if name is "proxy" or name is "lock":
            object.__setattr__(self, name, value)
        else:
            self.proxy.__dict__[name] = value
        
        
conn = libvirt.open("vbox:///session")
f = Foo(conn)
f.baz
f.hah('a', v2='b')
    
