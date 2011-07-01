import wx

from .ContainerPanel import ContainerPanel
from .RoleList import RoleTemplateList
from .DeploymentPanel import DeploymentPanel

class DeploymentTab(wx.Panel):
    
    def __init__(self, *args, **kwargs):

        wx.Panel.__init__(self, *args, **kwargs)
        self.splitter = wx.SplitterWindow(self, -1)
        self.splitter.SetMinimumPaneSize(150)
        vbox = wx.BoxSizer(wx.VERTICAL)

        self.tree = wx.TreeCtrl(self.splitter, -1, wx.DefaultPosition, 
                                (-1,-1), wx.TR_HIDE_ROOT|wx.TR_HAS_BUTTONS)
        self.treeRoot = self.tree.AddRoot('root')
        vbox.Add(self.splitter, 1, wx.EXPAND)
        
        self.displayPanel = ContainerPanel(self.splitter, -1)
   
        self.splitter.SplitVertically(self.tree, self.displayPanel)
        self.SetSizer(vbox)
        self.Layout()   
        
    def addDeploymentPanel(self, deploymentPanel):
        
        node = self.tree.AppendItem(self.treeRoot, deploymentPanel.deployment.name)
        self.displayPanel.addPanel(deploymentPanel.deployment.name, deploymentPanel)
        
        for d in deploymentPanel.deployment.deployments:
            label = deploymentPanel.deployment.name  + ":" + str(d.id)
            self.tree.AppendItem(node, label)
            self.displayPanel.addPanel(label, DeploymentPanel(d, self.displayPanel, -1))
        
        
class DeploymentTemplatePanel(wx.Panel):    
    
    def __init__(self, deployment, *args, **kwargs):
        wx.Panel.__init__(self, *args, **kwargs)
        
        self.SetSizer(wx.BoxSizer(wx.VERTICAL))
        
        self.deployment = deployment
        
        label = wx.StaticText(self, -1, deployment.name)
        label.SetFont(wx.Font(20, wx.DEFAULT, wx.DEFAULT, wx.BOLD))
        self.GetSizer().Add(label, 0, wx.BOTTOM, 10)
          
        label = wx.StaticText(self, -1, 'Roles')
        label.SetFont(wx.Font(wx.DEFAULT, wx.DEFAULT, wx.DEFAULT, wx.BOLD))
        self.GetSizer().Add(label, 0, wx.BOTTOM, 5)
        self.roles = RoleTemplateList(self, -1, items=deployment.roleTemplates)
        self.GetSizer().Add(self.roles, 0, wx.BOTTOM | wx.EXPAND, 5)
        
        self.deployButton = wx.Button(self, wx.ID_ANY, 'Create Deployment')
        self.GetSizer().Add(self.deployButton, 0, wx.ALL, 2)
        