import unittest
import d2c.AMICreator

class AMICreatorTest(unittest.TestCase):


    def test_full(self):
         settings = {}
         for l in open("/home/willmore/test.conf", "r"):
             (k, v) = string.split(l.strip(), "=")
             settings[k] = v
        
 
         ec2Cred = EC2Cred(settings['cert'], settings['privateKey'])
    
         logger = logger.StdOutLogger();
    
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
    
    AMITools("/opt/EC2TOOLS", settings['accessKey'], settings['secretKey']).registerAMI("ee.ut.cs.cloud/testupload/1298626840.76/xyz-main-partition.img.manifest.xml")
            

if __name__ == '__main__':
    unittest.main()



