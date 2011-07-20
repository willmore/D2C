
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
        self._view.showPanel("CLOUD")
        self._view.cloudPanel.cloudList.setItems(dao.getClouds())
        
        self._view.cloudPanel.Bind(wx.EVT_BUTTON, self._selectCloud)
        
        self._view.deploymentPanel.doneButton.Bind(wx.EVT_BUTTON, self._createDeployment)

        
    def _selectCloud(self, _):
        self._cloud = self._view.cloudPanel.cloudList.getSelectedItems()[0]
        
        self.choices = []
        for roleTemp in self._view.deploymentTemplate.roleTemplates:
            choice = RoleChoice(roleTemp, self._cloud)
            self.choices.append(choice)
            (countCtrl, instTypeCtrl) = self._view.deploymentPanel.addRoleChoice(choice)
            countCtrl.Bind(wx.EVT_SPINCTRL, choice.selectCountEvent)  
            instTypeCtrl.Bind(wx.EVT_COMBOBOX, choice.selectInstanceTypeEvent)
        
        self._view.showPanel("DEPLOYMENT")
        
    def _createDeployment(self, _):
        
        roleReq = {}
        for choice in self.choices:
            roleReq[choice.roleTemp] = (choice.sourceImg, choice.selectedInstanceType, choice.count)
        
        deployment = self._view.deploymentTemplate.createDeployment(self._cloud, roleReq)
        self._dao.save(deployment)
        wx.CallAfter(Publisher().sendMessage, "DEPLOYMENT CREATED", 
                             {'deployment':deployment})
        
        self._view.EndModal(wx.ID_OK)
    
        