
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
from d2c.RemoteShellExecutor import RemoteShellExecutor
import time
import guestfs


class Cloud(object):
    
    def __init__(self, id, name, instanceTypes=()):
        
        assert isinstance(name, basestring)
        
        self.id = id
        self.name = name
        self.instanceTypes=list(instanceTypes)
        
        
class CloudConnection(object):
    
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
    
    def getAllInstances(self):
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
        gf.mkdir_mode("/root/.ssh", 0755)
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
        
        gf.write("/etc/network/interfaces", static_ip)
        
        del gf #sync and shutdown   
        
    def start(self):
        
        self.image.path = self.image.path.replace(' ', '\\ ')
        ShellExecutor().run("dd if=%s of=%s" % (self.image.path, self.dataFile))
        os.chmod(self.dataFile,  0755)
        
        #Change the HD UUID, otherwise VirtualBox will refuse to use it if same UUID is already registered
        ShellExecutor().run("VBoxManage internalcommands sethduuid %s" % self.dataFile)
        
        self.__insertKey(self.dataFile, self.pubKeyFile)
        
        #TODO : lastoctet will increment for multiple instance cases
        lastoctet = 100
        IP = '192.168.152.%s' %(lastoctet)
        
        self.__setStaticIp(self.dataFile,IP)
        
        domain_xml_file  = GenerateXML.generateXML(self.dataFile,1,524288)
        network_xml_file = pkg_resources.resource_filename(__package__, "virtualbox_xml/mynetwork.xml")
        
        def return_xml(xml_location):
            lines = open(xml_location)
            xml = ''
            for line in lines:
                xml = xml+line
            return xml

        def ping(ip):
            return subprocess.call("ping -c 1 %s" % ip, shell=True, stdout=open('/dev/null', 'w'), stderr=subprocess.STDOUT)
        
        conn = libvirt.open("vbox:///session")

        #create vboxnet0 network
        try:
            network = conn.networkLookupByName("vboxnet0")
            print "vboxnet0 found, will destroy vboxnet0"
            network.destroy()
            time.sleep(5)
            network.undefine()
            print "vboxnet0 undefined"
        except:
            print "vboxnet0 not found, will create it now"
            
        network = libvirt.virConnect.networkDefineXML(conn, return_xml(network_xml_file))
        if(network.create()):
            print 'An error occured while creating the network,terminating program.'
            quit(1)
        print "Network Created with Name:"+network.name()
   
        #create domain
        self.dom = libvirt.virConnect.defineXML(conn, domain_xml_file)
    
        self.dom.create()

        print "pinging now..."

        while ping(IP):
            print "will sleep now"

        time.sleep(5)

        shell_executor = RemoteShellExecutor('q',IP,'%s/.ssh/id_rsa'%(os.getenv('HOME')))

        shell_executor.run("pwd")        
                
        self.public_dns_name  = IP        
        self.private_ip_address = IP
        self.private_dns_name = IP
        
        self.state = 'running'      
        
    def stop(self):
        if self.dom is not None:
            self.dom.destroy()
            self.dom.undefine()
    
    def update(self):
        pass

class LibVirtReservationThread(Thread):
    
    def __init__(self, reservation):
        Thread.__init__(self)
        self.reservation = reservation
        
    def run(self):
        self.reservation.reserve()

class LibVirtReservation(object):
    
    def __init__(self, image, instanceType, count, pubKeyFile):
        from .SourceImage import DesktopImage
        assert isinstance(image, DesktopImage), "image must be of type DesktopImage, is %s" % type(image)
        
        self.id = 'r' + ''.join(random.choice(string.ascii_uppercase + string.digits) for _ in range(6))
        self.dataDir = "/tmp/d2c/libvirt_reservation%s/" % self.id  
        self.pubKeyFile = pubKeyFile 
        self.instances = [LibVirtInstance(image, instanceType, self.dataDir, self.pubKeyFile) for _ in range(count)]        
       
        
        os.makedirs(self.dataDir)
        
    def reserve(self):
        for inst in self.instances:
            inst.start()     
        

class LibVirtConn(CloudConnection):
    
    def __init__(self):
        CloudConnection.__init__(self)
        self.reservations = {}
        self.publicKeyMap = {}
        
    def runInstances(self, image, instanceType, count, keyName):
        #TODO action acquire instances
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
        
        return os.path.join(dataDir, "%s/%s.pem" % (dataDir, keyPairName)) 

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
    


        
