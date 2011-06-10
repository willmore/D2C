
import pkg_resources

class Cloud:
    '''
    Region represents the EC2 concept of a region, which is an isolated instance of 
    a cloud system.
    '''
    
    def __init__(self, name, serviceURL, 
                 storageURL, ec2Cert, kernels=()):
        
        assert isinstance(name, basestring)
        assert isinstance(serviceURL, basestring)
        assert isinstance(storageURL, basestring)
        assert isinstance(ec2Cert, basestring)
        
        self.name = name
        self.ec2Cert = ec2Cert
        self.kernels = list(kernels)
        
    def getName(self):
        return self.name
    
    def _registerKernels(self, kernels):
        self.kernels.update(kernels)
    
    def getKernels(self):
        return self.kernels
    
    def getEC2Cert(self):
        return self.ec2Cert
    
    def getEndpoint(self):
        return self.endpoint
        
    def getFStab(self):
        return pkg_resources.resource_filename(__package__, "ami_data/fstab")


        
