import wx

from .ContainerPanel import ContainerPanel
from .RoleList import RoleTemplateList
from .DeploymentPanel import DeploymentPanel
from d2c.controller.DeploymentController import DeploymentController
from d2c.controller.DeploymentCreatorController import DeploymentCreatorController

from .DeploymentCreator import DeploymentCreator

class DeploymentTab(wx.Panel):
    
    def __init__(self, dao, *args, **kwargs):

        wx.Panel.__init__(self, *args, **kwargs)
        
        self.dao = dao
        
        self.splitter = wx.SplitterWindow(self, -1)
        self.splitter.SetMinimumPaneSize(200)
        vbox = wx.BoxSizer(wx.VERTICAL)

        self.tree = wx.TreeCtrl(self.splitter, -1, wx.DefaultPosition, 
                                (-1,-1), wx.TR_HIDE_ROOT|wx.TR_HAS_BUTTONS)
        self.treeRoot = self.tree.AddRoot('root')
        vbox.Add(self.splitter, 1, wx.EXPAND)
        
        self.displayPanel = ContainerPanel(self.splitter, -1)
   
        self.splitter.SplitVertically(self.tree, self.displayPanel)
        self.SetSizer(vbox)
        self.Layout()   
        
    def addDeploymentTemplatePanel(self, deploymentPanel):
        
        node = self.tree.AppendItem(self.treeRoot, deploymentPanel.deployment.name)
        self.displayPanel.addPanel(deploymentPanel.deployment.name, deploymentPanel)
        
        for d in deploymentPanel.deployment.deployments:
            self.addDeployment(d, node)
            
    def addDeployment(self, deployment, parentNode=None):
    
    
        if parentNode is None:
            #Find parent node based on deployment name
            parentNode = self._getTreeItemIdByName(deployment.deploymentTemplate.name)
    
        label = deployment.deploymentTemplate.name  + ":" + str(deployment.id)
        self.tree.AppendItem(parentNode, label)
        p = DeploymentPanel(deployment, self.displayPanel, -1)
        self.displayPanel.addPanel(label, p)
        DeploymentController(p, self.dao)
        
    def _getTreeItemIdByName(self, name):
        '''
        Only iterates through root children
        '''
        item, cookie = self.tree.GetFirstChild(self.treeRoot)
        while item:
            
            if name == self.tree.GetItemText(item):
                return item
            
            item, cookie = self.tree.GetNextChild(self.treeRoot, cookie)
        
        return None
        
class DeploymentTemplatePanel(wx.Panel):    
    
    def __init__(self, deployment, dao, *args, **kwargs):
        
        wx.Panel.__init__(self, *args, **kwargs)
        self.dao = dao
        
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
        
        self.deployButton.Bind(wx.EVT_BUTTON, self.showDeploymentCreation)
        self.GetSizer().Add(self.deployButton, 0, wx.ALL, 2)
        
    def showDeploymentCreation(self, _):
        c = DeploymentCreator(self, self.deployment)
        DeploymentCreatorController(c, self.dao)
        c.ShowModal()
        