import unittest
import os

import pwd

from d2c.RemoteShellExecutor import RemoteShellExecutor
from d2c.AsyncRemoteShellExecutor import AsyncRemoteShellExecutor
import time

def get_username():
    return pwd.getpwuid( os.getuid() )[ 0 ]


class RemoteShellExecutorTest(unittest.TestCase):


    def testCanonical(self):
        
        testFile = "/tmp/foo.txt"
        if (os.path.exists(testFile)):
            os.unlink(testFile)
        self.assertFalse(os.path.exists(testFile))
       
        cmd = "sleep 1 && echo foo > %s" % testFile
        
        
        executor = RemoteShellExecutor("willmore", "localhost", 
                                       "/home/willmore/.ssh/id_rsa_nopw")
        executor.run(cmd)
        
        #Race condition, but 10 sec should be enough
        time.sleep(2)
        self.assertTrue(os.path.exists(testFile))
        
    def testNewLine(self):
        
        testFile = "/tmp/foo.txt"
        if (os.path.exists(testFile)):
            os.unlink(testFile)
        self.assertFalse(os.path.exists(testFile))
       
        cmd = "sleep 1 && echo -e \"foo\\nbar\" > %s" % testFile
        
        
        executor = RemoteShellExecutor("willmore", "localhost", 
                                       "/home/willmore/.ssh/id_rsa_nopw")
        executor.run(cmd)
        
        #Race condition, but 10 sec should be enough
        time.sleep(2)
        self.assertTrue(os.path.exists(testFile))
        file = open(testFile, 'r')
        lines = file.readlines()
        file.close()
        
        self.assertEquals(2, len(lines))
        
        self.assertEquals("foo\n", lines[0])
        self.assertEquals("bar\n", lines[1])
        
    def testAsyncCanonical(self):
    
        
        testFile = "/tmp/foo.txt"
        if (os.path.exists(testFile)):
            os.unlink(testFile)
        self.assertFalse(os.path.exists(testFile))
       
        cmd = "sleep 1 && echo foo > %s" % testFile
        
        
        executor = AsyncRemoteShellExecutor("willmore", "localhost", 
                                       "/home/willmore/.ssh/id_rsa_nopw")
        executor.run(cmd)
        
        #Race condition, but 10 sec should be enough
        self.assertFalse(os.path.exists(testFile))
        time.sleep(2)
        self.assertTrue(os.path.exists(testFile))
        
    def testAsyncNewLine(self):
        
        testFile = "/tmp/foo.txt"
        if (os.path.exists(testFile)):
            os.unlink(testFile)
        self.assertFalse(os.path.exists(testFile))
       
        cmd = "sleep 1 && echo \"foo\nbar\" > %s" % testFile
        
        
        executor = AsyncRemoteShellExecutor("willmore", "localhost", 
                                       "/home/willmore/.ssh/id_rsa_nopw")
        executor.run(cmd)
        
        #Race condition, but 10 sec should be enough
        time.sleep(2)
        self.assertTrue(os.path.exists(testFile))
        file = open(testFile, 'r')
        lines = file.readlines()
        file.close()
        
        self.assertEquals(2, len(lines))
        
        self.assertEquals("foo\n", lines[0])
        self.assertEquals("bar\n", lines[1])


if __name__ == "__main__":
    #import sys;sys.argv = ['', 'Test.testName']
    unittest.main()