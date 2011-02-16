'''
Created on Feb 10, 2011

@author: willmore
'''

from d2c.model.SourceImage import SourceImage
from d2c.model.EC2Cred import EC2Cred
from d2c.model.AWSCred import AWSCred

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
    
    def saveAWSCred(self, aws_cred):
        c = self._conn.cursor()
        
        c.execute("select count(*) from aws_cred")
        if c.fetchone()[0] == 0:
            c.execute("insert into aws_cred values (1, ?,?)", (aws_cred.access_key_id, aws_cred.secret_access_key))
        else:
            c.execute("update aws_cred set access_key_id=?, secret_access_key=? where id = 1", (aws_cred.access_key_id, aws_cred.secret_access_key))
        
        self._conn.commit()
        c.close()
        
    def saveEC2Cred(self, ec2Cred):
        c = self._conn.cursor()
        
        c.execute("select count(*) from ec2_cred")
        
        if c.fetchone()[0] == 0:
            c.execute("insert into ec2_cred values (1, ?,?)", (ec2Cred.ec2_cert, ec2Cred.ec2_private_key))
        else:
            c.execute("update ec2_cred set cert=?, private_key=? where id = 1", (ec2Cred.ec2_cert, ec2Cred.ec2_private_key))
        
        self._conn.commit()
        c.close()
    
    
    def getAWSCred(self):
        
        c = self._conn.cursor()
        
        c.execute("select access_key_id, secret_access_key from aws_cred where id = 1")
        row = c.fetchone()
        
        if row is None:
            return None
        
        cred = AWSCred(row[0], row[1])
    
        c.close()
        
        return cred    
        
    
    def getEC2Cred(self):
        c = self._conn.cursor()
        
        c.execute("select cert, private_key from ec2_cred where id=1")
        row = c.fetchone()
        
        if row is None:
            return None
        
        cred = EC2Cred(row[0], row[1])
    
        c.close()
        
        return cred   
        
    

             

        
    