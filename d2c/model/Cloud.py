
import pkg_resources
from .Storage import WalrusStorage
from d2c.model.AWSCred import AWSCred
from urlparse import urlparse
import boto
from boto.ec2.regioninfo import RegionInfo
import threading
from sqlalchemy.orm import reconstructor



class Cloud(object):
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
        self.deployments = list()
        
    
    @reconstructor
    def init_on_load(self):    
        self.botoModule = boto
        
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
    
    def instanceTypeByName(self, name):
        for i in self.instanceTypes:
            if i.name == name:
                return i
            
        return None
    
    def _getParsedEndPoint(self):
        if not hasattr(self, '__parsedEndPoint'):
            self.__parsedEndPoint = urlparse(self.serviceURL)
            
        return self.__parsedEndPoint
    
    def getConnection(self, awsCred):
        
        assert isinstance(awsCred, AWSCred), "AWSCred is type=%s" % type(awsCred)
        
        if not hasattr(self, "__ec2Conn"):
            #Use string type because boto does not support unicode type
            parsedEndpoint = self._getParsedEndPoint()
            regionInfo = RegionInfo(name=str(self.name), endpoint=str(parsedEndpoint.hostname))

            self.__ec2Conn = self.botoModule.connect_ec2(aws_access_key_id=str(awsCred.access_key_id),
                                              aws_secret_access_key=str(awsCred.secret_access_key),
                                              is_secure=parsedEndpoint.scheme == "https",
                                              region=regionInfo,
                                              port=parsedEndpoint.port,
                                              path=str(parsedEndpoint.path))
        
        return self.__ec2Conn
    
    def __eq__(self, other):
        return self.name == other.name
    
    def __ne__(self, other): 
        return not self == other


        
