
import wx
from d2c.model.SourceImage import DesktopImage, AMI
from wx.lib.pubsub import Publisher

class RoleChoice(object):
    
    def __init__(self, roleTemp, cloud):
        
        self.roleTemp = roleTemp
        self.sourceImg = roleTemp.image.getHostedSourceImage(cloud)
        self.count = 1
        self.instanceTypes = cloud.instanceTypes
        self.selectedInstanceType = cloud.instanceTypes[0]
        
        self.imgStr = "None Available"
            
        if isinstance(self.sourceImg, AMI):
            self.imgStr = self.sourceImg.amiId
        elif isinstance(self.sourceImg, DesktopImage):
            self.imgStr = self.sourceImg.path
            
    def selectCountEvent(self, evt):
        self.count = evt.GetEventObject().GetValue()
    
    def selectInstanceTypeEvent(self, evt):
        name = evt.GetEventObject().GetValue()
        for t in self.instanceTypes:
            if t.name == name:
                self.selectedInstanceType = t
                

class DeploymentCreatorController(object):
    
    def __init__(self, deploymentCreator, dao):
        
        self._dao = dao
        self._view = deploymentCreator
        self._view.showPanel("SIZE")
        self._view.sizePanel.probSize.Bind(wx.EVT_TEXT, self._validateSize)
        self._view.sizePanel.chooseButton.Bind(wx.EVT_BUTTON, self._selectSize)
        self._view.sizePanel.chooseButton.Disable()
        self._view.cloudPanel.cloudList.setItems(dao.getClouds())
        self._view.cloudPanel.chooseButton.Bind(wx.EVT_BUTTON, self._selectCloud)
        self._view.credPanel.chooseButton.Bind(wx.EVT_BUTTON, self._selectCred)
        self._view.deploymentPanel.doneButton.Bind(wx.EVT_BUTTON, self._createDeployment)
        self._size = None
        self._cloud = None
        self._choices = None
	self._awsCred = None
      
    def _validateSize(self, _):
        
        self._view.sizePanel.chooseButton.Disable()
        
        val = self._view.sizePanel.probSize.GetValue()
        
        if len(val) == 0:
            return
            
        try:
            float(val)
        except:
            return
            
        self._view.sizePanel.chooseButton.Enable()

    def _selectSize(self, _):
        self._size = float(self._view.sizePanel.probSize.GetValue())
        self._view.showPanel("CLOUD") 
        
    def _selectCloud(self, _):
        self._cloud = self._view.cloudPanel.cloudList.getSelectedItems()[0]
        
        self._choices = []
        for roleTemp in self._view.deploymentTemplate.roleTemplates:
            choice = RoleChoice(roleTemp, self._cloud)
            self._choices.append(choice)
            (countCtrl, instTypeCtrl) = self._view.deploymentPanel.addRoleChoice(choice)
            countCtrl.Bind(wx.EVT_SPINCTRL, choice.selectCountEvent)  
            instTypeCtrl.Bind(wx.EVT_COMBOBOX, choice.selectInstanceTypeEvent)
        
        if self._cloud.requiresAWSCred():
            self._view.credPanel.credList.setItems(self._dao.getAWSCreds())
            self._view.showPanel("CREDENTIAL")
        else:
            self._view.showPanel("DEPLOYMENT")
        
    def _selectCred(self, _):
        self._awsCred = self._view.credPanel.credList.getSelectedItems()[0]
        self._view.showPanel("DEPLOYMENT")
    
    def _createDeployment(self, _):
        
        roleReq = {}
        for choice in self._choices:
            roleReq[choice.roleTemp] = (choice.sourceImg, choice.selectedInstanceType, choice.count)
        
        deployment = self._view.deploymentTemplate.createDeployment(self._cloud, roleReq, problemSize=self._size, awsCred=self._awsCred)
        self._dao.save(deployment)
        wx.CallAfter(Publisher().sendMessage, "DEPLOYMENT CREATED", 
                             {'deployment':deployment})
        
        self._view.EndModal(wx.ID_OK)
    
        
