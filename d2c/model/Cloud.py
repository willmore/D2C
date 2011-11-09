
import pkg_resources
from .Storage import WalrusStorage
from d2c.model.AWSCred import AWSCred
from urlparse import urlparse
#import boto
from boto.ec2.regioninfo import RegionInfo
import threading
import string
import random
from threading import Thread
import os
import libvirt
from d2c.ShellExecutor import ShellExecutor
from .GenerateDomainXml import GenerateXML
import subprocess
import time
import guestfs
from threading import RLock
import types
from d2c.util import ReadWriteLock

from abc import ABCMeta, abstractmethod

class Cloud(object):
    
    __metaclass__ = ABCMeta
    
    def __init__(self, id, name, instanceTypes=()):
        
        assert isinstance(name, basestring)
        
        self.id = id
        self.name = name
        self.instanceTypes=list(instanceTypes)
        
    def requiresAWSCred(self):
        '''
        Returns True if the Cloud requires AWS Credentials for access rights.
        This basically applies only to EC2 and Eucalyptus clouds.
        '''
        return False
        
        
class CloudConnection(object):
    
    __metaclass__ = ABCMeta
    
    def __init__(self):
        pass
     
    def getInstanceStates(self, reservationIds):
        '''
        Return a iterable of string states for all instances
        for the reservation IDs.
        '''  
        
        if len(reservationIds) == 0:
            raise Exception("reservationIds parameter does not contain at least one entry.")
        
        #self.logger.write("Getting instances states for reservation-id(s): %s" % str(reservationIds))
        
        # Filter only works in boto 2.0. Add back when we move from 1.9 to 2.0
        #res = self.cloud.getConnection(self.awsCred).get_all_instances(filters={'reservation-id':reservationIds})
        
        res = self.getAllInstances()
        
        #self.logger.write("Got reservations: %s" % str(res))
        
        states = []
        for r in res:
            if r.id in reservationIds:
                for i in r.instances:
                    states.append(i.state)
        
        #self.logger.write("Instance states for reservations %s are %s" % (str(reservationIds), str(states)))
        
        return states
    
    @abstractmethod
    def runInstances(self, image, instanceType, count, keyName):
        pass
    
    @abstractmethod
    def getAllInstances(self, reservationId=None):
        pass
    
    @abstractmethod
    def generateKeyPair(self, dataDir, keyPairName):
        pass


class LibVirtInstance(object):
    
    def __init__(self, image, instanceType, dataDir, pubKeyFile):
        from .SourceImage import DesktopImage
        assert isinstance(image, DesktopImage), "image must be of type DesktopImage, is %s" % type(image)
        
        self.id = 'i-' + ''.join(random.choice(string.ascii_uppercase + string.digits) for _ in range(6))
        self.state = 'requesting' #TODO match with initial state of EC2
        self.private_dns_name = None
        self.public_dns_name = None
        self.image = image
        self.instanceType = instanceType
        self.dataDir = dataDir
        self.dataFile = "%s/%s" % (self.dataDir, self.id)
        self.pubKeyFile = pubKeyFile
        self.dom = None
    
    def __insertKey(self, imageFile, publicKeyPath):
        
        file = open(publicKeyPath, "r")
        publicKey = file.read()
        file.close()
        
        gf = guestfs.GuestFS ()
        gf.set_trace(1)
        gf.set_autosync(1)
        
        gf.add_drive(imageFile)    
        gf.launch() 
        
        roots = gf.inspect_os()
        assert (len(roots) == 1) #Only supporting one main partition for now
        rootDev = roots[0]
        gf.mount(rootDev, "/")
        #gf.mkdir_mode("/root/.ssh", 0755)
        gf.write("/root/.ssh/authorized_keys", publicKey)
        gf.chown(0, 0, "/root/.ssh")
        gf.chown(0, 0, "/root/.ssh/authorized_keys")

        del gf #sync and shutdown
        
    def __setStaticIp(self,imageFile,IP):
        
        static_ip_file = pkg_resources.resource_filename(__package__, "virtualbox_xml/static_ip.txt") 
        
        file = open(static_ip_file, "r")
        static_ip = file.read()
        file.close()

        static_ip=static_ip.replace('$ip',IP)
        
        print static_ip
        
        gf = guestfs.GuestFS ()
        gf.set_trace(1)
        gf.set_autosync(1)
        
        gf.add_drive(imageFile)    
        gf.launch() 
        
        roots = gf.inspect_os()
        assert (len(roots) == 1) #Only supporting one main partition for now
        rootDev = roots[0]
        gf.mount(rootDev, "/")
        
        #Remove udev net rules so our new network device is assigned eth0
        # We could write a file that explicitly binds a known eth* to our known mac address..
        gf.rm('/etc/udev/rules.d/70-persistent-net.rules')
        gf.write("/etc/network/interfaces", static_ip)
        
        del gf #sync and shutdown   
    
    gfLock = RLock()
    
    def _provisionCopy(self, ipAddress):
        '''Make a copy of the image to use for this execution.'''
        self.image.path = self.image.path.replace(' ', '\\ ')
        ShellExecutor().run("dd if=%s of=%s" % (self.image.path, self.dataFile))
        os.chmod(self.dataFile,  0755)
        
        '''Change the HD UUID, otherwise VirtualBox will refuse to use it if same UUID is already registered'''
        ShellExecutor().run("VBoxManage internalcommands sethduuid %s" % self.dataFile)
        
        self.gfLock.acquire()
        self.__insertKey(self.dataFile, self.pubKeyFile)
        self.__setStaticIp(self.dataFile, ipAddress)
        self.gfLock.release()
        
        self.public_dns_name  = ipAddress        
        self.private_ip_address = ipAddress
        self.private_dns_name = ipAddress
        
    def start(self, conn):
        
        domain_xml_file  = GenerateXML.generateXML(self.dataFile,1,524288)
        
        def ping(ip):
            return subprocess.call("ping -c 1 %s" % ip, shell=True, stdout=open('/dev/null', 'w'), stderr=subprocess.STDOUT)

        self.dom = conn.defineXML(domain_xml_file)
        self.dom.create()

        ''' Wait until host is responding to ping'''
        while ping(self.public_dns_name):
            pass
        
        self.state = 'running'      
        
    def stop(self):
        if self.dom is not None:
            self.dom.destroy()
            time.sleep(5)
            self.dom.undefine()
            self.state = 'terminated'
    
    def update(self):
        pass
        
