from d2c.ShellExecutor import ShellExecutor
from d2c.logger import StdOutLogger
import os


class DataCollector(object):

    def __init__(self, 
                 source, 
                 sshCred=None,
                 logger=StdOutLogger()):
        
        assert isinstance(source, basestring) and source[0] == '/'
        
        self.source = source
        self.logger = logger
        self.sshCred = sshCred
        
    def copy(self):
        return DataCollector(self.source, self.sshCred)
     
    def collect(self, instance, destination): 
        
        destDir = os.path.dirname(destination)
        if not os.path.exists(destDir):
            os.makedirs(destDir)
        
        cmd = "scp -r -i %s -o StrictHostKeyChecking=no %s@%s:%s %s" % (self.sshCred.privateKey, 
                                                                        self.sshCred.username,
                                                                        instance.public_dns_name, 
                                                                        self.source,
                                                                        destination)
        self.executorFactory.executor(self.logger).run(cmd)