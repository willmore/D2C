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
from d2c.model.EC2Cred import EC2Cred

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


def extractRawImage(srcImg, destImg):
    subprocess.call(("VBoxManage", "clonehd", "-format", "RAW", srcImg, destImg))


def extractMainPartition(fullImg, outputImg):
    #if "Darwin" == platform.system():
        #    self.__extractMainPartitionMac(fullImg, outputImg)
    if "Linux" == platform.system():
        __extractMainPartitionLinux(fullImg, outputImg)
    else:
        raise UnsupportedPlatformError(platform.system())
    
    
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
        
        
def __extractMainPartitionLinux(fullImg, outputImg):
    #TODO
    print "Img is %s" % fullImg
    cmd = "/sbin/fdisk -lu %s" % fullImg
    print "cmd is " + cmd
        
    p = subprocess.Popen(shlex.split(cmd), stdout=subprocess.PIPE, 
                             stdin=subprocess.PIPE, stderr=subprocess.PIPE, close_fds=True)
      
    (stdoutdata, stderrdata) = p.communicate()
              
    partitionLines = False
    
    partitions = []
       
    for line in string.split(stdoutdata, "\n"):
        print line
        print "-----"
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
    print "Executing: " + cmd
    res = subprocess.call(shlex.split(cmd))
    print "Done!"
    

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
        
    def __fixImage(self, partitionImg):
        #TODO
        
        mntPoint = "/tmp/d2c_mnt/"
        
        #Step 1: mount
        proc = subprocess.Popen(("sudo", "mount", mntPoint, partitionImg),
                         stdout=subprocess.PIPE, 
                         stdin=subprocess.PIPE, 
                         stderr=subprocess.PIPE, 
                         close_fds=True)
        (stdoutput, stderroutput) = proc.communicate()
        if 0 != proc.returnCode:
            raise Exception("Mount command did not succeed. Error output:\n\t" + stderroutput)
        
        #Step 2: copy kernel
        kernelSrcTar = "data/2.6.35-24-virtual.tar"
        proc = subprocess.Popen(("tar", "-C", mntPoint, "-xf", kernelSrcTar),
                         stdout=subprocess.PIPE, 
                         stdin=subprocess.PIPE, 
                         stderr=subprocess.PIPE, 
                         close_fds=True)
        (stdoutput, stderroutput) = proc.communicate()
        if 0 != proc.returnCode:
            raise Exception("Extracting kernel to image did not succeed. Error output:\n\t" + stderroutput)
        
        
        #Step 3: Save old fstab
        shutil.copyfile(mntPoint + "/etc/fstab", mntPoint + "/etc/fstab.save")
        
        #Step 4: write out fstab
        f = open(mntPoint + "/etc/fstab", 'w')
        f.write("/dev/sda1       /       ext4    defaults        1 1\n\
                none            /dev/pts        devpts  gid=5,mode=620  0 0\n\
                none            /dev/shm        tmpfs   defaults        0 0\n\
                none    /proc   proc    defaults        0 0\n\
                none    /sys    sysfs   defaults        0 0\n\
                /dev/sda2       /mnt    ext3    defaults        0 0\n\
                /dev/sda3       swap    swap    defaults        0 0\n")
        
        #Step 5: unmount
        subprocess.Popen(("sudo", "mount", mntPoint, partitionImg))
          
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
        
    #ami = AMICreator("/Users/willmore/Documents/Research/ubuntu3.vdi")
    #ami.createAMI()
    ec2Cred = EC2Cred(settings['cert'], settings['privateKey'])
    
    #extractRawImage('/media/host/ubuntu.vdi', '/media/host/ubuntu.img')
    
    #extractRawImage('/media/host/ubuntu-small.vdi', '/media/host/ubuntu-small.img')
    extractMainPartition('/media/host/ubuntu-small.img', '/media/host/ubuntu-main-partition.img')
    
    #amiTools = AMITools()
    
    #amiTools.bundleImage("/opt/EC2TOOLS", "/tmp/d2c/data/images/ubuntu3.img", "/tmp/amibundle/", 
    #                     ec2Cred, settings['userid'])
    