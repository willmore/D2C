
import pkg_resources
from .Storage import WalrusStorage
from d2c.model.AWSCred import AWSCred
from urlparse import urlparse
import boto
from boto.ec2.regioninfo import RegionInfo

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
        self.storage = WalrusStorage("placeholder_name", storageURL)
        self.serviceURL = serviceURL
        self.parsedEndpoint = urlparse(serviceURL)
        self.regionInfo = RegionInfo(name=name, endpoint=self.parsedEndpoint.hostname)
        
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
    
    def bundleUploader(self):
        return self.storage.bundleUploader()
    
    def getConnection(self, awsCred):
        
        assert isinstance(awsCred, AWSCred)
        
        return boto.connect_ec2(aws_access_key_id=awsCred.access_key_id,
                                aws_secret_access_key=awsCred.secret_access_key,
                                is_secure=self.parsedEndpoint.scheme == "https",
                                region=self.regionInfo,
                                port=self.parsedEndpoint.port,
                                path=self.parsedEndpoint.path)


        
