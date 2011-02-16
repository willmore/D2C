'''
Created on Feb 15, 2011

@author: willmore
'''

class EC2Cred:
    
    ec2_cert = None #file path
    ec2_private_key = None #file path
    
    def __init__(self, ec2_cert, ec2_private_key):   
        self.ec2_cert = ec2_cert
        self.ec2_private_key = ec2_private_key
        
    def __str__(self):
        return "EC2Cred: \n\tcert: %s\n\tprivate_key: %s" % (self.ec2_cert, self.ec2_private_key)