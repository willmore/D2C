import wx

from .ContainerPanel import ContainerPanel
from .ItemList import ColumnMapper, ItemList

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
        self.tree.AppendItem(self.treeRoot, deploymentPanel.deployment.id)
        self.displayPanel.addPanel(deploymentPanel.deployment.id, deploymentPanel)
        