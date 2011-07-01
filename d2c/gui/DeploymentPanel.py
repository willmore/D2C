
import wx
       
from .RoleList import RoleList
from d2c.model.Deployment import DeploymentState

class DeploymentPanel(wx.Panel):    
    
    def __init__(self, deployment, *args, **kwargs):
        wx.Panel.__init__(self, *args, **kwargs)
        
        self.SetSizer(wx.BoxSizer(wx.VERTICAL))
        
        self.deployment = deployment
        
        label = wx.StaticText(self, -1, deployment.deploymentTemplate.name + ":" + str(deployment.id))
        label.SetFont(wx.Font(20, wx.DEFAULT, wx.DEFAULT, wx.BOLD))
        self.GetSizer().Add(label, 0, wx.BOTTOM, 10)
        
        self.statusSizer = wx.BoxSizer(wx.HORIZONTAL)
        self.GetSizer().Add(self.statusSizer, 0, wx.BOTTOM, 5)
        label = wx.StaticText(self, -1, 'Status')
        label.SetFont(wx.Font(wx.DEFAULT, wx.DEFAULT, wx.DEFAULT, wx.BOLD))
        self.statusSizer.Add(label, 0, wx.RIGHT, 5)
        self.statusField = wx.StaticText(self, -1, deployment.state)
        self.statusSizer.Add(self.statusField, 0, wx.ALIGN_CENTER)
             
        label = wx.StaticText(self, -1, 'Roles')
        label.SetFont(wx.Font(wx.DEFAULT, wx.DEFAULT, wx.DEFAULT, wx.BOLD))
        self.GetSizer().Add(label, 0, wx.BOTTOM, 5)
        self.roles = RoleList(self, -1, items=deployment.roles)
        self.GetSizer().Add(self.roles, 0, wx.BOTTOM | wx.EXPAND, 5)
        
        self.deployButton = wx.Button(self, wx.ID_ANY, 'Deploy', size=(110, -1))
        self.GetSizer().Add(self.deployButton, 0, wx.ALL, 2)
        
        self.cancelButton = wx.Button(self, wx.ID_ANY, 'Cancel', size=(110, -1))
        self.GetSizer().Add(self.cancelButton, 0, wx.ALL, 2)
        
        self.cancelButton.Hide()
           
        self.logPanel = wx.TextCtrl(self, -1, size=(100,100), style=wx.TE_MULTILINE )
        self.logPanel.Hide() # logPanel will be shown later on demand
        self.GetSizer().Add(self.logPanel, 1, wx.BOTTOM | wx.EXPAND, 5)
    
        
    def showLogPanel(self):
        self.logPanel.Show()
        self.Layout()
        
    def appendLogPanelText(self, text):
        self.logPanel.AppendText(str(text) + "\n")
    
    def update(self):
        '''
        Update the panel to reflect the current Deployment
        '''
        self.statusField.SetLabel(self.deployment.state)
        
        if self.deployment.state == DeploymentState.COMPLETED:
            self.cancelButton.Hide()