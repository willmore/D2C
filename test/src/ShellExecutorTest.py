import unittest
import os

import pwd

from d2c.ShellExecutor import ShellExecutor
from d2c.AsyncRemoteShellExecutor import AsyncRemoteShellExecutor
import time

def get_username():
    return pwd.getpwuid( os.getuid() )[ 0 ]


class ShellExecutorTest(unittest.TestCase):


    def testDD(self):
        
        ShellExecutor().run("dd if=/dev/zero of=/tmp/tmp123 bs=1024 count=1000000")
        print "Should run after dd"
    


if __name__ == "__main__":
    #import sys;sys.argv = ['', 'Test.testName']
    unittest.main()