'''
A utility class which installs collectd in supplied images.
'''
import libvirt

from .SourceImage import SourceImage
import libvirtmod
import re

class CollectdInstaller(object):
    '''
    TODO this class is not operable yet.
    '''
    def __init__(self):
        pass
    
    def install(self, img):

        assert isinstance(img, SourceImage)
      
        vConn = libvirt.open("vbox:///session")
        print vConn.numOfDefinedDomains()
      
        for d in vConn.listDefinedDomains():
            dom = vConn.lookupByName(d)
            if self._hasDisk(img.path, dom.XMLDesc(0)):
                print d
                print "Launching..."
                dom.create()
            
    def _hasDisk(self, diskFile, descriptionXml):
        #print descriptionXml
        if re.search(diskFile, descriptionXml):
            return True
        else:
            return False
    
