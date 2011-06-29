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
from d2c.model.SSHCred import SSHCred
from d2c.model.UploadAction import UploadAction
from .util import createEmptyChecker

def fieldsNotEmpty(self, *fields):
    for f in fields:
        if len(f.GetValue()) == 0:
            return False         
    return True

class DeploymentWizardController:
    
    def __init__(self, deploymentWizard, dao):
        
        self.dao = dao
        self.wizard = deploymentWizard
        
        createEmptyChecker(self.wizard.namePanel.nextButton,
                          self.wizard.namePanel.name)
        
        self.wizard.container.getPanel("ROLES").getPanel("ROLES").addRoleButton.Bind(wx.EVT_BUTTON, self.showAddRolePanel)
        
        self.wizard.container.getPanel("ROLES").getPanel("ADD_ROLE").addRoleButton.Bind(wx.EVT_BUTTON, self.addRole)
        self.wizard.container.getPanel("ROLES").getPanel("ROLES").finishButton.Bind(wx.EVT_BUTTON, self.addDeployment)
        
        p = self.wizard.container.getPanel("ROLES").getPanel("ADD_ROLE")
        createEmptyChecker(p.addRoleButton, p.roleName, p.instanceType, p.amiList)
        
        self.wizard.container.getPanel("COMPLETION").okButton.Bind(wx.EVT_BUTTON, self.done)
        
        self.wizard.namePanel.nextButton.Bind(wx.EVT_BUTTON, self.showClouds)
        self.wizard.cloudPanel.backButton.Bind(wx.EVT_BUTTON,self.showName)
        
        for c in self.dao.getClouds():
            self.wizard.cloudPanel.clouds.Append(c.name)
            
        self.wizard.cloudPanel.clouds.Bind(wx.EVT_COMBOBOX,self.checkClouds)
            
        self.wizard.cloudPanel.nextButton.Bind(wx.EVT_BUTTON, self.showRolesWizard)
        
        self.uploadScripts = []
        self.startScripts = []
        self.endScripts = []
        self.dataCollectors = []
           
        self.setupFlexList(p.sw, p.sw.uploadScriptBox.boxSizer,self.uploadScripts, 2, ("Source", "Destination"))   
        self.setupFlexList(p.sw, p.sw.startScriptBox.boxSizer,self.startScripts)
        self.setupFlexList(p.sw, p.sw.endScriptBox.boxSizer, self.endScripts)
        self.setupFlexList(p.sw, p.sw.dataBox.boxSizer, self.dataCollectors)
        
        self.wizard.container.showPanel("NAME")
        self.wizard.container.getPanel("NAME").name.Bind(wx.EVT_TEXT, self.checkNameDeployment)
        
        self.wizard.container.getPanel("ROLES").getPanel("ROLES").backButton.Bind(wx.EVT_BUTTON,self.showClouds)
        self.wizard.container.getPanel("ROLES").getPanel("ADD_ROLE").cancelButton.Bind(wx.EVT_BUTTON,self.showRoles)
    
    def checkClouds(self,event):
        if(event.GetSelection()+1):
            self.wizard.cloudPanel.nextButton.Enable()
        else:
            self.wizard.cloudPanel.nextButton.Disable()
    
    def setupFlexList(self, parent, boxsizer, textControls, fieldCount=1, labels=()):
        
        def handleRemove(evt): 
            for c in evt.GetEventObject().GetParent().GetChildren():
                if c in textControls:
                    textControls.remove(c)

            sizer = evt.GetEventObject().GetContainingSizer()
            sizer.Clear(deleteWindows=True)
            
            #Im not sure how to get rid of the sizer itself,
            #as calling sizer.Destroy() stops the program...
            #sizer.Destroy()
            
            self.wizard.Layout()
        
        def handleAdd(evt):
            if (evt is not None):
                evtBtn = evt.GetEventObject()
                evtBtn.SetLabel("Remove")
                evtBtn.Bind(wx.EVT_BUTTON, handleRemove)
            sizer = wx.BoxSizer(wx.HORIZONTAL)
            
            fields = []
            for i in range(fieldCount): 
                
                if len(labels) > i:
                    label = wx.StaticText(parent, -1, label=labels[i])
                    sizer.Add(label,0)
                
                field = wx.TextCtrl(parent, -1)
                sizer.Add(field, 1)
                fields.append(field)
                textControls.append(field)
                
                
            btn = wx.Button(parent, -1, "Add")
            btn.Bind(wx.EVT_BUTTON, handleAdd)
            sizer.Add(btn, 0)
            
            boxsizer.Add(sizer, 0, wx.EXPAND)
            
            createEmptyChecker(btn, *fields)
            self.wizard.Layout()
            
        handleAdd(None)
    
    def showRolesWizard(self, _):
        self.cloudName = self.wizard.cloudPanel.clouds.GetValue()      
        self.cloud = self.dao.getCloud(self.cloudName)
        
        instanceCombo = self.wizard.container.getPanel("ROLES").getPanel("ADD_ROLE").instanceType
        for i in self.cloud.instanceTypes:
            instanceCombo.Append(i.name)
        
        self.wizard.container.getPanel("ROLES").getPanel("ADD_ROLE").amiList.setAMIs(self.dao.getAMIs())
        
        self.wizard.container.showPanel("ROLES")
        
    def showName(self,_):
        self.newName = self.wizard.namePanel.name.GetValue()      
        self.wizard.container.showPanel("NAME")
        
    def showClouds(self, _):
        #TODO validation
        self.newName = self.wizard.namePanel.name.GetValue()      
        self.wizard.container.showPanel("CLOUD")
        
    def showRoles(self,_):
        self.newName = self.wizard.namePanel.name.GetValue()      
        self.wizard.container.getPanel("ROLES").showPanel("ROLES")

    def done(self, event):
        self.wizard.EndModal(wx.ID_OK)
    
    def addDeployment(self, event):
        
        roles = self.wizard.roleWizard.getPanel("ROLES").roleList.getItems()
        
        deployment = Deployment(self.newName, roles=roles, awsCred=self.dao.getAWSCred("mainKey"))
        deployment.setCloud(self.cloud)
    
        self.dao.save(deployment)
        
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
        
        tmpCred = SSHCred("foobar", "foobar")
    
        uploadActions = []
        for i in range(0, len(self.uploadScripts), 2):
            uploadActions.append(UploadAction(self.uploadScripts[i].GetValue(), 
                                             self.uploadScripts[i+1].GetValue(),
                                             tmpCred))        
        
        startActions = [Action(s.GetValue(), tmpCred) for s in self.startScripts] 
        finishedChecks = [FileExistsFinishedCheck(f.GetValue(), tmpCred) for f in self.endScripts]
        dataCollectors = [DataCollector(d.GetValue(), tmpCred) for d in self.dataCollectors]
        
        role = Role(roleName, amis[0], hostCount, instanceType,
                    startActions=startActions,
                    uploadActions=uploadActions,
                    finishedChecks=finishedChecks,
                    dataCollectors=dataCollectors )
        # Zero-out holders
        self.uploadScripts = []
        self.startScripts = []
        self.endScripts = []
        self.dataCollectors = []
        
        self.wizard.container.getPanel("ROLES").getPanel("ROLES").roleList.addItem(role)
        
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
        self.wizard.SetMinSize((600,600))
        self.wizard.Fit()
        

        
    def checkNameDeployment(self, _):
        panel = self.wizard.container.getPanel("NAME")
        
        if fieldsNotEmpty(panel.name):
            panel.nextButton.Enable()
        else:
            panel.nextButton.Disable()
