from d2c.RemoteShellExecutor import RemoteShellExecutor

class FileExistsFinishedCheck:

    def __init__(self, fileName):
        
        self.fileName = fileName
        self.cmd = "[ -f %s ] && echo exists" % fileName
    
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
        cred = self.dao.getEC2Cred(instance.key_name)
        
        RemoteShellExecutor('ec2-user', 
                            instance.public_dns_name, 
						    cred.private_key, 
                            outputLogger=checker).run(self.cmd)
        
        return checker.matchFound
        