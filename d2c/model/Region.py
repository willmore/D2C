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
    '''
    Region represents the EC2 concept of a region, which is an isolated instance of 
    a cloud system.
    '''
    
    def __init__(self, name, endpoint, ec2Cert):
        
        assert isinstance(name, basestring)
        assert isinstance(endpoint, basestring)
        assert isinstance(ec2Cert, basestring)
        
        self.__name = name
        self.__endpoint = endpoint
        self.__ec2Cert = ec2Cert
        self.__kernels = {}
        
    def getName(self):
        return self.__name
    
    def _registerKernels(self, kernels):
        self.__kernels.update(kernels)
        
    def getKernel(self, arch):
        '''
        Return a supported kernel for the region and provided architecture.
        '''  
        return self.__kernels[arch] if self.__kernels.has_key(arch) else None
    
    def getEC2Cert(self):
        return self.__ec2Cert
    
    def getEndpoint(self):
        return self.__endpoint
    
    def getConnection(self, awsCred):
        
        return boto.connect_ec2(aws_access_key_id=awsCred.access_key_id,
                              aws_secret_access_key=awsCred.secret_access_key,
                              is_secure=self.parsedEndpoint.scheme == "https",
                              region=self.regionInfo,
                              port=self.parsedEndpoint.port,
                              path=self.parsedEndpoint.path)
        
    def getFStab(self):
        return pkg_resources.resource_filename(__package__, "ami_data/fstab")
        
class EC2Region(Region):
        
    def __init__(self, name, endpoint, ec2Cert, logger=StdOutLogger()):
          
        Region.__init__(self, name, endpoint, ec2Cert)
        
        self.__ec2Conn = None
        self.__logger = logger
        
        kernelDir = pkg_resources.resource_filename(__package__, "ami_data/kernels")
        
        kernels = {Kernel.ARCH_X86: Kernel("aki-4deec439", Kernel.ARCH_X86, kernelDir + "/2.6.35-24-virtual.tar"), # eu west pygrub, i386
                    Kernel.ARCH_X86_64: Kernel('aki-4feec43b', Kernel.ARCH_X86_64, kernelDir + "/2.6.35-24-virtual-x86_64.tar")} # eu west pygrub, x86_64
         
        self._registerKernels(kernels)
        
    def getConnection(self, awsCred):
        
        if self.__ec2Conn is None:
            #TODO: add timeout - if network connection fails, this will spin forever
            self.__logger.write("Initiating connection to ec2 region '%s'..." % self.name)
            self.__ec2Conn = boto.ec2.connect_to_region(self.name, 
                                                        aws_access_key_id=awsCred.access_key_id, 
                                                        aws_secret_access_key=awsCred.secret_access_key)
            self.__logger.write("EC2 connection established")
            
        return self.__ec2Conn
    
    def getFStab(self):
        return pkg_resources.resource_filename(__package__, "ami_data/fstab")
    
        
class EucRegion(Region):
        
    def __init__(self, name, ec2Cert, endpoint):
        
        Region.__init__(self, name, endpoint, ec2Cert)
        
        assert endpoint is not None
                
        self.parsedEndpoint = urlparse(endpoint)
        self.regionInfo = RegionInfo(name=name, endpoint=self.parsedEndpoint.hostname)
        self.type = type
        
        kernelDir = pkg_resources.resource_filename(__package__, "ami_data/kernels")
        
        kernels = {Kernel.ARCH_X86_64: Kernel("eki-B482178C", Kernel.ARCH_X86_64, kernelDir + "/2.6.27.21-0.1-xen.tar")}
        
        self._registerKernels(kernels)
        
    def getConnection(self, awsCred):
        
        return boto.connect_ec2(aws_access_key_id=awsCred.access_key_id,
                              aws_secret_access_key=awsCred.secret_access_key,
                              is_secure=self.parsedEndpoint.scheme == "https",
                              region=self.regionInfo,
                              port=self.parsedEndpoint.port,
                              path=self.parsedEndpoint.path)
        
    def getFStab(self):
        return pkg_resources.resource_filename(__package__, "ami_data/fstab")

        
