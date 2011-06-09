import wx


class CompCloudConfController:
    
    def __init__(self, view, dao):
        
        self._view = view
        self._dao = dao
         
        self._view.compCloudConfPanel.setRegions(dao.getRegions())
        self._view.showPanel("MAIN") 
        
        
    '''  
    def _onSave(self, event):
        
        
        awsCred = AWSCred(self._credView._aws_key_id.GetValue(),
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
    '''
