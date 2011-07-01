'''
Created on Feb 9, 2011

@author: willmore
'''

import wx
from wx.lib.pubsub import Publisher as pub
from .ContainerPanel import ContainerPanel
from .AMIPanel import AMIPanel
import pkg_resources
from .RawImagePanel import RawImagePanel
from .DeploymentTab import DeploymentTab
from .ImageTab import ImageTab

class MainTabContainer(wx.Panel):
    
    def __init__(self, *args, **kwargs):
        wx.Panel.__init__(self, *args, **kwargs)
        
class Gui(wx.Frame):    
    
    _ID_LISTBOX = 1
    _ID_ADD_IMAGE = 2
    
    #Labels
    LABEL_SOURCE_IMAGES = "Source Images"
    LABEL_AMIS = "AMIs"
    
    ID_ADD_DEPLOYMENT = 3
    ID_CONF = 2
    ID_CLOUD = 5
    
    def __init__(self, dao, parent=None, id=-1, title='D2C'):
        wx.Frame.__init__(self, parent, id, title, size=(750, 450))

        self.Center()

        self.__initMenuBar()

        toolbar = self.CreateToolBar()
        
        toolbar.AddLabelTool(self.ID_CONF, '', wx.Bitmap(pkg_resources.resource_filename(__package__, "icons/keys-icon.png")))
        toolbar.AddLabelTool(self.ID_CLOUD, '', wx.Bitmap(pkg_resources.resource_filename(__package__, "icons/cloud-hd-icon.png")))
        toolbar.AddLabelTool(self.ID_ADD_DEPLOYMENT, '', wx.Bitmap(pkg_resources.resource_filename(__package__, "icons/network-icon.png")))
     
        self.tabContainer = wx.Notebook(self, -1, style=wx.NB_TOP)
        
        self.imageTab = ImageTab(dao, self.tabContainer, -1)
        self.tabContainer.AddPage(self.imageTab, "Images")
        
        self.imagePanel = RawImagePanel(self.tabContainer, -1)
        self.tabContainer.AddPage(self.imagePanel, "Source Images")
        
        self.amiPanel = AMIPanel(self.tabContainer, -1)
        self.tabContainer.AddPage(self.amiPanel, "AMIs")
        
        self.deploymentPanel = DeploymentTab(self.tabContainer, -1)
        self.tabContainer.AddPage(self.deploymentPanel, "Deployments")
        
        #TODO move to controller
        pub.subscribe(self.__createAMI, "CREATE AMI")
        
    def __initMenuBar(self):
        
        menubar = wx.MenuBar()
        file = wx.Menu()
        quit = wx.MenuItem(file, 1, '&Quit\tCtrl+Q')
        file.AppendItem(quit)

        menubar.Append(file, '&File')
        self.SetMenuBar(menubar)

        self.Bind(wx.EVT_MENU, self.OnQuit, id=1)
        
    def addPanel(self, label, panel):
        self._items.Append(label)
        self._containerPanel.addPanel(label, panel)
    
    def bindAddDeploymentTool(self, method):
        self.Bind(wx.EVT_TOOL, method, id=self.ID_ADD_DEPLOYMENT)
        
    def bindConfTool(self, method):
        self.Bind(wx.EVT_TOOL, method, id=self.ID_CONF)
        
    def bindCloudTool(self, method):
        self.Bind(wx.EVT_TOOL, method, id=self.ID_CLOUD)
        
    #TODO move to controller
    def __createAMI(self, _):
        self.tabContainer.ChangeSelection(2)
        
    def setSelection(self, label):
        self._items.SetStringSelection(label)
        self._containerPanel.showPanel(self._items.GetStringSelection())
    
    def getImagePanel(self):
        return self._containerPanel.getPanel(self.LABEL_SOURCE_IMAGES)
    
    def getAMIPanel(self):
        return self._containerPanel.getPanel(self.LABEL_AMIS)
    
    def OnQuit(self, event):
        self.Close()

    def OnListSelect(self, event):
        self._containerPanel.showPanel(self._items.GetStringSelection())





 



        
