'''
Created on Mar 3, 2011

@author: willmore
'''
import os
import subprocess
from subprocess import Popen
import string
import platform
import shlex
import re
import shutil
import logger
import boto.ec2

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

    


class AMIToolsFactory:
    
    def __init__(self):
        pass
    
    def getAMITools(self, ec2_tools, accessKey, secretKey, logger):
        return AMITools(ec2_tools, accessKey, secretKey, logger)

class AMITools:

    def __init__(self, ec2_tools, accessKey, secretKey, logger):
        
        self.__logger = logger
        
        #TODO un-hardcode
        region = "eu-west-1"
        #TODO: add timeout - if network connection fails, this will spin forever
        #TODO: add configurable region
        self.__logger.write("Initiating connection to ec2 region '%s'..." % region)
        self.__ec2Conn = boto.ec2.connect_to_region(region, aws_access_key_id=accessKey, 
                                                    aws_secret_access_key=secretKey)
        self.__logger.write("EC2 connection established")
        
        self.__EC2_TOOLS = ec2_tools
        
        self.__accessKey = accessKey
        self.__secretKey = secretKey
        
        
    def __execCmd(self, cmd):
    
        self.__logger.write("Executing: " + cmd)    
        
        p = Popen(cmd, shell=True,
               stdout=subprocess.PIPE, stderr=subprocess.STDOUT, close_fds=True)
        
        while True:
            line = p.stdout.readline()
            if not line: break
            self.__logger.write(line)
    
        # will block until process finishes and p.returncode is set
        p.wait()
        
        if 0 != p.returncode:
            raise Exception("Command failed with code %d '" % p.returncode) 
    
    def registerAMI(self, manifest):
       
        return self.__ec2Conn.register_image(image_location=manifest)
    
    def uploadBundle(self, bucket, manifest):
        __UPLOAD_CMD = "export EC2_HOME=%s; %s/bin/ec2-upload-bundle -b %s -m %s -a %s -s %s"
        
        uploadCmd = __UPLOAD_CMD % (self.__EC2_TOOLS, self.__EC2_TOOLS, 
                                    bucket, manifest, 
                                    self.__accessKey, self.__secretKey)
        
        self.__logger.write("Executing: " + uploadCmd)
        
        self.__logger.write(subprocess.call(uploadCmd, shell=True))
        
        return bucket + "/" + os.path.basename(manifest)

    def bundleImage(self, img, destDir, ec2Cred, userId):
    
        if not os.path.exists(destDir):
            os.makedirs(destDir)
    
        __BUNDLE_CMD = "export EC2_HOME=%s; %s/bin/ec2-bundle-image -i %s -c %s -k %s -u %s -r %s -d %s --kernel %s"
    
        arch = "i386"
        kernelId = "aki-4deec439" # eu west pygrub, no initrd specification necessary
        
        bundleCmd = __BUNDLE_CMD % (self.__EC2_TOOLS, self.__EC2_TOOLS, img, ec2Cred.cert, ec2Cred.private_key, 
                                    userId, arch, destDir, kernelId)
        
        self.__logger.write("Executing: " + bundleCmd)
        
        self.__logger.write(subprocess.call(bundleCmd, shell=True))
        
        return destDir + "/" + os.path.basename(img) + ".manifest.xml"

    def extractRawImage(self, srcImg, destImg, log=logger.DevNullLogger()):
        self.__execCmd("VBoxManage clonehd -format RAW %s %s" % (srcImg, destImg))

    def extractMainPartition(self, fullImg, outputImg):
        if "Linux" == platform.system():
            self.__extractMainPartitionLinux(fullImg, outputImg, logger)
        else:
            raise UnsupportedPlatformError(platform.system())

    def ec2izeImage(self, partitionImg, logger=logger.DevNullLogger()):
            """
            Mounts an image and does the following:
            1. Writes new kernel and modules.
            2. Writes an /etc/fstab to it suitable for EC2.
               The preexisting fstab will be preserved as /etc/fstab.save
            """
            
            mntPoint = "/opt/d2c/mnt/"
            mntSrc = "/opt/d2c/mntsrc"
            assert isinstance(partitionImg, basestring)
           
            fstab = None
           
            try:
                #Step 1: mount
                os.symlink(partitionImg, mntSrc)
                cmd = "mount %s" % (partitionImg,)
                logger.write("Executing: " + cmd)
                
                proc = subprocess.Popen(shlex.split(cmd),
                             stdout=subprocess.PIPE, 
                             stderr=subprocess.PIPE, 
                             close_fds=True)
                (_, stderroutput) = proc.communicate()
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
                (_, stderroutput) = proc.communicate()
                if 0 != proc.returncode:
                    raise Exception("Extracting kernel to image did not succeed. Error output:\n\t" + stderroutput)
            
            
                #Step 3: Save old fstab
                fromFile = mntPoint + "/etc/fstab"
                toFile = mntPoint + "/etc/fstab.save"
                logger.write("Saving image's old fstab")
                logger.write("Executing: mv /etc/fstab")
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
                    logger.write("Closing image's /etc/fstab")
                    fstab.close()
                    
                if os.path.ismount(mntPoint):
                    #Step 5: unmount
                    logger.write("Unmounting " + mntPoint)
                    proc = subprocess.Popen(("umount", "/dev/loop0"), #/dev/loop0 is temp hack
                                     stdout=subprocess.PIPE, 
                                     stderr=subprocess.PIPE, 
                                     close_fds=True)
                    (_, stderroutput) = proc.communicate()
                    if proc.returncode != 0:
                        logger.write("Unmount failed: " + stderroutput)
                        
                    logger.write("Done EC2ing image")
                    
                if os.path.exists(mntSrc):
                    os.unlink(mntSrc)
      
            
            
    def __extractMainPartitionLinux(self, fullImg, outputImg):
        
        self.__logger.write("Img is %s" % fullImg)
        cmd = "/sbin/fdisk -lu %s" % fullImg
        self.__logger.write("Executing: " + cmd)
        
        p = subprocess.Popen(shlex.split(cmd), stdout=subprocess.PIPE, 
                                 stdin=subprocess.PIPE, stderr=subprocess.PIPE, close_fds=True)
          
        (stdoutdata, _) = p.communicate()
                      
        partitions = []
           
        for line in string.split(stdoutdata, "\n"):
            match = re.match("(\S+)[\s\*]+(\d+)[\s]+(\d+)[\s]+(\S+)[\s]+(\S+)[\s]+(.+)", line)
            if match is not None:
                print match.groups()
                partitions.append({'start':int(match.group(2)), 'end':int(match.group(3)), 
                                   'system':match.group(6)})
        
        self.__logger.write("Image partitions are: " + str(partitions))
    
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
        self.__logger.write("Executing: " + cmd)
        self.__logger.write(subprocess.call(shlex.split(cmd)))
        self.__logger.write("Done extracting partition.")