class SynchronizedProxy(object):
    '''
    Synchrnonizes access to object methods through use of RLock
    '''
    
    def __init__(self, proxy):
        self.proxy = proxy
        self.lock = threading.RLock()
    
    def __getattribute__(self, attrName):
        if attrName is "proxy" or attrName is "lock":
            return object.__getattribute__(self, attrName)
            
        #print "Type = %s" % type(self.proxy.__getattribute__(attrName))
        attr = getattr(self.proxy, attrName)
        if isinstance(attr, types.MethodType):
            def lockedFunc(*args, **kwargs):
                self.lock.acquire()
                v = attr(*args, **kwargs)
                self.lock.release()
                return v
            return lockedFunc
        return attr
    
    def __setattr__(self, name, value):
        if name is "proxy" or name is "lock":
            object.__setattr__(self, name, value)
        else:
            self.proxy.__dict__[name] = value
    

class LibVirtReservationThread(Thread):
    
    def __init__(self, reservation):
        Thread.__init__(self)
        self.reservation = reservation
        
    def run(self):
        self.reservation.reserve()

class LibVirtReservation(object):
    
    __libVirtConn = None
    
    def __init__(self, image, instanceType, count, pubKeyFile):
        from .SourceImage import DesktopImage
        assert isinstance(image, DesktopImage), "image must be of type DesktopImage, is %s" % type(image)
        
        self.id = 'r' + ''.join(random.choice(string.ascii_uppercase + string.digits) for _ in range(6))
        self.dataDir = "/tmp/d2c/libvirt_reservation%s/" % self.id  
        self.pubKeyFile = pubKeyFile 
        self.instances = [LibVirtInstance(image, instanceType, self.dataDir, self.pubKeyFile) for _ in range(count)]        
       
        os.makedirs(self.dataDir)
        
    def __createVirtualNetwork(self, conn):
        ''' create vboxnet0 network '''
        try:
            network = conn.networkLookupByName("vboxnet0")
            print "existing vboxnet0 found"
            return
        except:
            print "vboxnet0 not found, will create it now"
            
            
        def return_xml(xml_location):
            lines = open(xml_location)
            xml = ''
            for line in lines:
                xml = xml+line
            return xml    
            
        network_xml_file = pkg_resources.resource_filename(__package__, "virtualbox_xml/mynetwork.xml")
        conn.networkDefineXML(return_xml(network_xml_file))
        if(network.create()):
            print 'An error occured while creating the network,terminating program.'
            quit(1)
        print "Network Created with Name:"+network.name()

    lastoctet = 100
    __iplock = RLock()
    @classmethod
    def getIPAddress(cls):
        
        cls.__iplock.acquire()
        ipAddress = '192.168.152.%s' % cls.lastoctet
        cls.lastoctet += 1
        cls.__iplock.release()
        return ipAddress
    
    __reserveLock = threading.RLock()

    rwLock = ReadWriteLock()

    def reserve(self):
        
        ''' Ensure we can get a libvirt connection before making large image copies. '''
        conn = LibVirtReservation.getLibVirtConn()
        self.__createVirtualNetwork(conn)
        self.rwLock.acquireRead()
        for inst in self.instances:
            inst._provisionCopy(LibVirtReservation.getIPAddress())
                                
        self.rwLock.release()
        
        self.rwLock.acquireWrite()
        for inst in self.instances:
            inst.start(conn) 
        self.rwLock.release()
            
        #LibVirtReservation.__reserveLock.release()   
    
    __lock = threading.RLock()
    
    @classmethod  
    def getLibVirtConn(cls):
        '''
        Control access to libvirt singleton
        '''
        cls.__lock.acquire()
        print "Accessing libvirt conn"
        if cls.__libVirtConn is None:
            print "Opening conn"
            cls.__libVirtConn = SynchronizedProxy(libvirt.open("vbox:///session"))
        cls.__lock.release()
        
        return cls.__libVirtConn
        

