from .Cloud import Cloud


class SourceImage(object):

    def __init__(self, id, image, cloud, dateAdded, size):
        
        assert id is None or isinstance(id, int)
        assert image is None or isinstance(image, Image)
        assert isinstance(cloud, Cloud)
        
        self.id = id
        self.image = image
        self.cloud = cloud
        self.dateAdded = dateAdded
        self.size = size
        
class DesktopImage(SourceImage):
    
    def __init__(self, id, image, cloud, dateAdded, size, sizeOnDisk, path):
        SourceImage.__init__(self, id, image, cloud, dateAdded, size)
        
        assert isinstance(path, basestring)
        self.sizeOnDisk = sizeOnDisk
        self.path = path

    def getDisplayName(self):
        return "%s:%s" % (self.image.name, self.path)
        
class Image(object):
    
    def __init__(self, id, name, originalImage, reals=()):
        
        assert id is None or isinstance(id, int)
        assert name is None or isinstance(name, basestring)
        assert isinstance(originalImage, SourceImage)
        
        self.id = id
        self.name = name
        self.originalImage = originalImage
        self.reals = list(reals)
        
        if self.originalImage not in self.reals:
            self.reals.append(self.originalImage)
        
    def getHostedSourceImage(self, cloud):
        '''
        Return the real image that is hosted on the cloud.
        '''
    
        assert isinstance(cloud, Cloud)
        
        for real in self.reals:
            if real.cloud is cloud:
                return real
        
        return None
    
class AMI(SourceImage):
    
    def __init__(self, id, image, amiId, cloud, dateAdded, size, kernel=None, ramdisk=None):
        SourceImage.__init__(self, id, image, cloud, dateAdded, size)
        self.amiId = amiId
        self.ramdisk = ramdisk
        self.kernel = kernel
        
    def getDisplayName(self):
        return "%s:%s" % (self.image.name, self.amiId)