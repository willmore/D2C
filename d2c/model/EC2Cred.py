'''
Created on Feb 15, 2011

@author: willmore
'''

class EC2Cred(object):
    
    cert = None #file path
    private_key = None #file path
    
    def __init__(self, id, ec2_cert, ec2_private_key):   
        self.id = id
        self.cert = ec2_cert
        self.private_key = ec2_private_key
        
    def __str__(self):
        return "EC2Cred: \n'tid: %s\n\tcert: %s\n\tprivate_key: %s" % (self.id, self.cert, self.private_key)