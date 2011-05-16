from d2c.logger import StdOutLogger
from d2c.ShellExecutor import ShellExecutor

import os

class UploadAction():

    def __init__(self, 
                 source, 
                 destination,
                 sshCred,
                 logger=StdOutLogger()):
        
        assert isinstance(source, basestring)
        assert isinstance(destination, basestring)
        
        self.source = source
        self.destination = destination
        self.logger = logger
        self.sshCred = sshCred
     
    def execute(self, instance):   
            
        assert os.path.isdir(self.source) or os.path.isfile(self.source)    
       
        cmd = "scp -i %(key)s %(src)s %(user)s@%(host)s:%(dest)s" % \
            {'key':self.sshCred.privateKey,
             'src':self.source,
             'dest':self.destination,
             'user':self.sshCred.username,
             'host':instance.public_dns_name}
            
        ShellExecutor(self.logger, self.logger).run(cmd)
