'''
Created on Feb 15, 2011

@author: willmore
'''
import wx
from d2c.model.EC2Cred import EC2Cred
from d2c.model.AWSCred import AWSCred
from d2c.model.Configuration import Configuration

class ConfController:
    
    def __init__(self, credView, dao):
        
        self._credView = credView
        self._dao = dao
         
        conf = dao.getConfiguration()
        
        if conf is not None: 
            if conf.ec2ToolHome is not None:
                self._credView.ec2_tool_home.WriteText(conf.ec2ToolHome)
            
            if conf.awsUserId is not None:
                self._credView.aws_user_id.WriteText(conf.awsUserId)
         
            if conf.awsCred is not None:
                self._credView._aws_key_id.WriteText(conf.awsCred.access_key_id)
                self._credView._aws_secret_access_key.WriteText(conf.awsCred.secret_access_key)
       
            if conf.ec2Cred is not None:
                self._credView._ec2_cert.WriteText(conf.ec2Cred.cert)
                self._credView._ec2_private_key.WriteText(conf.ec2Cred.private_key)  
        
        credView._updateButton.Bind(wx.EVT_BUTTON, self._onSave)
        
    def _onSave(self, event):
        
        
        awsCred = AWSCred("mainKey", self._credView._aws_key_id.GetValue(),
                          self._credView._aws_secret_access_key.GetValue())
        
        ec2Cred = EC2Cred("defaultEC2Cred", self._credView._ec2_cert.GetValue(),
                self._credView._ec2_private_key.GetValue())
        
        ec2ToolHome = self._credView.ec2_tool_home.GetValue();
        awsUserId = self._credView.aws_user_id.GetValue();
        
        conf = Configuration(ec2ToolHome=ec2ToolHome,
                             awsUserId=awsUserId,
                             ec2Cred=ec2Cred,
                             awsCred=awsCred)
        
        self._dao.saveConfiguration(conf)
        
        self._credView.EndModal(wx.ID_OK)
