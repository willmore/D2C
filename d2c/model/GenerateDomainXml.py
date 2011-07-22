from xml.dom.minidom import parse
import os
import sys
import random
import pkg_resources

class GenerateXML:
    
    @staticmethod   
    def generateXML(vdi_image,noOfCPU,memory):

        source_xml =  pkg_resources.resource_filename(__package__, "virtualbox_xml/mydomain.xml")
        
        def randomizeName():
            s="abcdef123"
            return ''.join(random.sample(s,len(s)))
        
        # set path of vdi image
        doc = parse(source_xml)
        ref = doc.getElementsByTagName('source')[0]
        ref.attributes["file"]=vdi_image
        
        # set domain name
        node = doc.getElementsByTagName('name')
        node[0].firstChild.nodeValue = randomizeName()
        
        # set cpu number
        node = doc.getElementsByTagName('vcpu')
        node[0].firstChild.nodeValue = noOfCPU
        
        
        # set memory
        node = doc.getElementsByTagName('memory')
        node[0].firstChild.nodeValue = memory
        
        # set current memory
        node = doc.getElementsByTagName('currentMemory')
        node[0].firstChild.nodeValue = memory
        
        ## persist changes to new file
        #xml_file = open(generated_xml_file, "w")
        #doc.writexml(xml_file, encoding="utf-8")
        #xml_file.close()
        print "generated xml file:"
        print doc.toxml()
        
        return doc.toxml()