

class Kernel(object):
    
    
    ARCH_X86 = 'i386'
    ARCH_X86_64 = 'x86_64'
    
    def __init__(self, aki, arch, contents, cloudName=None):
        self.aki = aki
        self.arch = arch
        self.contents = contents
        self.cloudName = cloudName
        
    def __str__(self):
        return self.aki
    
    def __eq__(self, other):
        return (self.aki, self.cloudName) == (other.aki, other.cloudName)
    
    def __ne__(self, other):
        return not self.__eq__(other)
    
    