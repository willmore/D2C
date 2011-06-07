'''
Created on Jun 6, 2011

@author: willmore
'''
from urlparse import urlparse
from boto.ec2.regioninfo import RegionInfo
import boto.ec2
from d2c.logger import StdOutLogger
from d2c.model.Kernel import Kernel
import pkg_resources

class Region:
    
    def __init__(self):
        pass
    
        
class EC2Region(Region):
        
    def __init__(self, name, logger=StdOutLogger()):
        Region.__init__(self)
        self.name = name
        
    def getConnection(self, awsCred):
        
        if self.__ec2Conn is None:
            #TODO: add timeout - if network connection fails, this will spin forever
            self.__logger.write("Initiating connection to ec2 region '%s'..." % self.name)
            self.__ec2Conn = boto.ec2.connect_to_region(self.name, 
                                                        aws_access_key_id=awsCred.access_key_id, 
                                                        aws_secret_access_key=awsCred.secret_access_key)
            self.__logger.write("EC2 connection established")
            
        return self.__ec2Conn
    
    def getKernel(self, arch):
        
        kernelDir = pkg_resources.resource_filename(__package__, "ami_data/kernels")
        
        kernels = {Kernel.ARCH_X86: Kernel("aki-4deec439", Kernel.ARCH_X86, kernelDir + "/2.6.35-24-virtual.tar"), # eu west pygrub, i386
                    Kernel.ARCH_X86_64: Kernel('aki-4feec43b', Kernel.ARCH_X86_64, kernelDir + "/2.6.35-24-virtual-x86_64.tar")} # eu west pygrub, x86_64
         
        return kernels[arch]
    
    def getFStab(self):
        return pkg_resources.resource_filename(__package__, "ami_data/fstab")
    
        
class EucRegion(Region):
        
    def __init__(self, name, endpoint):
        
        Region.__init__(self, tuple())
        
        assert name is not None
        assert endpoint is not None
                
        self.endpoint = urlparse(endpoint)
        self.regionInfo = RegionInfo(name=name, endpoint=self.endpoint.hostname)
        self.type = type
        
    def getConnection(self, awsCred):
        
        return boto.connect_ec2(aws_access_key_id=awsCred.access_key_id,
                              aws_secret_access_key=awsCred.secret_access_key,
                              is_secure=self.regionInfo.scheme == "https",
                              region=self.regionInfo,
                              port=self.regionInfo.port,
                              path=self.endpoint.path)
        
    
        
