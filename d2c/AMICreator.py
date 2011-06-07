'''
Created on Feb 16, 2011

@author: willmore
'''

import os
import time

from d2c.data.DAO import DAO
from d2c.AMITools import AMITools
from d2c.logger import StdOutLogger

class AMICreator:
    '''
    Encapsulates all procedures to covert a VirtualBox VDI to an Amazon S3-backed AMI
    '''
    
    def __init__(self, srcImg, 
                 ec2Cred, awsCred,
                 userId, s3Bucket,
                 region, s3Storage,
                 outputDir,
                 logger=StdOutLogger()):
        
        self.__srcImg = srcImg
        self.__ec2Cred = ec2Cred
        self.__awsCred = awsCred
        self.__userId = userId
        self.__s3Bucket = s3Bucket
        self.__s3Storage = s3Storage
        self.__amiTools = AMITools(logger)
        self.__logger = logger
        self.__region = region
        self.__outputDir = outputDir
        self.__dao = DAO()
    
    def createAMI(self):
        """
        Creates AMI and returns the newly created AMI ID
        """
        self.__logger.write("Extracting raw image from VDI")

        if not os.path.exists(self.__outputDir):
            os.makedirs(self.__outputDir)
            
        self.__logger.write("Job directory is: " + self.__outputDir)
       
        self.__logger.write("EC2izing image")
        
        arch = self.__amiTools.getArch(self.__srcImg)
        kernel = self.__region.getKernel(arch)
        
        newImg = self.__amiTools.ec2izeImage(self.__srcImg, self.__outputDir, 
                                             kernel, self.__region.getFStab())       

        self.__logger.write("Bundling AMI")
        bundleDir =  self.__outputDir + "/bundle"
        manifest = self.__amiTools.bundleImage(newImg, 
                                               bundleDir, 
                                               self.__ec2Cred,
                                               self.__userId,
                                               self.__region,
                                               kernel) 
    
        self.__logger.write("Uploading bundle")
        s3ManifestPath = self.__amiTools.uploadBundle(self.__s3Storage,
                                                      "ee.ut.cs.cloud/testupload/" + str(time.time()), 
                                                      manifest, self.__awsCred)
    
        self.__logger.write("Registering AMI: " + s3ManifestPath)
        amiId = self.__amiTools.registerAMI(s3ManifestPath, self.__awsCred)     
        
        self.__dao.addAMI(amiId, self.__srcImg)
        ami = self.__dao.getAMIById(amiId)
        
        return ami    
    
        