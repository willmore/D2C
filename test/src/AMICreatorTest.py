import unittest
#import d2c.AMICreator
import string
import time
import sys

#sys.path.append("/home/willmore/workspace/cloud/src")
print sys.path
from d2c.logger import StdOutLogger
from d2c.model.EC2Cred import EC2Cred
from d2c.AMITools import AMITools
from d2c.EC2ConnectionFactory import EC2ConnectionFactory
from d2c.data.DAO import DAO
from d2c.data.CredStore import CredStore
#import d2c.AMICreator as AMICreator

class AMICreatorTest():

    def main(self):
         
        
        settings = {}
        for l in open("/home/willmore/test.conf", "r"):
            (k, v) = string.split(l.strip(), "=")
            settings[k] = v
        
        ec2Cred = EC2Cred("default", settings['cert'], settings['privateKey'])
        
        
        logger = StdOutLogger();
        
        amiTools = AMITools("/opt/EC2TOOLS", settings['accessKey'], 
                            settings['secretKey'], EC2ConnectionFactory(settings['accessKey'], 
                            settings['secretKey'], StdOutLogger()), 
                            StdOutLogger(),
                            kernelDir="../../data/kernels")
        '''
        jobDir = "/media/host/opt/d2c/job/1304594602.11/"
        partitionImg = jobDir + "cloud-hpcc.vdi.main"
        amiTools.ec2izeImage(partitionImg)
                
        logger.write("Bundling AMI")
        bundleDir = jobDir + "/bundle"
        manifest = amiTools.bundleImage(partitionImg,
                                               bundleDir,
                                               ec2Cred,
                                               settings['userid'])
    
        logger.write("Uploading bundle")
        s3ManifestPath = amiTools.uploadBundle("ee.ut.cs.cloud/testupload/" + str(time.time()),
                                                     manifest)
        '''
        s3ManifestPath="ee.ut.cs.cloud/testupload/hpcc/cloud-hpcc.vdi.main.manifest.xml"
        logger.write("Registering AMI: " + s3ManifestPath)
        amiId = amiTools.registerAMI(s3ManifestPath)       
        print "AMIID = " + amiId

if __name__ == '__main__':
    AMICreatorTest().main()



