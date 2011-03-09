import unittest
import d2c.AMICreator
import string
import time
from d2c.logger import StdOutLogger
from d2c.model.EC2Cred import EC2Cred
from d2c.AMICreator import AMITools
import d2c.AMICreator as AMICreator

class AMICreatorTest(unittest.TestCase):


    def test_full(self):
        settings = {}
        for l in open("/home/willmore/test.conf", "r"):
            (k, v) = string.split(l.strip(), "=")
            settings[k] = v
        
 
        ec2Cred = EC2Cred(settings['cert'], settings['privateKey'])
    
        logger = StdOutLogger();
    
        #extractRawImage('/media/host/xyz.vdi', '/media/host/xyz-full.img', logger)
        #extractMainPartition('/media/host/xyz-full.img', '/media/host/xyz-main-partition.img', logger)
        #ec2izeImage("/media/host/xyz-main-partition.img", logger)
        #amiTools = AMITools()
    
        #AMITools("/opt/EC2TOOLS").bundleImage("/media/host/xyz-main-partition.img", 
        #                                      "/media/host/xyz-bundle/", 
        #                                      ec2Cred, settings['userid'])
    
    
        #AMITools("/opt/EC2TOOLS").uploadBundle("ee.ut.cs.cloud/testupload/" + str(time.time()), 
        #                                       "/media/host/xyz-bundle/xyz-main-partition.img.manifest.xml", 
        #                                       settings['accessKey'], 
        #                                       settings['secretKey'])
       
        amiTools = AMITools("/opt/EC2TOOLS", settings['accessKey'], settings['secretKey'])
        """
        outputImg = "/media/host/opt/d2c/job/1299148902.92/openfoam.vdi.main"
        
        #AMICreator.ec2izeImage(outputImg, logger)       

        logger.write("Bundling AMI")
        jobDir = '/media/host/opt/d2c/job/1299148902.92'
        bundleDir = jobDir + "/bundle"
        manifest = amiTools.bundleImage(outputImg, 
                                               bundleDir, 
                                               ec2Cred,
                                               settings['userid'])
    
        manifest = "/media/host/opt/d2c/job/1299148902.92/bundle/openfoam.vdi.main.manifest.xml"
        logger.write("Uploading bundle")
        s3ManifestPath = amiTools.uploadBundle("ee.ut.cs.cloud/testupload/" + str(time.time()), 
                                                     manifest)
        """
        s3ManifestPath = "ee.ut.cs.cloud/testupload/1299158318.77/openfoam.vdi.main.manifest.xml"
        logger.write("Registering AMI: " + s3ManifestPath)
        amiId = amiTools.registerAMI(s3ManifestPath)        

if __name__ == '__main__':
    unittest.main()



