'''
Created on Feb 16, 2011

@author: willmore
'''

import os
import subprocess
import string
import platform
import shlex
import re
import shutil
import logger
from model.EC2Cred import EC2Cred

class UnsupportedPlatformError(Exception):
    def __init__(self, value):
        self.value = value
    def __str__(self):
        return repr(self.value)
    
class UnsupportedImageError(Exception):
    def __init__(self, value):
        self.value = value
    def __str__(self):
        return repr(self.value)


def extractRawImage(srcImg, destImg, log=logger.DevNullLogger()):
    cmd = "VBoxManage clonehd -format RAW %s %s" % (srcImg, destImg)
    log.write("Executing: " + cmd)
    subprocess.call(shlex.split(cmd))


def extractMainPartition(fullImg, outputImg, logger=logger.DevNullLogger()):
    #if "Darwin" == platform.system():
        #    self.__extractMainPartitionMac(fullImg, outputImg)
    if "Linux" == platform.system():
        __extractMainPartitionLinux(fullImg, outputImg, logger)
    else:
        raise UnsupportedPlatformError(platform.system())

def ec2izeImage(partitionImg, logger=logger.DevNullLogger()):
        """
        Mounts an image and does the following:
        1. Writes new kernel and modules.
        2. Writes an /etc/fstab to it suitable for EC2.
           The preexisting fstab will be preserved as /etc/fstab.save
        """
        
        mntPoint = "/opt/d2c/mnt/"
        assert isinstance(partitionImg, basestring)
       
        fstab = None
       
        try:
            #Step 1: mount
            cmd = "mount %s" % (partitionImg,)
            logger.write("Executing: " + cmd)
            
            proc = subprocess.Popen(shlex.split(cmd),
                         stdout=subprocess.PIPE, 
                         stderr=subprocess.PIPE, 
                         close_fds=True)
            (stdoutput, stderroutput) = proc.communicate()
            if 0 != proc.returncode:
                raise Exception("Mount command did not succeed '" + cmd +". Error output:\n\t" + stderroutput)
            
            #Step 2: copy kernel
            kernelSrcTar = "../data/kernels/2.6.35-24-virtual.tar"
        
            cmd = "tar -C %s -xf %s" % (mntPoint, kernelSrcTar)
            logger.write("Executing: " + cmd)
            proc = subprocess.Popen(shlex.split(cmd),
                         stdout=subprocess.PIPE, 
                         stderr=subprocess.PIPE, 
                         close_fds=True)
            (stdoutput, stderroutput) = proc.communicate()
            if 0 != proc.returncode:
                raise Exception("Extracting kernel to image did not succeed. Error output:\n\t" + stderroutput)
        
        
            #Step 3: Save old fstab
            fromFile = mntPoint + "/etc/fstab"
            toFile = mntPoint + "/etc/fstab.save"
            logger.write("Saving image's old fstab")
            logger.write("Executing: mv %/etc/fstab")
            shutil.copyfile(fromFile, toFile)
        
            #Step 4: write out fstab
            logger.write("Writting out new EC2 /etc/fstab")
            fstab = open(fromFile, 'w')
            fstab.write("/dev/sda1\t/\text4\tdefaults\t1\t1\n")
            fstab.write("none\t/dev/pts\tdevpts\tgid=5,mode=620\t0\t0\n")
            fstab.write("none\t/dev/shm\ttmpfs\tdefaults\t0\t0\n")
            fstab.write("none\t/proc\tproc\tdefaults\t0\t0\n")
            fstab.write("none\t/sys\tsysfs\tdefaults\t0\t0\n")
            fstab.write("/dev/sda2\t/mnt\text3\tdefaults\t0\t0\n")
            fstab.write("/dev/sda3\tswap\tswap\tdefaults\t0\t0\n")
            
        
        finally:
            if fstab is not None and not fstab.closed:
                logger.write("Closing images /etc/fstab")
                fstab.close()
        
            if os.path.ismount(mntPoint):
                #Step 5: unmount
                logger.write("Unmounting " + mntPoint)
                proc = subprocess.Popen(("umount", mntPoint),
                                 stdout=subprocess.PIPE, 
                                 stderr=subprocess.PIPE, 
                                 close_fds=True)
                (stdoutput, stderroutput) = proc.communicate()
                if proc.returncode != 0:
                    logger.write("Unmount failed: " + stderroutput)
                    
                logger.write("Done EC2ing image")
    
