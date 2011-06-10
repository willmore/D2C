'''
Created on Jun 7, 2011

@author: willmore
'''
class Kernel:
    
    
    ARCH_X86 = 'i386'
    ARCH_X86_64 = 'x86_64'
    
    def __init__(self, aki, arch, contents):
        self.aki = aki
        self.arch = arch
        self.contents = contents