
class SourceImage(object):

    def __init__(self, id, image):
        
        assert id is None or isinstance(id, int)
        assert image is None or isinstance(image, Image)
        
        self.id = id
        self.image = image
        
class DesktopImage(SourceImage):
    
    def __init__(self, id, image, path):
        SourceImage.__init__(self, id, image)
        self.path = path

    def getDisplayName(self):
        return "%s:%s" % (self.image.name, self.path)
        
class Image(object):
    
    def __init__(self, id, name, originalImage, reals=()):
        
        assert id is None or isinstance(id, int)
        assert name is None or isinstance(name, basestring)
        
        self.id = id
        self.name = name
        self.originalImage = originalImage
        self.reals = list(reals)