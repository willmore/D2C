'''
Created on Feb 16, 2011

@author: willmore
'''

import os
import time
import tempfile

from d2c.AMITools import AMITools
from d2c.logger import StdOutLogger

class UnsupportedImageError(Exception):
    def __init__(self, value):
        self.value = value
    def __str__(self):
        return repr(self.value)

class AMICreator:
    '''
    Encapsulates all procedures to covert a VirtualBox VDI to an Amazon S3-backed AMI
    '''
    
    def __init__(self, srcImg, 
                 ec2Cred, awsCred,
                 userId, s3Bucket,
                 region, s3Storage,
                 dao, outputDir=None,
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
        
        if outputDir is None:
            outputDir = tempfile.mkdtemp()
        
        self.__outputDir = outputDir
        self.__dao = dao
    
    def createAMI(self):
        """
        Creates AMI from source image, uploads to storage, 
        registers, and returns the new AMI handle object.
        """
        
        if not os.path.exists(self.__outputDir):
            os.makedirs(self.__outputDir)
       
        arch = self.__amiTools.getArch(self.__srcImg)
        
        kernel = self.__region.getKernel(arch)
        
        if kernel is None:
            raise UnsupportedImageError("Cannot get kernel for architecture %s in region %s" % 
                                        (arch, self.__region))
        
        self.__logger.write("EC2izing image")
        self.__logger.write("Job directory is: " + self.__outputDir)
        
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
                                                      self.__s3Bucket, 
                                                      manifest, self.__awsCred)
    
        self.__logger.write("Registering AMI: " + s3ManifestPath)
        amiId = self.__amiTools.registerAMI(s3ManifestPath, self.__region, self.__awsCred)     
        
        self.__dao.addAMI(amiId, self.__srcImg)
        ami = self.__dao.getAMIById(amiId)
        
        return ami    
    
        