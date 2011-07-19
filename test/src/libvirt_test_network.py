import libvirt
import time
import subprocess
from d2c.RemoteShellExecutor import RemoteShellExecutor

network_xml_file = "/home/sina/.d2c/mynetwork.xml"
domain_xml_file = "/home/sina/.d2c/mydomain.xml"

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
   
#dom = libvirt.virConnect.defineXML(conn, return_xml(domain_xml_file))
#dom.create()

try:
    #TODO: Keyword will come from user 
    dom = conn.lookupByName('ubuntu10')
except:
    print 'No deployment found with that name,creating the one from xml!'
    dom = libvirt.virConnect.defineXML(conn, return_xml(domain_xml_file))
    
dom.create()

print "going to pinging now..."

while ping('192.168.152.2'):
    print "will sleep now"

time.sleep(5)

shell_executor = RemoteShellExecutor('q','192.168.152.2','/home/sina/.ssh/id_rsa')

shell_executor.run("pwd")

time.sleep(5)

print "will shut down now"

dom.shutdown()

network.undefine()