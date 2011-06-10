'''
Created on Feb 16, 2011

@author: willmore
'''

import os
import tempfile

from d2c.AMITools import AMITools
from d2c.logger import StdOutLogger
from d2c.model.EC2Cred import EC2Cred
from d2c.model.AWSCred import AWSCred
from d2c.model.Kernel import Kernel
from d2c.data.DAO import DAO
from d2c.model.Cloud import Cloud

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
                 cloud, kernel,
                 dao, outputDir=None,
                 logger=StdOutLogger()):
        
        assert isinstance(srcImg, basestring)
        assert isinstance(ec2Cred, EC2Cred)
        assert isinstance(awsCred, AWSCred)
        assert isinstance(cloud, Cloud)
        assert isinstance(kernel, Kernel)
        assert isinstance(dao, DAO)
        
        self.__srcImg = srcImg
        self.__ec2Cred = ec2Cred
        self.__awsCred = awsCred
        self.__userId = userId
        self.__s3Bucket = s3Bucket
        self.__amiTools = AMITools(logger)
        self.__logger = logger
        self.__cloud = cloud
        self.__kernel = kernel
        
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
       
        if not self.__kernel in self.__cloud.getKernels():
            raise Exception("Kernel %s not in cloud kernels" % str(self.__kernel))
       
        arch = self.__amiTools.getArch(self.__srcImg)
        
        if arch != self.__kernel.arch:
            raise Exception("Image architecture %s does not match kernel architecture %s." % (arch, self.__kernel.arch))
        
        self.__logger.write("EC2izing image")
        self.__logger.write("Job directory is: " + self.__outputDir)
        
        newImg = self.__amiTools.ec2izeImage(self.__srcImg, self.__outputDir, 
                                             self.__kernel, self.__cloud.getFStab())       

        self.__logger.write("Bundling AMI")
        bundleDir =  self.__outputDir + "/bundle"
        manifest = self.__amiTools.bundleImage(newImg, 
                                               bundleDir, 
                                               self.__ec2Cred,
                                               self.__userId,
                                               self.__cloud,
                                               self.__kernel) 
    
        self.__logger.write("Uploading bundle")
        s3ManifestPath = self.__amiTools.uploadBundle(self.__cloud,
                                                      self.__s3Bucket, 
                                                      manifest, self.__awsCred)
    
        self.__logger.write("Registering AMI: " + s3ManifestPath)
        amiId = self.__amiTools.registerAMI(s3ManifestPath, self.__cloud, self.__awsCred)     
        
        self.__dao.addAMI(amiId, self.__srcImg)
        ami = self.__dao.getAMIById(amiId)
        
        return ami    
    
        