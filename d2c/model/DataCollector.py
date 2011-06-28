from d2c.ShellExecutor import ShellExecutor
from d2c.logger import StdOutLogger
import os


class DataCollector(object):

    def __init__(self, 
                 source, 
                 sshCred,
                 logger=StdOutLogger()):
        
        assert source is not None
        
        self.source = source
        self.logger = logger
        self.sshCred = sshCred
     
    def collect(self, instance, destinationDir): 
        
        dest = "%s/%s/" % (destinationDir, instance.id)
        
        if not os.path.exists(os.path.dirname(dest)):
            os.makedirs(os.path.dirname(dest))
        
        dest = dest + os.path.basename(self.source)
        
        cmd = "scp -r -i %s -o StrictHostKeyChecking=no %s@%s:%s %s" % (self.sshCred.privateKey, 
                                                                        self.sshCred.username,
                                                                        instance.public_dns_name, 
                                                                        self.source,
                                                                        dest)
        self.executorFactory.executor(self.logger).run(cmd)