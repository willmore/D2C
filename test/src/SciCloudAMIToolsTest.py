
import string

from d2c.logger import StdOutLogger
from d2c.EC2ConnectionFactory import EC2ConnectionFactory

from d2c.AMITools import AMITools
import guestfs
from d2c.model.Region import Region
from d2c.model.EC2Cred import EC2Cred

class SciCloudTest():


    def main(self):
        disk = '/home/willmore/Downloads/linux-0.2.img'

        '''
        gf = guestfs.GuestFS ()
        gf.set_trace(1)
        gf.set_autosync(1)
        gf.set_qemu('/usr/local/bin/qemu-system-x86_64')
        gf.add_drive(disk)
        gf.launch()
        '''
        
        settings = {}
        for l in open("/home/willmore/scicloud.conf", "r"):
            (k, v) = string.split(l.strip(), "=")
            settings[k] = v
        
        amiTools = AMITools("/opt/EC2_TOOLS", settings['accessKey'], 
                            settings['secretKey'], EC2ConnectionFactory(settings['accessKey'], 
                            settings['secretKey'], StdOutLogger()), 
                            StdOutLogger())
        
        #amiTools.ec2izeImage(gf)
        #Causes sync and handle to close
        #del gf
        
        ec2Cred = EC2Cred("default", settings['cert'], settings['privateKey'])
        region = Region("SciCloud", "http://172.17.60.242:8773/services/Eucalyptus")
        manifest = amiTools.bundleImage(disk, "d2c-test/test-images", region, ec2Cred, 
                             settings['userid'], AMITools.ARCH_X86, "eki-94411709")
        
        amiId = amiTools.registerAMI(manifest)

if __name__ == '__main__':
    SciCloudTest().main()



