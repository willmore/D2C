'''
Created on Feb 16, 2011

@author: willmore
'''

import os
import tempfile

from d2c.model.AMI import AMI
from d2c.logger import StdOutLogger
from d2c.model.EC2Cred import EC2Cred
from d2c.model.AWSCred import AWSCred
from d2c.model.Kernel import Kernel
from d2c.data.DAO import DAO
from d2c.model.Cloud import Cloud
from d2c.model.SourceImage import SourceImage

class UnsupportedImageError(Exception):
    def __init__(self, value):
        self.value = value
    def __str__(self):
        return repr(self.value)

class AMICreator:
    '''
    Encapsulates all procedures to covert desktop VM images to an Amazon S3-backed AMI.
    '''
    
    def __init__(self, srcImg, 
                 ec2Cred, awsCred,
                 userId, s3Bucket,
                 cloud, kernel,
                 dao, 
                 amiToolsFactory,
                 outputDir=None,
                 logger=StdOutLogger(),
                 ramdisk=None):
        
        assert isinstance(srcImg, SourceImage)
        assert isinstance(ec2Cred, EC2Cred)
        assert isinstance(awsCred, AWSCred)
        assert isinstance(cloud, Cloud)
        assert isinstance(kernel, Kernel)
        assert isinstance(dao, DAO)
        assert outputDir is None or isinstance(outputDir, basestring)
        
        self.__srcImg = srcImg
        self.__ec2Cred = ec2Cred
        self.__awsCred = awsCred
        self.__userId = userId
        self.__s3Bucket = s3Bucket
        self.__amiTools = amiToolsFactory.getAMITools(logger)
        self.__logger = logger
        self.__cloud = cloud
        self.__kernel = kernel
        self.__ramdisk = ramdisk
        
        if outputDir is None:
            outputDir = tempfile.mkdtemp()
        
        self.__outputDir = outputDir
        self.__dao = dao
    
    def createAMI(self):
        """
        Creates AMI from source image, uploads to storage, 
        registers, and returns the new AMI object.
        """
        
        self.__logger.write("Staring AMI creation process")
        
        if not os.path.exists(self.__outputDir):
            os.makedirs(self.__outputDir)
       
        cloudKernels = self.__cloud.kernels
        if not self.__kernel in cloudKernels:
            raise Exception("Kernel %s not in cloud kernels %s" % (str(self.__kernel), str(cloudKernels)))
       
        arch = self.__amiTools.getArch(self.__srcImg)
        
        if arch != str(self.__kernel.architecture.arch):
            raise Exception("Image architecture %s does not match kernel architecture %s." % (arch, self.__kernel.architecture.arch))
        
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
                                               self.__kernel,
                                               self.__ramdisk) 
    
        self.__logger.write("Uploading bundle")
        s3ManifestPath = self.__amiTools.uploadBundle(self.__cloud,
                                                      self.__s3Bucket, 
                                                      manifest, self.__awsCred)
    
        self.__logger.write("Registering AMI: %s" % s3ManifestPath)
        amiId = self.__amiTools.registerAMI(s3ManifestPath, self.__cloud, self.__awsCred)     
        
        ami = AMI(None, self.__srcImg.image, amiId, self.__cloud, kernel=self.__kernel)
        self.__dao.add(ami)
        
        return ami    
    
        