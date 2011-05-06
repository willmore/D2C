import unittest
#import d2c.AMICreator
import string
import time
import sys

#sys.path.append("/home/willmore/workspace/cloud/src")
print sys.path
from d2c.logger import StdOutLogger
from d2c.model.EC2Cred import EC2Cred
from d2c.AMITools import AMITools
#import d2c.AMICreator as AMICreator
import guestfs


class GFAMICreatorTest():


    def main(self):
        disk = '/home/willmore/Downloads/dsl-4.4.10-x86.vdi'

        g = guestfs.GuestFS ()
        g.set_qemu('/usr/local/bin/qemu-system-x86_64')
        # Attach the disk image read-only to libguestfs.
        g.add_drive_opts (disk, readonly=1)
        
        

if __name__ == '__main__':
    GFAMICreatorTest().main()



