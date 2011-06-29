import re
import pkg_resources

class Kernel(object):
    
    
    ARCH_X86 = 'i386'
    ARCH_X86_64 = 'x86_64'
    
    def __init__(self, aki, architecture, contents, cloud=None, recommendedRamdisk=None):
        self.aki = aki
        self.architecture = architecture
        self.contents = contents
        self.cloud = cloud
        self.recommendedRamdisk = recommendedRamdisk
        
    def getContentsAbsPath(self):
        '''
        Return absolute path of contents
        '''
        
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
    
    