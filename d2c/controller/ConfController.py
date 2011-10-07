import wx
from d2c.model.EC2Cred import EC2Cred
from d2c.model.AWSCred import AWSCred
from d2c.model.Configuration import Configuration

class ConfController:
    
    def __init__(self, credView, dao):
        
        self._credView = credView
        self._dao = dao
        
        credView.addButton.Bind(wx.EVT_BUTTON, self._onAdd)
        credView.closeButton.Bind(wx.EVT_BUTTON, self._onClose)
        
    def _onClose(self, _):
        self._credView.close()
        
    def _onAdd(self, _):
        self._credView.addCred()
        
    def _onSave(self, event):
        

        awsCred = AWSCred("mainKey", self._credView._aws_key_id.GetValue(),
                          self._credView._aws_secret_access_key.GetValue())
        
        ec2Cred = EC2Cred("defaultEC2Cred", self._credView._ec2_cert.GetValue(),
                self._credView._ec2_private_key.GetValue())
        
        awsUserId = self._credView.aws_user_id.GetValue()
        
        conf = Configuration(awsUserId=awsUserId,
                             ec2Cred=ec2Cred,
                             awsCred=awsCred)
        
        self._dao.saveConfiguration(conf)
        
        self._credView.EndModal(wx.ID_OK)
