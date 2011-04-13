'''
Created on Apr 12, 2011

@author: willmore
'''
import unittest
import os

import pwd

from d2c.model.DataCollector import DataCollector
from boto.ec2.instance import Instance
from d2c.data.CredStore import CredStore
from d2c.model.EC2Cred import EC2Cred
from mockito import *
import random

def get_username():
    return pwd.getpwuid( os.getuid() )[ 0 ]


class DataCollectorTest(unittest.TestCase):


    def testCanonical(self):
        
        src = "/tmp/d2c.test/collect_src"
        dest = "/tmp/d2c.test/collect_dest/%d" % random.randint(0,100)
        
        
        if not os.path.exists(os.path.dirname(src)):
            os.mkdir(os.path.dirname(src))
            
        open(src, 'a').close()
             
        instance = mock(Instance)
        instance.key_name = "dummyKey"
        instance.public_dns_name = 'localhost'
        
        credStore = mock(CredStore)
        ec2Cred = mock(EC2Cred)
        ec2Cred.private_key = "/home/willmore/.ssh/id_rsa_nopw"
        when(credStore).getEC2Cred("dummyKey").thenReturn(ec2Cred)
        
        collector = DataCollector(src, dest, credStore=credStore, user='willmore')
        collector.collect(instance)
        
        self.assertTrue(os.path.exists(dest))


if __name__ == "__main__":
    #import sys;sys.argv = ['', 'Test.testName']
    unittest.main()