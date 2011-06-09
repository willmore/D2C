'''
Created on Mar 16, 2011

@author: willmore
'''
import wx
from d2c.gui.DeploymentWizard import DeploymentWizard
from d2c.model.AMI import AMI
from d2c.model.Role import Role
from d2c.model.Deployment import Deployment
from d2c.model.InstanceType import InstanceType
from wx.lib.pubsub import Publisher

class DeploymentWizardController:
    
    def __init__(self, deploymentWizard, dao):
        
        self.dao = dao
        self.wizard = deploymentWizard
        
        self.wizard.container.getPanel("ROLES").getPanel("ROLES").addRoleButton.Bind(wx.EVT_BUTTON, self.showAddRolePanel)
        self.wizard.container.getPanel("ROLES").getPanel("ADD_ROLE").amiList.setAMIs(dao.getAMIs())
        self.wizard.container.getPanel("ROLES").getPanel("ADD_ROLE").addRoleButton.Bind(wx.EVT_BUTTON, self.addRole)
        self.wizard.container.getPanel("ROLES").getPanel("ROLES").finishButton.Bind(wx.EVT_BUTTON, self.addDeployment)
        
        self.wizard.container.getPanel("COMPLETION").okButton.Bind(wx.EVT_BUTTON, self.done)
        
        self.wizard.namePanel.nextButton.Bind(wx.EVT_BUTTON, self.showRolesWizard)
        
        self.wizard.container.showPanel("NAME")   
    
    def showRolesWizard(self, event):
        #TODO validation
        self.newName = self.wizard.namePanel.name.GetValue()      
        self.wizard.container.showPanel("ROLES")

    def done(self, event):
        self.wizard.EndModal(wx.ID_OK)
    
    def addDeployment(self, event):
        
        roles = self.wizard.roleWizard.getPanel("ROLES").roleList.getRoles()
                
        #TODO
        startActions = ()
        
        #TODO
        dataCollections = ()
        
        deployment = Deployment(self.newName, roles=roles)
    
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
        
        role = Role(self.newName, roleName, amis[0], hostCount, instanceType)
        
        self.wizard.container.getPanel("ROLES").getPanel("ROLES").roleList.addRole(role)
        
        self.wizard.container.getPanel("ROLES").showPanel("ROLES")
        
    def showDeplomentSettingsPanel(self, event):
        
        if len(self.wizard.roleWizard.getPanel("ROLES").roleList.getRoles()) <= 0:
            dial = wx.MessageDialog(None, 'Must specify at least one role', 'Exclamation', wx.OK | 
                                    wx.ICON_EXCLAMATION)
            dial.ShowModal()
            return
        
        self.wizard.container.showPanel("CONF")
        
    def showAddRolePanel(self, event):
        self.wizard.container.getPanel("ROLES").showPanel("ADD_ROLE")
        
if __name__ == '__main__':

    app = wx.PySimpleApp()  # Start the application

    # Create wizard and add any kind pages you'd like
    mywiz = DeploymentWizard(None, -1, 'Simple Wizard', size=(400,300))
    
    class DummyDao:
        def getAMIs(self):
            return (AMI("blah", "xyz"),)
        
        def saveDeployment(self, d):
            pass
    
    controller = DeploymentWizardController(mywiz, DummyDao())
    # Cleanup
    mywiz.ShowModal()
    mywiz.Destroy()
    app.MainLoop()