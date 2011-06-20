class SSHCred:
    
    def __init__(self, username, privateKey):
        
        assert isinstance(username, basestring)
        assert isinstance(privateKey, basestring)
        
        self.username = username
        self.privateKey = privateKey