def __extractMainPartitionMac(fullImg, outputImg):
        '''
        This code does not work, as OSX cannot mount rw ext4
        '''
        
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
        
        count = (max_len - max_start)
        cmd = "/bin/dd if=%s of=%s bs=512 skip=%d count=%s" % (fullImg, outputImg, start, count)
        print "Execing: " + cmd
        
        resp = subprocess.call(string.split(cmd, " "))
        
        
def __extractMainPartitionLinux(fullImg, outputImg, logger=logger.DevNullLogger()):
    
    logger.write("Img is %s" % fullImg)
    cmd = "/sbin/fdisk -lu %s" % fullImg
    logger.write("cmd is " + cmd)
    
    p = subprocess.Popen(shlex.split(cmd), stdout=subprocess.PIPE, 
                             stdin=subprocess.PIPE, stderr=subprocess.PIPE, close_fds=True)
      
    (stdoutdata, stderrdata) = p.communicate()
                  
    partitions = []
       
    for line in string.split(stdoutdata, "\n"):
        match = re.match("(\S+)[\s\*]+(\d+)[\s]+(\d+)[\s]+(\S+)[\s]+(\S+)[\s]+(.+)", line)
        if match is not None:
            print match.groups()
            partitions.append({'start':int(match.group(2)), 'end':int(match.group(3)), 
                               'system':match.group(6)})
    
    print partitions

    linuxPartition = None

    for p in partitions:
        if p['system'] == 'Linux':
            if linuxPartition is None:
                linuxPartition = p
            else:
                raise UnsupportedImageError("More than one Linux partition found. Only one partition supported.")
                
    if linuxPartition is None:
        raise UnsupportedImageError("No Linux partition found")       
    
    blockSize = 512 #TODO check what the real size is
    
    cmd = "/bin/dd if=%s of=%s bs=%d skip=%d count=%d" % (fullImg, 
                                                          outputImg, 
                                                          blockSize, 
                                                          linuxPartition['start'], 
                                                          linuxPartition['end'] - linuxPartition['start'] + 1)
    logger.write("Executing: " + cmd)
    res = subprocess.call(shlex.split(cmd))
    logger.write("Done extracting partition.")
    

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
        subprocess.call(("VBoxManage", "clonehd", "-format", "RAW", self.__srcImg, fullImg))                
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
    
    def __extractRawImage(self, srcImg, destImg):
        extractRawImage(srcImg, destImg)
        
    
          
    def __extractMainPartition(self, fullImg, outputImg):
       extractMainPartition(fullImg, outputImg)
        

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
    for l in open("/home/willmore/test.conf", "r"):
        (k,v) = string.split(l.strip(), "=")
        settings[k] = v
        
 
    ec2Cred = EC2Cred(settings['cert'], settings['privateKey'])
    
    logger = logger.StdOutLogger();
    
    #extractRawImage('/media/host/xyz.vdi', '/media/host/xyz-full.img', logger)
    #extractMainPartition('/media/host/xyz-full.img', '/media/host/xyz-main-partition.img', logger)
    ec2izeImage("/media/host/xyz-main-partition.img", logger)
    #amiTools = AMITools()
    
    #amiTools.bundleImage("/opt/EC2TOOLS", "/tmp/d2c/data/images/ubuntu3.img", "/tmp/amibundle/", 
    #                     ec2Cred, settings['userid'])
    