class LibVirtConn(CloudConnection):
    
    def __init__(self):
        CloudConnection.__init__(self)
        self.reservations = {}
        self.publicKeyMap = {}
        
    def runInstances(self, image, instanceType, count, keyName, runFromCopy=True):
        
        if not runFromCopy:
            ''' We can only run from source directly if there is only one instance '''
            assert count == 1 
        
        reservation = LibVirtReservation(image, instanceType, count, self.publicKeyMap[keyName])
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
            if not self.reservations.has_key(reservationId):
                raise Exception("No reservation found for id: %s" % reservationId)
            return [self.reservations[reservationId]]
    
    def generateKeyPair(self, dataDir, keyPairName):
        '''
        Creates a key pair, with the public key being saved in the cloud 
        and the private saved locally in the directory dataDir.
        
        Return the full path location to the new private key.
        '''
        if not os.path.exists(dataDir):
            os.makedirs(dataDir, mode=0700)
        
        privKeyFile = os.path.join(dataDir, "%s/%s" % (dataDir, keyPairName))
        
        ShellExecutor().run("ssh-keygen -t rsa -P \"\" -f %s" % privKeyFile)
        
        pubKeyFile = "%s.pub" % privKeyFile
        os.chmod(privKeyFile, 0600)
        assert os.path.exists(pubKeyFile)
        
        self.publicKeyMap[keyPairName] = pubKeyFile
        
        return os.path.join(dataDir, "%s/%s" % (dataDir, keyPairName)) 


class DesktopCloud(Cloud):
    
    def __init__(self, id, name, instanceTypes):
        Cloud.__init__(self, id, name, instanceTypes)
        self.__conn = None
        
    def getConnection(self, *args):
        
        if (not hasattr(self, "conn")) or self.conn is None:
            self.conn = LibVirtConn()
        
        return self.conn
  
    
class EC2CloudConn(CloudConnection):
    
    def __init__(self, botoConn):
        CloudConnection.__init__(self)
        self.botoConn = botoConn
        
    def runInstances(self, image, instanceType, count, keyName):    
        
        return self.botoConn.run_instances(image.amiId, 
                                    min_count=count, 
                                    max_count=count, 
                                    instance_type=str(instanceType.name),
                                    key_name=keyName)
        
    def getAllInstances(self, reservationId=None):
        if reservationId is None:
            return self.botoConn.get_all_instances()                    
        else:
            return self.botoConn.get_all_instances(filters={'reservation-id':reservationId})
        
    def registerImage(self, imageLocation):
        return self.botoConn.register_image(image_location=imageLocation)
    
    def generateKeyPair(self, dataDir, keyPairName):
        '''
        Creates a key pair, with the public key being saved in the cloud 
        and the private saved locally in the directory dataDir.
        
        Return the full path location to the new private key.
        '''
        if not os.path.exists(dataDir):
            os.makedirs(dataDir, mode=0700)
        self.botoConn.create_key_pair(keyPairName).save(dataDir)
        
        privKey = os.path.join(dataDir, "%s/%s.pem" % (dataDir, keyPairName))
        os.chmod(privKey, 0600)
        
        #Temp hack as I'm having trouble getting EC2 to insert the public key
        #return privKey
        #return "/home/willmore/.d2c/root_key"
        return privKey
    

class EC2Cloud(Cloud):
    '''
    Region represents the EC2 concept of a region, which is an isolated instance of 
    a cloud system.
    '''
    
    def __init__(self, id, name, serviceURL, 
                 storageURL, ec2Cert, botoModule, 
                 kernels=list(), 
                 instanceTypes=list()):
        
        assert isinstance(name, basestring)
        assert isinstance(serviceURL, basestring)
        assert isinstance(storageURL, basestring)
        assert isinstance(ec2Cert, basestring)
        
        Cloud.__init__(self, id, name, instanceTypes)
        
        self.mylock = threading.RLock()
        self.botoModule = botoModule
        self.serviceURL = serviceURL
        self.storageURL = storageURL
        self.ec2Cert = ec2Cert
        self.kernels = list(kernels)
        self.storage = WalrusStorage("placeholder_name", storageURL)
        self.deployments = list()
    
    def requiresAWSCred(self):
        return True
       
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
        return self.id == other.id
    
    def __ne__(self, other): 
        return not self == other
    
    
class CloudCred(object):
    
    def __init__(self, 
                 id,
                 name,
                 awsUserId,
                 awsKeyId,
                 awsSecretAccessKey,
                 ec2CloudCert,
                 ec2Cert,
                 ec2PrivateKey):
        
        self.id = id
        self.name = name
        self.awsUserId = awsUserId
        self.awsKeyId = awsKeyId
        self.awsSecretAccessKey = awsSecretAccessKey
        self.ec2CloudCert = ec2CloudCert
        self.ec2Cert = ec2Cert
        self.ec2PrivateKey = ec2PrivateKey
    


        
