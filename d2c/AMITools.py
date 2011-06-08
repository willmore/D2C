'''
Created on Mar 3, 2011

@author: willmore
'''
import os
import subprocess
from subprocess import Popen
import pkg_resources
import guestfs
from d2c.model.Storage import S3Storage
from d2c.logger import StdOutLogger
from d2c.model.AWSCred import AWSCred

class UnsupportedPlatformError(Exception):
    def __init__(self, value):
        self.value = value
    def __str__(self):
        return repr(self.value)
    


class AMIToolsFactory:
    
    def __init__(self):
        # TODO refactor when possible to inject ec2ConnFactory
        #self.ec2ConnFactory = ec2ConnFactory
        pass
    
    def getAMITools(self, accessKey, secretKey, logger):
        return AMITools(accessKey, secretKey, logger)

class AMITools:

    def __init__(self, logger=StdOutLogger()):
        
        self.__logger = logger
        
   
    def __execCmd(self, cmd):
    
        self.__logger.write("Executing: " + cmd)    
        
        p = Popen(cmd, shell=True,
               stdout=subprocess.PIPE, stderr=subprocess.STDOUT, close_fds=True)
        
        while True:
            line = p.stdout.readline()
            if not line: break
            self.__logger.write(line)
    
        # This call will block until process finishes and p.returncode is set.
        p.wait()
        
        if 0 != p.returncode:
            raise Exception("Command failed with code %d '" % p.returncode) 
    
    def registerAMI(self, manifest, region, awsCred):

        return region.getConnection(awsCred).register_image(image_location=manifest)
    
    def uploadBundle(self, s3Storage, bucket, manifest, awsCred):
        
        assert isinstance(s3Storage, S3Storage)
        assert isinstance(bucket, basestring)
        assert isinstance(manifest, basestring)
        assert isinstance(awsCred, AWSCred)
        
        return s3Storage.bundleUploader().upload(manifest, bucket, awsCred)

    def bundleImage(self, img, destDir, ec2Cred, userId, region, kernel):
    
        if not os.path.exists(destDir):
            os.makedirs(destDir)
    
        BUNDLE_CMD = "ec2-bundle-image -i %s -c %s -k %s -u %s -r %s -d %s --kernel %s"
        
        bundleCmd = BUNDLE_CMD % (img, ec2Cred.cert, ec2Cred.private_key, 
                                    userId, kernel.arch, destDir, kernel.aki)
        
        self.__logger.write("Executing: " + bundleCmd)
        
        self.__execCmd(bundleCmd)
        
        return destDir + "/" + os.path.basename(img) + ".manifest.xml"
    
        
    def getArch(self, image):
        
        gf = self.__initGuestFS(image)
        
        roots = gf.inspect_os()
        assert (len(roots) == 1)
        rootDev = roots[0]
        gf.mount(rootDev, "/")
        
        #Determine architecture by looking at arch of /bin/sh
        TEST_FILE = "/bin/sh"
        arch = gf.file_architecture(self.__guestFSLink(gf, TEST_FILE)) 
        
        del gf
        return arch   
    
    def __initGuestFS(self, *disks):
        
        gf = guestfs.GuestFS ()
        gf.set_trace(1)
        gf.set_autosync(1)
        
        for disk in disks:
            gf.add_drive(disk)
            
        gf.launch()
        
        return gf
            
        
    def ec2izeImage(self, disk, outputDir, kernel, fstab):
        """
        Mounts an image and does the following:
        1. Extract disk's main partition to a new file in outputDir.
        1. Writes new kernel and modules.
        2. Writes an /etc/fstab to it suitable for EC2.
           The pre-existing fstab will be preserved as /etc/fstab.save
        """
        
        assert disk is not None
        assert os.path.isdir(outputDir)
        
        outputImg = os.path.join(outputDir, os.path.basename(disk))
        
        assert not os.path.exists(outputImg)
                      
        try: 
            
            gf = self.__initGuestFS(disk)
    
            roots = gf.inspect_os()
            assert (len(roots) == 1)
            rootDev = roots[0]
            
            partSize = gf.blockdev_getsize64(rootDev)
                           
            self.__logger.write("Creating new disk file %s. Size = %d bytes" % (outputImg,partSize))
            
            f = open (outputImg, "w")
            f.truncate (partSize)
            f.close()
            
            self.__logger.write("Restarting guestfs to add new drive.")
            
            del gf
            
            gf = self.__initGuestFS(disk, outputImg)
            
            newDev = "/dev/vdb" # a guess of name by ordering
       
            self.__logger.write("DDing from root device %s to new device %s" % (rootDev, newDev))
            
            gf.dd(rootDev, newDev)
            
            self.__logger.write("Modifying new image")
            
            gf.mount(newDev, "/")
            
            #Step 2: copy kernel            
            gf.tar_in(kernel.contents, "/") 
                    
            #Step 3: Save old fstab
            self.__logger.write("Saving image's old fstab")
            self.__logger.write("Executing: mv /etc/fstab")
            gf.mv("/etc/fstab", "/etc/fstab.d2c.save")
                    
            #Step 4: write out fstab
            self.__logger.write("Writing out new EC2 /etc/fstab")
            
            gf.upload(fstab, "/etc/fstab")
        
        except Exception as x:
            
            self.__logger.write("Exception encountered: %s" % str(x))
            raise
        
        finally:
            
            if gf is not None:
                del gf
                
        self.__logger.write("Done EC2ing image %s" % outputImg)
        
        return outputImg        
    
    def __guestFSLink(self, gf, path):
        '''
        Returns actual path of 'path' in guestfs handle.
        '''
        if not gf.is_symlink(path):
            return path
        
        link = gf.readlink(path)
        
        if os.path.isabs(link):
            return link
        
        return os.path.join(os.path.dirname(path), link)
        