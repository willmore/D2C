'''
Created on Apr 12, 2011

@author: willmore
'''
import unittest
import os

import pwd

from d2c.model.UploadAction import UploadAction
from boto.ec2.instance import Instance
from d2c.model.SSHCred import SSHCred
import random
from mockito import *

def get_username():
    return pwd.getpwuid( os.getuid() )[ 0 ]


class UploadActionTest(unittest.TestCase):


    def testCanonical(self):
             
        instance = mock(Instance)
        instance.key_name = "dummyKey"
        instance.public_dns_name = 'localhost'
        
        sshCred = mock(SSHCred)
        sshCred.privateKey = "/home/willmore/.ssh/id_rsa_nopw"
        sshCred.username = "willmore"
        
        
        src = "/tmp/srcFile"
        dest = "/tmp/dest.%d" % random.randint(1,10000)
        
        f = open(src, "a")
        f.close()
        
        UploadAction(src, dest, sshCred).execute(instance)        
        
        self.assertTrue(os.path.isfile(dest))

if __name__ == "__main__":
    #import sys;sys.argv = ['', 'Test.testName']
    unittest.main()