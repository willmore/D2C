from d2c.RemoteShellExecutor import RemoteShellExecutor

class FileExistsFinishedCheck(object):

    def __init__(self, fileName, sshCred=None):
        
        self.fileName = fileName
        
        self.sshCred = sshCred
    
    class LineChecker:
        
        def __init__(self, match):
            self.match = match
            self.matchFound = False
            
        def write(self, line):
            if self.match == line.strip():
                self.matchFound = True
        
        
    def check(self, instance):
        '''
        Log into the instance via SSH and test self.fileName for existence.
        Return True if the file exists on the remote host.
        '''    
        
        checker = self.LineChecker('exists')
        cmd = "if [ -f %s ]; then echo exists; fi" % self.fileName
        RemoteShellExecutor(self.sshCred.username, 
                            instance.public_dns_name, 
						    self.sshCred.privateKey, 
                            outputLogger=checker).run(cmd)
        
        return checker.matchFound
    
    def copy(self):
        
        return FileExistsFinishedCheck(self.fileName)