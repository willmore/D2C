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
        
        
        
        
    '''   
            kernelDir = pkg_resources.resource_filename(__package__, "ami_data/kernels")
            
            kernelSrcTar = {AMITools.ARCH_X86:kernelDir + "/2.6.35-24-virtual.tar",
                            AMITools.ARCH_X86_64:kernelDir + "/2.6.35-24-virtual-x86_64.tar"}
            
            kernels = {AMITools.ARCH_X86: AMITools.Kernel("aki-4deec439", AMITools.ARCH_X86), # eu west pygrub, i386
                        AMITools.ARCH_X86_64:AMITools.Kernel('aki-4feec43b', AMITools.ARCH_X86_64)} # eu west pygrub, x86_64
    '''   