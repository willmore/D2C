'''
Created on Feb 15, 2011

@author: willmore
'''


class AWSCred:
    
    access_key_id = None
    secret_access_key = None
    
    def __init__(self, aws_access_key_id, aws_secret_access_key):
        self.access_key_id = aws_access_key_id;
        self.secret_access_key = aws_secret_access_key;
        
    def __str__(self):
        return "AWSCred: \n\tkey_id:%s\nacces_key:\t%s" % (self.access_key_id, self.secret_access_key)