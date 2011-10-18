from d2c.logger import StdOutLogger
import os

class UploadAction(object):

    def __init__(self, 
                 source, 
                 destination,
                 sshCred,
                 logger=StdOutLogger()):
        
        assert isinstance(source, basestring) 
        assert len(source) > 0
        assert isinstance(destination, basestring)
        assert len(destination) > 0
    
        self.source = source
        self.destination = destination
        self.logger = logger
        self.sshCred = sshCred
        
    def copy(self):      
        return UploadAction(self.source, self.destination, self.sshCred)
     
    def execute(self, instance, shellVars):   
            
        #assert os.path.isdir(self.source) or os.path.isfile(self.source), "Source not present: %s" % str(self.source)    
       
        cmd = "scp -i %(key)s -o StrictHostKeyChecking=no %(src)s %(user)s@%(host)s:%(dest)s" % \
            {'key':self.sshCred.privateKey,
             'src':self.source,
             'dest':self.destination,
             'user':self.sshCred.username,
             'host':instance.public_dns_name}
            
        self.executorFactory.executor(self.logger, self.logger).run(cmd)
