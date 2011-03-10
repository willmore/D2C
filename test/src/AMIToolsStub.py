'''
Created on Mar 9, 2011

@author: willmore
'''
import os

class AMIToolsFactoryStub:
    
    def __init__(self):
        pass
    
    def getAMITools(self, ec2_tools, accessKey, secretKey, logger):
        return AMIToolsStub(logger)

class AMIToolsStub:
    
    def __init__(self, logger):
        self.__logger=logger
    
    def extractRawImage(self, srcImg, destImg, log):
        self.__logger.write("Extracting image")

    def extractMainPartition(self, fullImg, outputImg):
        self.__logger.write("Extracting main partition")
    
    def ec2izeImage(self, partitionImg, logger):
        self.__logger.write("E2izing image")
        
    def uploadBundle(self, bucket, manifest):
        
        self.__logger.write("Uploading bundle")
        
        return "DUMMY" + bucket + "/" + os.path.basename(manifest)

    def bundleImage(self, img, destDir, ec2Cred, userId):
    
        self.__logger.write("Bundling image")
        return "DUMMY." + destDir + "/" + os.path.basename(img) + ".manifest.xml"
    
    def registerAMI(self, manifest):
        self.__logger.write("Registering AMI")
        return "dummyAMIID"