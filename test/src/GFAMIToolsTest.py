
import string

from d2c.logger import StdOutLogger
from d2c.EC2ConnectionFactory import EC2ConnectionFactory

from d2c.AMITools import AMITools
import guestfs



class GFAMICreatorTest():


    def main(self):
        disk = '/home/willmore/Downloads/dsl-4.4.10-x86.vdi'

        gf = guestfs.GuestFS ()
        gf.set_trace(1)
        gf.set_autosync(1)
        gf.set_qemu('/usr/local/bin/qemu-system-x86_64')
        gf.add_drive(disk)
        gf.launch()
        
        settings = {}
        for l in open("/home/willmore/test.conf", "r"):
            (k, v) = string.split(l.strip(), "=")
            settings[k] = v
        
        amiTools = AMITools("/opt/EC2TOOLS", settings['accessKey'], 
                            settings['secretKey'], EC2ConnectionFactory(settings['accessKey'], 
                            settings['secretKey'], StdOutLogger()), 
                            StdOutLogger())
        
        amiTools.ec2izeImage(gf)
        #Causes sync and handle to close
        del gf

if __name__ == '__main__':
    GFAMICreatorTest().main()



