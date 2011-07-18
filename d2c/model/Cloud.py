
import pkg_resources
from .Storage import WalrusStorage
from d2c.model.AWSCred import AWSCred
from urlparse import urlparse
import boto
from boto.ec2.regioninfo import RegionInfo
import threading
import string
import random
from threading import Thread
import os
import libvirt
from .SourceImage import DesktopImage
from d2c.ShellExecutor import ShellExecutor

class Cloud(object):
    
    def __init__(self, name):
        self.name = name
        
class CloudConnection(object):
    
    def __init__(self):
        pass

class LibVirtInstance(object):
    
    def __init__(self, image, instanceType, dataDir):
        
        assert isinstance(image, DesktopImage), "image must be of type DesktopImage, is %s" % type(image)
        
        self.id = 'i-' + ''.join(random.choice(string.ascii_uppercase + string.digits) for _ in range(6))
        self.state = 'requesting' #TODO match with initial state of EC2
        self.private_ip_address = None
        self.image = image
        self.instanceType = instanceType
        self.dataDir = dataDir
        
    def start(self):
        #TODO clone
        ShellExecutor().run("dd if=%s of=%s/%s" % (self.image.path, self.dataDir, self.id))
        self.state = 'running'
        
    def update(self):
        pass

class LibVirtReservationThread(Thread):
    
    def __init__(self, reservation):
        Thread.__init__(self)
        self.reservation = reservation
        
    def run(self):
        self.reservation.reserve()

class LibVirtReservation(object):
    
    def __init__(self, image, instanceType, count):
        
        assert isinstance(image, DesktopImage), "image must be of type DesktopImage, is %s" % type(image)
        
        self.id = 'r' + ''.join(random.choice(string.ascii_uppercase + string.digits) for _ in range(6))
        self.dataDir = "/tmp/d2c/libvirt_reservation%s/" % self.id    
        self.instances = [LibVirtInstance(image, instanceType, self.dataDir) for _ in range(count)]        
        
        os.makedirs(self.dataDir)
        
    def reserve(self):
        for inst in self.instances:
            inst.start()     
        

class LibVirtConn(CloudConnection):
    
    def __init__(self):
        CloudConnection.__init__(self)
        self.reservations = {}
        
    def runInstances(self, image, instanceType, count):
        #TODO action acquire instances
        reservation = LibVirtReservation(image, instanceType, count)
        self.reservations[reservation.id] = reservation
        LibVirtReservationThread(reservation).start()
        return reservation
    
    def getAllInstances(self, reservationId=None):
        #TODO return instance objects
        '''
        If reservationId is none, return a list of all reservations
        if resrvationId is defined, return a list of the one reservation that matches
        Must create a new VirtualBox instance type that has a fields:
            - state
            - public_dns_name = ip of vbox instance
        '''
    
        if reservationId is None:
            return list(self.reservations.values())
        else:
            return [self.reservations[reservationId]] if self.reservations.has_key(reservationId) else list()
            

class DesktopCloud(Cloud):
    
    def __init__(self, name):
        Cloud.__init__(self, name)
        
    def getConnection(self, *args):
        return LibVirtConn()
  
    
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


        
