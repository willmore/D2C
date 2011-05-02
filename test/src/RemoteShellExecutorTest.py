import unittest
import os

import pwd

from d2c.RemoteShellExecutor import RemoteShellExecutor
from d2c.AsyncRemoteShellExecutor import AsyncRemoteShellExecutor
from mockito import *
import time

def get_username():
    return pwd.getpwuid( os.getuid() )[ 0 ]


class RemoteShellExecutorTest(unittest.TestCase):


    def testCanonical(self):
        
        testFile = "/tmp/foo.txt"
        if (os.path.exists(testFile)):
            os.unlink(testFile)
        self.assertFalse(os.path.exists(testFile))
       
        cmd = "nohup sh -c \\\"sleep 10 && echo foo > %s\\\" &> /dev/null </dev/null &" % testFile
        
        
        executor = RemoteShellExecutor("willmore", "localhost", 
                                       "/home/willmore/.ssh/id_rsa_nopw")
        executor.run(cmd)
        
        #Race condition, but 10 sec should be enough
        self.assertFalse(os.path.exists(testFile))
        time.sleep(11)
        self.assertTrue(os.path.exists(testFile))
        
    def testAsyncCanonical(self):
    
        
        testFile = "/tmp/foo.txt"
        if (os.path.exists(testFile)):
            os.unlink(testFile)
        self.assertFalse(os.path.exists(testFile))
       
        cmd = "sleep 10 && echo foo > %s" % testFile
        
        
        executor = AsyncRemoteShellExecutor("willmore", "localhost", 
                                       "/home/willmore/.ssh/id_rsa_nopw")
        executor.run(cmd)
        
        #Race condition, but 10 sec should be enough
        self.assertFalse(os.path.exists(testFile))
        time.sleep(11)
        self.assertTrue(os.path.exists(testFile))


if __name__ == "__main__":
    #import sys;sys.argv = ['', 'Test.testName']
    unittest.main()