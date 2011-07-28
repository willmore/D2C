import libvirt
import time
import subprocess
import generate_domain_xml
from d2c.RemoteShellExecutor import RemoteShellExecutor
import pkg_resources

domain_xml_file  = generate_domain_xml.GenerateXML.generateXML('/home/sina/VirtualBox VMs/ubuntu1004/ubuntu1004.vdi',1,524288)
network_xml_file = pkg_resources.resource_filename("test", "virtualbox_xml/mynetwork.xml")

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
   
#dom = libvirt.virConnect.defineXML(conn, return_xml(domain_xml_file))
#dom.create()

try:
    #TODO: Keyword will come from user 
    dom = conn.lookupByName('ubuntu10')
except:
    print 'No deployment found with that name,creating the one from xml!'
    dom = libvirt.virConnect.defineXML(conn, domain_xml_file)
    
dom.create()

print "going to pinging now..."

while ping('192.168.152.2'):
    print "will sleep now"

time.sleep(5)

shell_executor = RemoteShellExecutor('q','192.168.152.2','/home/sina/.ssh/id_rsa')

shell_executor.run("pwd")

time.sleep(5)

print "Will destroy domain now"

dom.destroy()

print "Domain destroyed, will sleep for 5 seconds"

time.sleep(5)

dom.undefine()

print "Domain undefined"

network.destroy()

print "Network destroyed"

print "Will sleep 5 seconds"

time.sleep(5)

network.undefine()

print "Network undefined"
