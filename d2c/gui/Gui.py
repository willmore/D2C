'''
Created on Feb 9, 2011

@author: willmore
'''

import wx
from wx.lib.pubsub import Publisher as pub
from .ContainerPanel import ContainerPanel
from .AMIPanel import AMIPanel
from .RawImagePanel import RawImagePanel
import pkg_resources


class Gui(wx.Frame):    
    
    _ID_LISTBOX = 1
    _ID_ADD_IMAGE = 2
    
    #Labels
    LABEL_SOURCE_IMAGES = "Source Images"
    LABEL_AMIS = "AMIs"
    
    ID_ADD_DEPLOYMENT = 1
    ID_CONF = 2
    ID_COMPUTER_CLOUD_CONF = 3
    ID_CLOUD_STORAGE = 4
    ID_CLOUD = 5
    
    def __init__(self, parent=None, id=-1, title='D2C'):
        wx.Frame.__init__(self, parent, id, title, size=(750, 450))

        # Menubar
        menubar = wx.MenuBar()
        file = wx.Menu()
        quit = wx.MenuItem(file, 1, '&Quit\tCtrl+Q')
        file.AppendItem(quit)

        menubar.Append(file, '&File')
        self.SetMenuBar(menubar)

        self.Bind(wx.EVT_MENU, self.OnQuit, id=1)
        
        vbox = wx.BoxSizer(wx.VERTICAL)
        hbox = wx.BoxSizer(wx.HORIZONTAL)
  
        labels = []
        self._containerPanel = ContainerPanel(self)
        
        for (label, panel) in [(self.LABEL_SOURCE_IMAGES, RawImagePanel(self._containerPanel)),
                               (self.LABEL_AMIS, AMIPanel(self._containerPanel))]:
            self._containerPanel.addPanel(label, panel)
            labels.append(label)
        
        self._items = wx.ListBox(self, self._ID_LISTBOX, wx.DefaultPosition, (170, 130), labels, wx.LB_SINGLE)
        self._items.SetSelection(0)
        
        
        #TODO move to controller
        self.Bind(wx.EVT_LISTBOX, self.OnListSelect, id=self._ID_LISTBOX)

        toolbar = self.CreateToolBar()
        
        
        
        toolbar.AddLabelTool(self.ID_CONF, '', wx.Bitmap(pkg_resources.resource_filename(__package__, "icons/Configuration-Settings-icon.png")))
        toolbar.AddLabelTool(self.ID_ADD_DEPLOYMENT, '', wx.Bitmap(pkg_resources.resource_filename(__package__, "icons/network-icon.png")))

        toolbar.AddLabelTool(self.ID_COMPUTER_CLOUD_CONF, '', wx.Bitmap(pkg_resources.resource_filename(__package__, "icons/Configuration-Settings-icon.png")))

        toolbar.AddLabelTool(self.ID_CLOUD_STORAGE, '', wx.Bitmap(pkg_resources.resource_filename(__package__, "icons/cloud-hd-icon.png")))
        toolbar.AddLabelTool(self.ID_CLOUD, '', wx.Bitmap(pkg_resources.resource_filename(__package__, "icons/cloud-hd-icon.png")))

        hbox.Add(self._items, 0, wx.ALL|wx.EXPAND, 5)
        hbox.Add(self._containerPanel, 1, wx.ALL|wx.EXPAND, 5)
        
        vbox.Add(hbox, 1, wx.EXPAND)
        
        self.SetSizer(vbox)
        
        #TODO move to controller
        pub.subscribe(self.__createAMI, "CREATE AMI")
        
    def addPanel(self, label, panel):
        self._items.Append(label)
        self._containerPanel.addPanel(label, panel)
    
    def bindAddDeploymentTool(self, method):
        self.Bind(wx.EVT_TOOL, method, id=self.ID_ADD_DEPLOYMENT)
        
    def bindConfTool(self, method):
        self.Bind(wx.EVT_TOOL, method, id=self.ID_CONF)
        
    def bindCompCloudConfTool(self, method):
        self.Bind(wx.EVT_TOOL, method, id=self.ID_COMPUTER_CLOUD_CONF)
        
    def bindStoreTool(self, method):
        self.Bind(wx.EVT_TOOL, method, id=self.ID_CLOUD_STORAGE)
        
    def bindCloudTool(self, method):
        self.Bind(wx.EVT_TOOL, method, id=self.ID_CLOUD)
        
    #TODO move to controller
    
    
    def __createAMI(self, msg):
        self._containerPanel.showPanel(self.LABEL_AMIS)
        self._items.SetSelection(2)
        
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





 



        
