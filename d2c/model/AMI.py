from .SourceImage import SourceImage

class AMI(SourceImage):
    
    def __init__(self, id, image, amiId, cloud, kernel=None, ramdisk=None):
        SourceImage.__init__(self, id, image, cloud)
        self.amiId = amiId
        self.ramdisk = ramdisk
        self.kernel = kernel
        
    def getDisplayName(self):
        return "%s:%s" % (self.image.name, self.amiId)