'''
Created on Apr 12, 2011

@author: willmore
'''
import unittest
import os

import os
import pwd

from MicroMock import MicroMock
from d2c.model.FileExistsFinishedCheck import FileExistsFinishedCheck

def get_username():
    return pwd.getpwuid( os.getuid() )[ 0 ]


class FileExistsFinishedCheckTest(unittest.TestCase):


    def testCanonical(self):
        checker = FileExistsFinishedCheck('/tmp/foobar', get_username(), '/home/willmore/.ssh/id_rsa_nopw')
        self.assertTrue(checker.check(MicroMock(public_dns_name='localhost')))


if __name__ == "__main__":
    #import sys;sys.argv = ['', 'Test.testName']
    unittest.main()