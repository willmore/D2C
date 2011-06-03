'''
Created on Feb 16, 2011

@author: willmore
'''

import os
import time

from d2c.data.DAO import DAO

from guestfs import GuestFS

def can_create_ami(srcImg):
    """
    returns true if an AMI can be created
    """
    dao = DAO()
    
    return None is dao.getAMIBySrcImg(srcImg)

class AMICreator:
    '''
        Encapsulates all procedures to covert a VirtualBox VDI to an Amazon S3-backed AMI
    '''
    
    __JOB_ROOT = "/media/host/opt/d2c/job"
    __IMAGE_DIR = "/tmp/d2c/data/images/"
    
    def __init__(self, srcImg, ec2Cred, 
                 userId, s3Bucket, amiTools, 
                 logger):
        self.__srcImg = srcImg
        self.__ec2Cred = ec2Cred
        self.__userId = userId
        self.__s3Bucket = s3Bucket
        self.__amiTools = amiTools
        self.__logger = logger
        self.__dao = DAO()
    
    def createAMI(self):
        """
        Creates AMI and returns the newly created AMI ID
        """
        self.__logger.write("Extracting raw image from VDI")

        jobId = str(time.time())
        jobDir = self.__JOB_ROOT + "/" + jobId
        
        os.makedirs(jobDir)
        self.__logger.write("Job directory is: " + jobDir)
           
        imgName = os.path.basename(self.__srcImg)
        rawImg = jobDir + "/" + imgName + ".raw"
        
        self.__amiTools.extractRawImage(self.__srcImg, rawImg, self.__logger)
        
        self.__logger.write("Raw img created")
        
        self.__logger.write("Extracting main partition")
        #we only support one partition now
        outputImg = jobDir + "/" + imgName + ".main"
        self.__amiTools.extractMainPartition(rawImg, outputImg)
        
        self.__logger.write("EC2izing image")
        
        gf = GuestFS ()
        gf.set_trace(1)
        gf.set_autosync(1)
        gf.set_qemu('/usr/local/bin/qemu-system-x86_64')
        gf.add_drive(outputImg)
        self.__logger.write("Launching libguestfs - this could take a bit.")
        gf.launch()
        
        self.__amiTools.ec2izeImage(gf)       
        
        #TODO remove duplicate arch test
        TEST_FILE = "/bin/sh"
        arch = gf.file_architecture(gf.readlink(TEST_FILE)) if (gf.is_symlink(TEST_FILE)) else gf.file_architecture(TEST_FILE)
            
        #sync and close handle
        del gf

        self.__logger.write("Bundling AMI")
        bundleDir = jobDir + "/bundle"
        manifest = self.__amiTools.bundleImage(outputImg, 
                                               bundleDir, 
                                               self.__ec2Cred,
                                               self.__userId,
                                               arch) 
    
        self.__logger.write("Uploading bundle")
        s3ManifestPath = self.__amiTools.uploadBundle("ee.ut.cs.cloud/testupload/" + str(time.time()), 
                                                     manifest)
    
        self.__logger.write("Registering AMI: " + s3ManifestPath)
        amiId = self.__amiTools.registerAMI(s3ManifestPath)     
        
        self.__dao.addAMI(amiId, self.__srcImg)
        ami = self.__dao.getAMIById(amiId)
        
        return ami    
    
        