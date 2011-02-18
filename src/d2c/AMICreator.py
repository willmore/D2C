'''
Created on Feb 16, 2011

@author: willmore
'''

import os
import subprocess
import string
import platform
from d2c.model.EC2Cred import EC2Cred

class UnsupportedPlatformError(Exception):
    def __init__(self, value):
        self.value = value
    def __str__(self):
        return repr(self.value)




class AMICreator:
    '''
        Encapsulates all procedures to covert a VirtualBox VDI to an Amazon S3-backed AMI
    '''
    
    __JOB_ROOT = "/tmp/d2c/job/"
    __IMAGE_DIR = "/tmp/d2c/data/images/"
    
    
    def __init__(self, srcImg, ec2Cred, s3Cred, userId, s3Bucket):
        self.__srcImg = srcImg
        self.__ec2Cred = ec2Cred
        self.__userId = userId
        self.__s3Cred = s3Cred
        self.__s3Bucket = s3Bucket
    
    def createAMI(self):
        jobId = "foobar" #TODO generate something meaningful here
        
        jobDir = self.__JOB_ROOT + jobId
        mntDir = jobDir + "/mnt"
        subprocess.Popen(("mkdir", "-p", mntDir))
        subprocess.Popen(("mkdir", "-p", self.__IMAGE_DIR))
        
        fullImg = self.__IMAGE_DIR + string.replace(os.path.basename(self.__srcImg), ".vdi", ".fullImg")
        partitionImg = self.__IMAGE_DIR + string.replace(os.path.basename(self.__srcImg), ".vdi", ".img")

        print "Creating raw img at: " + fullImg
        #subprocess.call(("VBoxManage", "clonehd", "-format", "RAW", self.__srcImg, fullImg))                
        print "Raw img created"
        
        print "Extracting main partition"
        #we only support one partition now
        self.__extractMainPartition(fullImg, partitionImg)
        
        print "Fixing image"
        self.__fixImage(partitionImg)
        
        print "Finally creating AMI"
        manifest = self.bundleImage(partitionImg)
        manifestFile = self.uploadBundle(manifest)
        self.registerAMI(self.__s3Bucket, manifestFile)
        
    def __fixImage(self, partitionImg):
        #TODO
        return None
    
    
        
    def __extractMainPartition(self, fullImg, outputImg):
        if "Darwin" == platform.system():
            self.__extractMainPartitionMac(fullImg, outputImg)
        else:
            raise UnsupportedPlatformError(platform.system())
    
    def __extractMainPartitionMac(self, fullImg, outputImg):
        print "Img is %s" % fullImg
        cmd = ("/usr/sbin/fdisk", "-d", fullImg)
        lines = subprocess.check_output(cmd)
        
        # Examine first two fields of each line,
        # save start and length of largest partition.
        # We only support one partition now.
        
        max_start = 0
        max_len = 0
        
        for line in string.split(lines, "\n"):
            parts = string.split(line, ",")[:2]
            
            if len(parts) != 2:
                continue
            
            (start, length) = map(int, parts)
            
            if length > max_len:
                max_len = length
                max_start = start
        
        print "Max start: %d; max len: %d " % (max_start, max_len)
        
        count = (max_len - start) + 1 # add one for some reason...
        cmd = "/bin/dd if=%s of=%s bs=512 skip=%d count=%s" % (fullImg, outputImg, start, count)
        print "Execing: " + cmd
        
        resp = subprocess.call(string.split(cmd, " "))
        
        
    def __extractMainPartitionLinux(self, fullImg, outputImg):
        #TODO
        print "Img is %s" % fullImg
        cmd = "/sbin/fdisk -lu %s 2>&1 | grep Linux | grep -iv swap" % fullImg
        print "cmd is " + cmd
        line = subprocess.call("fdisk -lu %s 2>&1 | grep Linux | grep -iv swap" % fullImg)
        print line
        #TODO finish


class AMITools: 
    
    
    def __init__(self, ec2_tools="/opt/EC2TOOLS"):
    
        self.__EC2_TOOLS = ec2_tools
    
    def bundleImage(self, ec2Home, img, destDir, ec2Cred, userId):
    
        __BUNDLE_CMD = "export EC2_HOME=%s; %s/bin/ec2-bundle-image -i %s -c %s -k %s -u %s -r %s -d %s"
    
        arch = "i386"
        
        bundleCmd = __BUNDLE_CMD % (self.__EC2_TOOLS, ec2Home, img, ec2Cred.cert, ec2Cred.private_key, 
                                    userId, arch, destDir)
        
        print "CMD = " + bundleCmd
        
        out = subprocess.call(bundleCmd, shell=True)
        
if __name__ == "__main__":
    
    settings = {}
    for l in open("/Users/willmore/test.conf", "r"):
        (k,v) = string.split(l.strip(), "=")
        settings[k] = v
        
    #ami = AMICreator("/Users/willmore/Documents/Research/ubuntu3.vdi")
    #ami.createAMI()
    ec2Cred = EC2Cred(settings['cert'], settings['privateKey'])
    
    amiTools = AMITools()
    
    amiTools.bundleImage("/opt/EC2TOOLS", "/tmp/d2c/data/images/ubuntu3.img", "/tmp/amibundle/", 
                         ec2Cred, settings['userid'])
    