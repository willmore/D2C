'''
Created on Feb 10, 2011

@author: willmore
'''

from d2c.model.SourceImage import SourceImage
from d2c.model.EC2Cred import EC2Cred
from d2c.model.AWSCred import AWSCred
from d2c.model.Configuration import Configuration

import sqlite3


class DAO:
    
    _SQLITE_FILE = "/tmp/d2c_db.sqlite"
      
    def __init__(self):
        self._conn = sqlite3.connect(self._SQLITE_FILE)  
        self._init_db()
        
    def _init_db(self):
        
        c = self._conn.cursor()
        
        c.execute('''create table if not exists aws_cred
                    (id integer primary key, access_key_id text, secret_access_key text)''')
        
        c.execute('''create table if not exists ec2_cred
                    (id integer primary key, cert text, private_key text)''')
        
        c.execute('''create table if not exists conf
                    (key text, value text)''')
        
        c.execute('''create table if not exists src_img
                    (path text primary key)''')
        
        self._conn.commit()
        c.close()
          
    def getSourceImages(self):
        c = self._conn.cursor()

        out = []

        for r in c.execute("select path from src_img"):
            out.append(SourceImage(r[0]))
        
        c.close()
        
        return out   
    
    def addSourceImage(self, srcImgPath):
        c = self._conn.cursor()

        c.execute("insert into src_img values (?)", (srcImgPath,))
        
        self._conn.commit()
        c.close()
        
    def saveConfiguration(self, conf):
        c = self._conn.cursor()
        
        self.__saveConfigurationValue(c, 'ec2ToolHome', conf.ec2ToolHome)
        self.__saveConfigurationValue(c, 'awsUserId', conf.awsUserId)
        
        if conf.ec2Cred is not None:
            self.__saveConfigurationValue(c, 'ec2Cert', conf.ec2Cred.cert)
            self.__saveConfigurationValue(c, 'ec2PrivateKey', conf.ec2Cred.private_key)
  
        if conf.awsCred is not None:
            self.__saveConfigurationValue(c, 'awsAccessKeyId', conf.awsCred.access_key_id)
            self.__saveConfigurationValue(c, 'awsSecretAccessKey', conf.awsCred.secret_access_key)
        
        self._conn.commit()
        c.close()
        
    def getConfiguration(self):
        c = self._conn.cursor()
        
        ec2ToolHome = self.__getConfigurationValue(c, 'ec2ToolHome')
        awsUserId = self.__getConfigurationValue(c, 'awsUserId')
                    
        ec2Cert = self.__getConfigurationValue(c, 'ec2Cert')
        ec2PrivateKey = self.__getConfigurationValue(c, 'ec2PrivateKey')
              
        awsAccessKeyId = self.__getConfigurationValue(c, 'awsAccessKeyId')
        awsSecretAccessKey = self.__getConfigurationValue(c, 'awsSecretAccessKey')
        
        self._conn.commit()
        c.close()
        
        ec2Cred = EC2Cred(ec2Cert, ec2PrivateKey) if (ec2Cert is not None and ec2PrivateKey is not None) else None
        awsCred = AWSCred(awsAccessKeyId, awsSecretAccessKey) if (awsAccessKeyId is not None and awsSecretAccessKey is not None) else None
            
        return Configuration(ec2ToolHome=ec2ToolHome,
                             awsUserId=awsUserId,
                             ec2Cred=ec2Cred,
                             awsCred=awsCred)
        
    def __getConfigurationValue(self, cursor, key): 
        h = cursor.execute("select value from conf where key=? limit 1", (key,)) 
        
        r = h.fetchone()
        
        if r is not None:
            return r[0]
        else:
            return None
                            
        
    def __saveConfigurationValue(self, cursor, key, value):

        if None is cursor.execute("select * from conf where key==?", (key,)).fetchone():
            cursor.execute("insert into conf (key, value) values (?,?)", (key, value))
        else:
            cursor.execute("update conf set value=? where key=?", (value, key))
    
   
        
    

             

        
    
