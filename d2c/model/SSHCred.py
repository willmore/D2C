class SSHCred(object):
    
    def __init__(self, id, username, privateKey):
        
        assert isinstance(username, basestring)
        assert isinstance(privateKey, basestring)
        
        self.id = id
        self.username = username
        self.privateKey = privateKey