import os
import guestfs
from d2c.logger import StdOutLogger
from d2c.model.AWSCred import AWSCred
from d2c.ShellExecutor import ShellExecutor
from d2c.model.Kernel import Kernel
from d2c.model.EC2Cred import EC2Cred
from d2c.model.Cloud import Cloud

import re

class UnsupportedPlatformError(Exception):
    def __init__(self, value):
        Exception.__init__(self)
        self.value = value
        
    def __str__(self):
        return repr(self.value)
    

class AMIToolsFactory:
    
    def getAMITools(self, logger):
        return AMITools(logger)

class AMITools:

    def __init__(self, logger=StdOutLogger()):
        
        self.__logger = logger     
        
    def registerAMI(self, manifest, cloud, awsCred):
        assert isinstance(cloud, Cloud)
        assert isinstance(manifest, basestring)
        assert isinstance(awsCred, AWSCred)

        return cloud.getConnection(awsCred).registerImage(manifest)
    
    def uploadBundle(self, cloud, bucket, manifest, awsCred):
        
        assert isinstance(cloud, Cloud)
        assert isinstance(bucket, basestring)
        assert isinstance(manifest, basestring)
        assert isinstance(awsCred, AWSCred)

        UPLOAD_CMD = "euca-upload-bundle --url %s -b %s -m %s -a %s -s %s"
        
        uploadCmd = UPLOAD_CMD % (cloud.storageURL,
                                      bucket, manifest,
                                      awsCred.access_key_id,
                                      awsCred.secret_access_key)
        
        ShellExecutor().run(uploadCmd)
            
        return bucket + "/" + os.path.basename(manifest)

    def bundleImage(self, img, destDir, ec2Cred, userId, cloud, kernel, ramdisk=None):
        '''
        Create an AMI bundle.
        '''
    
        assert isinstance(img, basestring)
        assert isinstance(destDir, basestring)
        assert isinstance(ec2Cred, EC2Cred)
        assert isinstance(userId, basestring)
        assert isinstance(kernel, Kernel)
        assert isinstance(cloud, Cloud)
    
        if not os.path.exists(destDir):
            os.makedirs(destDir)
        
        BUNDLE_CMD = "euca-bundle-image -i %s -c %s -k %s -u %s -r %s -d %s --kernel %s --ec2cert %s"
        
        bundleCmd = BUNDLE_CMD % (img, ec2Cred.cert, ec2Cred.private_key, 
                                    userId, kernel.architecture.arch, destDir, kernel.aki,
                                    cloud.getEC2Cert())
        
        if ramdisk is not None:
            bundleCmd += " --ramdisk %s" % ramdisk.id
                
        ShellExecutor(self.__logger).run(bundleCmd)
        
        return destDir + "/" + os.path.basename(img) + ".manifest.xml"
    
        
    def getArch(self, image):
        '''
        Return the determined architecture of a desktop VM image.
        '''
                
        gf = self.__initGuestFS(image.path)
        
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
        
        outputImg = os.path.join(outputDir, os.path.basename(disk.path))
        
        assert not os.path.exists(outputImg)
                      
        try: 
            
            gf = self.__initGuestFS(disk.path)
    
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
            
            gf = self.__initGuestFS(disk.path, outputImg)
            
            newDev = "/dev/vdb" # a guess of name by ordering
       
            self.__logger.write("Copying main partition from source device")
            
            gf.dd(rootDev, newDev)
            
            self.__logger.write("Modifying new image")
            
            rc = gf.fsck("ext4", newDev)
            
            if rc != 0:
                raise Exception("FSCK returned code: %d" % rc)
            
            gf.mount(newDev, "/")     
            
            #Step 2: copy kernel
            contentsPath = kernel.getContentsAbsPath()
            if re.match('.*tgz', contentsPath) or re.match('.*tar\.gz', contentsPath):
                gf.tgz_in(contentsPath, "/") 
            elif re.match('.*tar', contentsPath):
                gf.tar_in(contentsPath, "/")
            else:
                raise Exception("Unknown extension in contents file path: %s" % contentsPath)
            
            gf.umount_all()
            rc = gf.fsck("ext4", newDev)
            if rc != 0:
                raise Exception("FSCK returned code: %d" % rc)
            
            gf.mount(newDev, "/")   
            #Step 3: Save old fstab
            self.__logger.write("Saving image's old fstab")
            gf.mv("/etc/fstab", "/etc/fstab.d2c.save")
                    
            #Step 4: write out fstab
            self.__logger.write("Writing out new EC2-compatible /etc/fstab")
            
            gf.upload(fstab, "/etc/fstab")
            
            gf.umount_all()
            rc = gf.fsck("ext4", newDev)
            
            if rc != 0:
                raise Exception("FSCK returned code: %d" % rc)
        
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
        Returns actual path of 'path', resolving any symlinks, in guestfs handle.
        '''
        if not gf.is_symlink(path):
            return path
        
        link = gf.readlink(path)
        
        if os.path.isabs(link):
            return link
        
        return os.path.join(os.path.dirname(path), link)
        