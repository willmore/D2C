from xml.dom.minidom import parse
import os
import sys

class GenerateXML:
    
    def generateXML(source_xml_file,generated_xml_file,vdi_image,noOfCPU,memory):
        
        # set path of vdi image
        doc = parse(source_xml_file)
        ref = doc.getElementsByTagName('source')[0]
        ref.attributes["file"]=vdi_image
        
        # set cpu number
        node = doc.getElementsByTagName('vcpu')
        node[0].firstChild.nodeValue = noOfCPU
        
        
        # set memory
        node = doc.getElementsByTagName('memory')
        node[0].firstChild.nodeValue = memory
        
        # set current memory
        node = doc.getElementsByTagName('currentMemory')
        node[0].firstChild.nodeValue = memory
        
        # persist changes to new file
        xml_file = open(generated_xml_file, "w")
        doc.writexml(xml_file, encoding="utf-8")
        xml_file.close()
        
        return
    
    generateXML(sys.argv[1],sys.argv[2],sys.argv[3],sys.argv[4],sys.argv[5])