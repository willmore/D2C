'''
Created on Feb 16, 2011

@author: willmore
'''

import os
import logger
import time

from d2c.data.DAO import DAO



class AMICreator:
    '''
        Encapsulates all procedures to covert a VirtualBox VDI to an Amazon S3-backed AMI
    '''
    
    __JOB_ROOT = "/media/host/opt/d2c/job"
    __IMAGE_DIR = "/tmp/d2c/data/images/"
    
    
    def __init__(self, srcImg, ec2Cred, 
                 userId, s3Bucket, amiTools, 
                 logger=logger.DevNullLogger()):
        self.__srcImg = srcImg
        self.__ec2Cred = ec2Cred
        self.__userId = userId
        self.__s3Bucket = s3Bucket
        self.__amiTools = amiTools
        self.__logger = logger
    
    def createAMI(self):
        """
        Creates AMI and returns the newly created AMI ID
        """
        self.__logger.write("Extracting raw image from VDI")

        jobId = str(time.time())
        jobDir = self.__JOB_ROOT + "/" + jobId
        self.__logger.write("Job directory is: " + jobDir)
        
        imgName = os.path.basename(self.__srcImg)
        rawImg = jobDir + "/" + imgName + ".raw"
        
        self.__amiTools.extractRawImage(self.__srcImg, rawImg, self.__logger)
        
        self.__logger.write("Raw img created")
        
        self.__logger.write("Extracting main partition")
        #we only support one partition now
        outputImg = jobDir + "/" + imgName + ".main"
        self.__amiTools.extractMainPartition(rawImg, outputImg, self.__logger)
        
        self.__logger.write("EC2izing image")
        self.__amiTools.ec2izeImage(outputImg, self.__logger)       

        self.__logger.write("Bundling AMI")
        bundleDir = jobDir + "/bundle"
        manifest = self.__amiTools.bundleImage(outputImg, 
                                               bundleDir, 
                                               self.__ec2Cred,
                                               self.__userId)
    
    
        self.__logger.write("Uploading bundle")
        s3ManifestPath = self.__amiTools.uploadBundle("ee.ut.cs.cloud/testupload/" + str(time.time()), 
                                                     manifest)
    
        self.__logger.write("Registering AMI: " + s3ManifestPath)
        amiId = self.__amiTools.registerAMI(s3ManifestPath)
        
        dao = DAO()
        dao.addAMI(amiId, self.__srcImg)
        
        return amiId     
    
        