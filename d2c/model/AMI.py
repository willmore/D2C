from .SourceImage import SourceImage

class AMI(SourceImage):
    
    def __init__(self, id, image, amiId, cloud, kernel=None, ramdisk=None):
        SourceImage.__init__(self, id, image)
        self.amiId = amiId
        self.cloud = cloud
        self.ramdisk = ramdisk
        self.kernel = kernel