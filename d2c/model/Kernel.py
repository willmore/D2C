import re
import pkg_resources

class Kernel(object):
    
    
    def __init__(self, aki, architecture, 
                 contents, cloud=None, recommendedRamdisk=None, isPvGrub=False):
        self.aki = aki
        self.architecture = architecture
        self.contents = contents #Kernel Modules and optional GRUB
        self.cloud = cloud
        self.recommendedRamdisk = recommendedRamdisk
        self.isPvGrub = isPvGrub
        
    def getContentsAbsPath(self):
        '''
        Return absolute path of contents
        '''
        
        if self.contents is None:
            return None
        
        m = re.match("internal\:\/\/(.*)", self.contents)
        if m:
            return pkg_resources.resource_filename(__package__, m.group(1))
        else:
            return self.contents
        
    def __str__(self):
        return self.aki
    
    def __eq__(self, other):
        return (self.aki, self.cloud.name) == (other.aki, other.cloud.name)
    
    def __ne__(self, other):
        return not self.__eq__(other)

        
        
        