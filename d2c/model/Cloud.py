
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
from d2c.ShellExecutor import ShellExecutor


class Cloud(object):
    
    def __init__(self, id, name):
        
        assert isinstance(name, basestring)
        
        self.id = id
        self.name = name
        
class CloudConnection(object):
    
    def __init__(self):
        pass

class LibVirtInstance(object):
    
    def __init__(self, image, instanceType, dataDir):
        from .SourceImage import DesktopImage
        assert isinstance(image, DesktopImage), "image must be of type DesktopImage, is %s" % type(image)
        
        self.id = 'i-' + ''.join(random.choice(string.ascii_uppercase + string.digits) for _ in range(6))
        self.state = 'requesting' #TODO match with initial state of EC2
        #self.private_ip_address = None
        self.public_dns_name = None
        self.image = image
        self.instanceType = instanceType
        self.dataDir = dataDir
        self.dataFile = "%s/%s" % (self.dataDir, self.id)
        
        
    def start(self):
        
        ShellExecutor().run("dd if=%s of=%s" % (self.image.path, self.dataFile))
        
        domain_xml_file  = GenerateDomainXml.GenerateXML.generateXML('/home/sina/VirtualBox VMs/ubuntu1004/ubuntu1004.vdi',1,524288)
        network_xml_file = pkg_resources.resource_filename("model", "virtualbox_xml/mynetwork.xml")
        
        def return_xml(xml_location):
            lines = open(xml_location)
            xml = ''
            for line in lines:
                xml = xml+line
            return xml

        def ping(ip):
            return subprocess.call("ping -c 1 %s" % ip, shell=True, stdout=open('/dev/null', 'w'), stderr=subprocess.STDOUT)
        
        conn = libvirt.open("vbox:///session")

        try:
            conn.networkLookupByName("vboxnet0")
            print "vboxnet0 found, no need to create"
        except:
            network = libvirt.virConnect.networkDefineXML(conn, return_xml(network_xml_file))
            if(network.create()):
                print 'An error occured while creating the network,terminating program.'
                quit(1)
            print "Network Created with Name:"+network.name()
   
        dom = libvirt.virConnect.defineXML(conn, domain_xml_file)
    
        dom.create()

        print "going to pinging now..."

        while ping('192.168.152.2'):
            print "will sleep now"

        time.sleep(5)

        shell_executor = RemoteShellExecutor('q','192.168.152.2','%s/.ssh/id_rsa'%(os.getenv('HOME')))

        shell_executor.run("pwd")        
        
        #TODO set self.public_dns_name
        
        self.public_dns_name  = '192.168.152.2'
        
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
        from .SourceImage import DesktopImage
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
    
    def __init__(self, id, name):
        Cloud.__init__(self, id, name)
        
    def getConnection(self, *args):
        return LibVirtConn()
  
    
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
        
        return os.path.join(dataDir, "%s/%s.pem" % (dataDir, keyPairName))

class EC2Cloud(Cloud):
    '''
    Region represents the EC2 concept of a region, which is an isolated instance of 
    a cloud system.
    '''
    
    def __init__(self, id, name, serviceURL, 
                 storageURL, ec2Cert, botoModule=boto, 
                 kernels=list(), 
                 instanceTypes=list()):
        
        assert isinstance(name, basestring)
        assert isinstance(serviceURL, basestring)
        assert isinstance(storageURL, basestring)
        assert isinstance(ec2Cert, basestring)
        
        Cloud.__init__(self, id, name)
        
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
        return self.id == other.id
    
    def __ne__(self, other): 
        return not self == other
    


        
