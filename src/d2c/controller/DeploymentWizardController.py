'''
Created on Mar 16, 2011

@author: willmore
'''
import wx
from d2c.gui.DeploymentWizard import DeploymentWizard
from d2c.model.AMI import AMI
from d2c.model.Deployment import Role
from d2c.model.Deployment import Deployment

class DeploymentWizardController:
    
    def __init__(self, deploymentWizard, dao):
        
        self.dao = dao
        self.wizard = deploymentWizard
        
        self.wizard.container.getPanel("ROLES").getPanel("ROLES").nextButton.Bind(wx.EVT_BUTTON, self.showDeplomentSettingsPanel)
        self.wizard.container.getPanel("ROLES").getPanel("ROLES").addRoleButton.Bind(wx.EVT_BUTTON, self.showAddRolePanel)
        
        self.wizard.container.getPanel("ROLES").getPanel("ADD_ROLE").amiList.setAMIs(dao.getAMIs())
        self.wizard.container.getPanel("ROLES").getPanel("ADD_ROLE").addRoleButton.Bind(wx.EVT_BUTTON, self.addRole)
        self.wizard.container.getPanel("CONF").finishButton.Bind(wx.EVT_BUTTON, self.addDeployment)
        
        self.wizard.container.getPanel("COMPLETION").okButton.Bind(wx.EVT_BUTTON, self.done)
        
        self.wizard.container.showPanel("ROLES")   
    
    
    def done(self, event):
        print "Done"
        self.wizard.EndModal(wx.ID_OK)
    
    def addDeployment(self, event):
        print "Saving Deployment"
        roles = self.wizard.container.getPanel("ROLES").getPanel("ROLES").roleList.getRoles()
        
        #TODO
        startActions = ()
        
        #TODO
        dataCollections = ()
        
        deployment = Deployment(Deployment.NEW_ID, roles, 
                                startActions, dataCollections)
    
        self.dao.saveDeployment(deployment)
        
        self.wizard.container.showPanel("COMPLETION")
    
    def addRole(self, event):
        p = self.wizard.container.getPanel("ROLES").getPanel("ADD_ROLE")
        roleName = p.roleName.GetValue()
        hostCount = int(p.hostCount.GetValue())
        amis = p.amiList.getSelectedAMIs()
        
        role = Role(roleName, amis[0], hostCount)
        
        self.wizard.container.getPanel("ROLES").getPanel("ROLES").roleList.addRole(role)
        
        self.wizard.container.getPanel("ROLES").showPanel("ROLES")
        
    def showDeplomentSettingsPanel(self, event):
        self.wizard.container.showPanel("CONF")
        
    def showAddRolePanel(self, event):
        self.wizard.container.getPanel("ROLES").showPanel("ADD_ROLE")
        
if __name__ == '__main__':

    app = wx.PySimpleApp()  # Start the application

    # Create wizard and add any kind pages you'd like
    mywiz = DeploymentWizard(None, -1, 'Simple Wizard')
    
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