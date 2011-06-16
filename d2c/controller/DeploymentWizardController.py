'''
Created on Mar 16, 2011

@author: willmore
'''
import wx
from d2c.model.Role import Role
from d2c.model.Deployment import Deployment
from d2c.model.InstanceType import InstanceType
from wx.lib.pubsub import Publisher
from d2c.model.Action import Action
from d2c.model.FileExistsFinishedCheck import FileExistsFinishedCheck
from d2c.model.DataCollector import DataCollector
from .util import createEmptyChecker

class DeploymentWizardController:
    
    def __init__(self, deploymentWizard, dao):
        
        self.dao = dao
        self.wizard = deploymentWizard
        
        self.wizard.container.getPanel("ROLES").getPanel("ROLES").addRoleButton.Bind(wx.EVT_BUTTON, self.showAddRolePanel)
        
        self.wizard.container.getPanel("ROLES").getPanel("ADD_ROLE").addRoleButton.Bind(wx.EVT_BUTTON, self.addRole)
        self.wizard.container.getPanel("ROLES").getPanel("ROLES").finishButton.Bind(wx.EVT_BUTTON, self.addDeployment)
        
        p = self.wizard.container.getPanel("ROLES").getPanel("ADD_ROLE")
        createEmptyChecker(p.addRoleButton, p.roleName, p.instanceType, p.amiList)
        
        self.wizard.container.getPanel("COMPLETION").okButton.Bind(wx.EVT_BUTTON, self.done)
        
        self.wizard.namePanel.nextButton.Bind(wx.EVT_BUTTON, self.showClouds)
        
        for c in self.dao.getClouds():
            self.wizard.cloudPanel.clouds.Append(c.name)
        self.wizard.cloudPanel.nextButton.Bind(wx.EVT_BUTTON, self.showRolesWizard)
        
        
        self.startScripts = []
        self.endScripts = []
        self.dataCollectors = []
        
        
        self.setupFlexList(p.sw, p.sw.startScriptBox.boxSizer,self.startScripts)
        self.setupFlexList(p.sw, p.sw.endScriptBox.boxSizer, self.endScripts)
        self.setupFlexList(p.sw, p.sw.dataBox.boxSizer, self.dataCollectors)
        
        self.wizard.container.showPanel("NAME")  
    
    def setupFlexList(self, parent, boxsizer, textControls):
        
        def handleRemove(evt): 
            
            textControls.remove(evt.GetEventObject().assocTxt)
            sizer = evt.GetEventObject().GetContainingSizer()
            sizer.Clear(deleteWindows=True)
            
            #Im not sure how to get rid of the sizer itself,
            #as calling sizer.Destroy() stops the program...
            #sizer.Destroy()
            
            self.wizard.Layout()
        
        def handleAdd(evt):
            evtBtn = evt.GetEventObject()
            evtBtn.SetLabel("Remove")
            evtBtn.Bind(wx.EVT_BUTTON, handleRemove)
            sizer = wx.BoxSizer(wx.HORIZONTAL)
            btn = wx.Button(parent, -1, "Add")
            btn.Bind(wx.EVT_BUTTON, handleAdd)
             
            field = wx.TextCtrl(parent, -1)
            btn.assocTxt = field
            textControls.append(field)
            
            sizer.Add(field, 1)
            sizer.Add(btn, 0)
            boxsizer.Add(sizer, 0, wx.EXPAND)
            self.wizard.Layout()
        
        sizer = wx.BoxSizer(wx.HORIZONTAL)
        btn = wx.Button(parent, -1, "Add")
        btn.Bind(wx.EVT_BUTTON, handleAdd)
             
        field = wx.TextCtrl(parent, -1)
        btn.assocTxt = field
        textControls.append(field)
        sizer.Add(field, 1)
        sizer.Add(btn, 0)
        boxsizer.Add(sizer, 0, wx.EXPAND)
    
    def showRolesWizard(self, _):
        self.cloudName = self.wizard.cloudPanel.clouds.GetValue()      
        self.cloud = self.dao.getCloud(self.cloudName)
        
        instanceCombo = self.wizard.container.getPanel("ROLES").getPanel("ADD_ROLE").instanceType
        for i in self.cloud.instanceTypes:
            instanceCombo.Append(i.name)
        
        self.wizard.container.getPanel("ROLES").getPanel("ADD_ROLE").amiList.setAMIs(self.dao.getAMIs())
        
        self.wizard.container.showPanel("ROLES")
        
    def showClouds(self, _):
        #TODO validation
        self.newName = self.wizard.namePanel.name.GetValue()      
        self.wizard.container.showPanel("CLOUD")

    def done(self, event):
        self.wizard.EndModal(wx.ID_OK)
    
    def addDeployment(self, event):
        
        roles = self.wizard.roleWizard.getPanel("ROLES").roleList.getRoles()
        
        deployment = Deployment(self.newName, roles=roles, awsCred=self.dao.getAWSCred("mainKey"))
        deployment.setCloud(self.cloud)
    
        self.dao.saveDeployment(deployment)
        
        wx.CallAfter(Publisher().sendMessage, "DEPLOYMENT CREATED", 
                             {'deployment':deployment})
        
        self.wizard.container.showPanel("COMPLETION")
    
    def addRole(self, event):
        p = self.wizard.container.getPanel("ROLES").getPanel("ADD_ROLE")
        roleName = p.roleName.GetValue()
        hostCount = int(p.hostCount.GetValue())
        amis = p.amiList.getSelectedAMIs()
        instanceType = InstanceType.TYPES[p.instanceType.GetValue()]
        
        assert len(amis) == 1, "Only one AMI at a time supported"
        
        
        self.startScripts = []
        self.endScripts = []
        self.dataCollectors = []
        
        startActions = [Action(s, None) for s in self.startScripts]
        
        finishedChecks = [FileExistsFinishedCheck(f, None) for f in self.endScripts]
        dataCollectors = [DataCollector(d) for d in self.dataCollectors]
        
        role = Role(roleName, amis[0], hostCount, instanceType,startActions=startActions,
                    finishedChecks=finishedChecks,
                    dataCollectors=dataCollectors )
        
        self.wizard.container.getPanel("ROLES").getPanel("ROLES").roleList.addRole(role)
        
        self.wizard.container.getPanel("ROLES").showPanel("ROLES")
        
        self.wizard.SetMinSize(self.prevSize)
        self.wizard.Fit()
        
    def showDeplomentSettingsPanel(self, event):
        
        if len(self.wizard.roleWizard.getPanel("ROLES").roleList.getRoles()) <= 0:
            dial = wx.MessageDialog(None, 'Must specify at least one role', 'Exclamation', wx.OK | 
                                    wx.ICON_EXCLAMATION)
            dial.ShowModal()
            return
        
        self.wizard.container.showPanel("CONF")
        
    def showAddRolePanel(self, event):
        
        self.wizard.container.getPanel("ROLES").showPanel("ADD_ROLE")
        self.prevSize = self.wizard.GetSize()
        self.wizard.SetMinSize((500,500))
        self.wizard.Fit()
