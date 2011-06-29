
class AMI(object):
    
    def __init__(self, id, cloud, srcImg=None, kernel=None, ramdisk=None):
        self.id = id
        self.cloud = cloud
        self.srcImg = srcImg
        self.ramdisk = ramdisk
        self.kernel = kernel