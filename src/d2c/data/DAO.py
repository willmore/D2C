'''
Created on Feb 10, 2011

@author: willmore
'''

from d2c.model.SourceImage import SourceImage



class DAO:
        
    def getSourceImages(self):
        return [SourceImage("foobar"), SourceImage("foobar")]
    
    def getAWSCred(self):
        return AWSCred(aws_acces_key_id="dummy_aws_acces_key_id",
                       aws_secret_access_key="dummy_aws_secret_access_key")
    
    def getEC2Cred(self):
        return EC2Cred(ec2_cert="dummy_ec2_cert",
                       ec2_private_key="dummy_ec2_private_key")
        
    
class EC2Cred:
    
    _ec2_cert = None #file path
    _ec2_private_key = None #file path
    
    def __init__(self, ec2_cert, ec2_private_key):   
        self._ec2_cert = ec2_cert
        self._ec2_private_key = ec2_private_key
             

class AWSCred:
    
    _aws_acces_key_id = None
    _aws_secret_access_key = None
    
    def __init__(self, aws_acces_key_id, aws_secret_access_key):
        self._aws_acces_key_id = aws_acces_key_id;
        self._aws_secret_access_key = aws_secret_access_key;
        
    