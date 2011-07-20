class SSHCred(object):
    
    def __init__(self, id, name, username, privateKey):
        
        assert isinstance(username, basestring)
        assert isinstance(privateKey, basestring)
        
        self.id = id
        self.name = name
        self.username = username
        self.privateKey = privateKey