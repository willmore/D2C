
import pkg_resources
from .Storage import WalrusStorage
from d2c.model.AWSCred import AWSCred
from urlparse import urlparse
import boto
from boto.ec2.regioninfo import RegionInfo
import threading


class Cloud(object):
    
    def __init__(self, name):
        self.name = name
        
class CloudConnection(object):
    
    def __init__(self):
        pass

class DummyReservation(object):
    
    def __init__(self):
        self.id = 0
        self.instances = []

class VBoxConn(CloudConnection):
    
    def __init__(self):
        CloudConnection.__init__(self)
        
    def runInstances(self, image, instanceType, count):
        #TODO action acquire instances
        return DummyReservation() #because Deployment needs a reservation handle
    
    def getAllInstances(self, reservationId=None):
        #TODO return instance objects
        '''
        If reservationId is none, return a list of all reservations
        if resrvationId is defined, return a list of the one reservation that matches
        Must create a new VirtualBox instance type that has a fields:
            - state
            - public_dns_name = ip of vbox instance
        '''
        pass

class DesktopCloud(Cloud):
    
    def __init__(self, name):
        Cloud.__init__(self, name)
        
    def getConnection(self, *args):
        return VBoxConn()
  
    
class EC2CloudConn(CloudConnection):
    
    def __init__(self, botoConn):
        CloudConnection.__init__(self)
        self.botoConn = botoConn
        
    def runInstances(self, image, instanceType, count):    
        
        return self.botoConn.run_instances(image.amiId, 
                                    key_name=None,#key_name=str(launchKey) if launchKey is not None else None,
                                    min_count=count, 
                                    max_count=count, 
                                    instance_type=str(instanceType.name))
        
    def getAllInstances(self, reservationId=None):
        if reservationId is None:
            return self.botoConn.get_all_instances()                    
        else:
            return self.botoConn.get_all_instances(filters={'reservation-id':reservationId})

class EC2Cloud(Cloud):
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
        
        Cloud.__init__(self, name)
        
        self.mylock = threading.RLock()
        self.botoModule = botoModule
        self.serviceURL = serviceURL
        self.storageURL = storageURL
        self.ec2Cert = ec2Cert
        self.kernels = list(kernels)
        self.instanceTypes = list(instanceTypes)
        self.storage = WalrusStorage("placeholder_name", storageURL)
        self.deployments = list()
       
    def getName(self):
        return self.name
    
    def getKernels(self):
        return self.kernels
    
    def getEC2Cert(self):
        return self.ec2Cert
        
    def getFStab(self):
        return pkg_resources.resource_filename(__package__, "ami_data/fstab")
    
    def bundleUploader(self):
        return self.storage.bundleUploader()
    
    def instanceTypeByName(self, name):
        for i in self.instanceTypes:
            if i.name == name:
                return i
            
        return None
    
    def getConnection(self, awsCred):
        
        assert isinstance(awsCred, AWSCred), "AWSCred is type=%s" % type(awsCred)
        
        if not hasattr(self, "__ec2Conn"):
            #Use string type because boto does not support unicode type
            
            parsedEndpoint = urlparse(self.serviceURL)
            
            regionInfo = RegionInfo(name=str(self.name), endpoint=str(parsedEndpoint.hostname))

            self.__ec2Conn = EC2CloudConn(self.botoModule.connect_ec2(
                                              aws_access_key_id=str(awsCred.access_key_id),
                                              aws_secret_access_key=str(awsCred.secret_access_key),
                                              is_secure=parsedEndpoint.scheme == "https",
                                              region=regionInfo,
                                              port=parsedEndpoint.port,
                                              path=str(parsedEndpoint.path)))
        
        return self.__ec2Conn
    
    def __eq__(self, other):
        return self.name == other.name
    
    def __ne__(self, other): 
        return not self == other


        
