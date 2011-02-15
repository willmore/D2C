'''
Created on Feb 10, 2011

@author: willmore
'''

from d2c.model.SourceImage import SourceImage
import sqlite3


class DAO:
    
    _SQLLITE_FILE = "d2c_db"
      
    def __init__(self):
        self._conn = sqlite3.connect(self._SQLLITE_FILE)
        
        self._init_db()
        
    def _init_db(self):
        
        c = self._conn.cursor()
        
        c.execute('''create table if not exists aws_cred
                    (access_key_id text, secret_access_key text)''')
        
        self._conn.commit()
        c.close()
          
    def getSourceImages(self):
        return [SourceImage("foobar"), SourceImage("foobar")]
    
    def setAWSCred(self, aws_cred):
        c = self._conn.cursor()
        
        c.execute("insert into aws_cred values (?,?)", (aws_cred.access_key_id, aws_cred.secret_access_key))
        
        self._conn.commit()
        c.close()
    
    
    def getAWSCred(self):
        
        c = self._conn.cursor()
        
        c.execute("select access_key_id, secret_access_key from aws_cred")
        row = c.fetchone()
        
        cred = AWSCred(row[0], row[1])
    
        c.close()
        
        return cred
        
        
    
    def getEC2Cred(self):
        return EC2Cred(ec2_cert="dummy_ec2_cert",
                       ec2_private_key="dummy_ec2_private_key")
        
    
class EC2Cred:
    
    ec2_cert = None #file path
    ec2_private_key = None #file path
    
    def __init__(self, ec2_cert, ec2_private_key):   
        self.ec2_cert = ec2_cert
        self.ec2_private_key = ec2_private_key
             

class AWSCred:
    
    access_key_id = None
    secret_access_key = None
    
    def __init__(self, aws_access_key_id, aws_secret_access_key):
        self.access_key_id = aws_access_key_id;
        self.secret_access_key = aws_secret_access_key;
        
    