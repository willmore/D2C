'''
Created on Apr 12, 2011

@author: willmore
'''
import unittest
import os

import pwd

from d2c.model.FileExistsFinishedCheck import FileExistsFinishedCheck
from boto.ec2.instance import Instance
from d2c.data.CredStore import CredStore
from d2c.model.EC2Cred import EC2Cred
from mockito import *

def get_username():
    return pwd.getpwuid( os.getuid() )[ 0 ]


class FileExistsFinishedCheckTest(unittest.TestCase):


    def testCanonical(self):
        
        fileName = '/tmp/foobar'
        open(fileName, 'a').close()
             
        instance = mock(Instance)
        instance.key_name = "dummyKey"
        instance.public_dns_name = 'localhost'
        
        credStore = mock(CredStore)
        ec2Cred = mock(EC2Cred)
        ec2Cred.private_key = "/home/willmore/.ssh/id_rsa_nopw"
        when(credStore).getEC2Cred("dummyKey").thenReturn(ec2Cred)
        
        checker = FileExistsFinishedCheck(fileName, credStore, user='willmore')
        
        self.assertTrue(checker.check(instance))


if __name__ == "__main__":
    #import sys;sys.argv = ['', 'Test.testName']
    unittest.main()