
import pkg_resources
from .Storage import WalrusStorage
from d2c.model.AWSCred import AWSCred
from urlparse import urlparse
import boto
from boto.ec2.regioninfo import RegionInfo
from d2c.util import synchronous
import threading


class Cloud:
    '''
    Region represents the EC2 concept of a region, which is an isolated instance of 
    a cloud system.
    '''
    
    def __init__(self, name, serviceURL, 
                 storageURL, ec2Cert, botoModule=boto, 
                 kernels=list(), 
                 instanceTypes=list()):
        
        assert isinstance(name, basestring)
        assert isinstance(serviceURL, basestring)
        assert isinstance(storageURL, basestring)
        assert isinstance(ec2Cert, basestring)
        
        self.mylock = threading.RLock()
        self.botoModule = botoModule
        self.name = name
        self.serviceURL = serviceURL
        self.storageURL = storageURL
        self.ec2Cert = ec2Cert
        self.kernels = list()
        self.addKernels(kernels)
        self.instanceTypes = list()
        self.addInstanceTypes(instanceTypes)
        self.storage = WalrusStorage("placeholder_name", storageURL)
        self.parsedEndpoint = urlparse(serviceURL)
        self.regionInfo = RegionInfo(name=name, endpoint=self.parsedEndpoint.hostname)
        self.deployments = list()
        
    def getName(self):
        return self.name
    
    def addDeployment(self, deployment):
        if self.deployments not in deployment:
            self.deployments.addpend(deployment)
        
        if deployment.cloud is not self:
            deployment.cloud = self
    
    def addInstanceTypes(self, types):
        for t in types:
            self.instanceTypes.append(t)
            t.cloud = self
    
    def addKernels(self, kernels):
        
        for k in kernels:
            k.cloudName = self.name
        
        self.kernels.extend(kernels)
    
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
    
    @synchronous('mylock')
    def getConnection(self, awsCred):
        
        assert isinstance(awsCred, AWSCred)
        
        if not hasattr(self, "__ec2Conn"):
            self.__ec2Conn = self.botoModule.connect_ec2(aws_access_key_id=awsCred.access_key_id,
                                              aws_secret_access_key=awsCred.secret_access_key,
                                              is_secure=self.parsedEndpoint.scheme == "https",
                                              region=self.regionInfo,
                                              port=self.parsedEndpoint.port,
                                              path=self.parsedEndpoint.path)
        
        return self.__ec2Conn
    
    def __eq__(self, other):
        return self.name == other.name
    
    def __ne__(self, other): 
        return not self == other


        
