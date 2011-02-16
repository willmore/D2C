'''
Created on Feb 15, 2011

@author: willmore
'''
import wx
from d2c.model.EC2Cred import EC2Cred
from d2c.model.AWSCred import AWSCred


class CredController:
    
    def __init__(self, credView, dao):
        
        self._credView = credView
        self._dao = dao
         
        awsCred = dao.getAWSCred()
        ec2Cred = dao.getEC2Cred()
        
        if awsCred is not None:
            self._credView._aws_key_id.WriteText(awsCred.access_key_id)
            self._credView._aws_secret_access_key.WriteText(awsCred.secret_access_key)
       
        if ec2Cred is not None:
            self._credView._ec2_cert.WriteText(ec2Cred.ec2_cert)
            self._credView._ec2_private_key.WriteText(ec2Cred.ec2_private_key)  
        
        credView._updateButton.Bind(wx.EVT_BUTTON, self._onSave)
        
    def _onSave(self, event):
        self._dao.saveAWSCred(AWSCred(self._credView._aws_key_id.GetValue(),
                                      self._credView._aws_secret_access_key.GetValue()))
        
        self._dao.saveEC2Cred(EC2Cred(self._credView._ec2_cert.GetValue(),
                                      self._credView._ec2_private_key.GetValue()))