'''
Created on Mar 10, 2011

@author: willmore
'''

import wx
from .ContainerPanel import ContainerPanel
from .CompCloudConfPanel import RegionList
from .KernelList import KernelList
  
       
class CloudPanel(wx.Panel):    
    
    def __init__(self, *args, **kwargs):
        wx.Panel.__init__(self, *args, **kwargs)
 
        self.regionList = RegionList(self, -1, style=wx.LC_REPORT, size=(-1, 200))
        self.chooseButton = wx.Button(self, wx.ID_ANY, 'Choose Cloud', size=(190, -1))
        self.cancelButton = wx.Button(self, wx.ID_ANY, 'Cancel', size=(190, -1))
        
        self.sizer = wx.BoxSizer(wx.VERTICAL) 
        self.SetSizer(self.sizer)
        
        addStoreTxt = wx.StaticText(self, -1, 'Choose Cloud')
        addStoreTxt.SetFont(wx.Font(15, wx.DEFAULT, wx.DEFAULT, wx.BOLD))
        self.sizer.Add(addStoreTxt, 0)
        
        self.sizer.Add(self.regionList, 0, wx.EXPAND|wx.ALL, 5)
        self.sizer.Add(self.chooseButton, 0, wx.ALIGN_RIGHT|wx.ALL, 2)
        self.sizer.Add(self.cancelButton, 0, wx.ALIGN_RIGHT|wx.ALL, 2)     
        
    def setClouds(self, clouds):
        self.regionList.setRegions(clouds)
        
        
class KernelPanel(wx.Panel):    
    
    def __init__(self, *args, **kwargs):
        wx.Panel.__init__(self, *args, **kwargs)
 
        self.kernelList = KernelList(self, -1, style=wx.LC_REPORT, size=(-1, 200))
        self.chooseButton = wx.Button(self, wx.ID_ANY, 'Select', size=(190, -1))
        self.cancelButton = wx.Button(self, wx.ID_ANY, 'Cancel', size=(190, -1))
        
        self.sizer = wx.BoxSizer(wx.VERTICAL) 
        self.SetSizer(self.sizer)
        
        addStoreTxt = wx.StaticText(self, -1, 'Choose Kernel')
        addStoreTxt.SetFont(wx.Font(15, wx.DEFAULT, wx.DEFAULT, wx.BOLD))
        self.sizer.Add(addStoreTxt, 0)
        
        self.sizer.Add(self.kernelList, 0, wx.EXPAND|wx.ALL, 5)
        self.sizer.Add(self.chooseButton, 0, wx.ALIGN_RIGHT|wx.ALL, 2)
        self.sizer.Add(self.cancelButton, 0, wx.ALIGN_RIGHT|wx.ALL, 2)     
        
    def setStores(self, stores):
        self.storeList.setStores(stores)


class NewAMIWizard(wx.Dialog):
    
    def __init__(self, *args, **kwargs):
        wx.Dialog.__init__(self, *args, **kwargs)
        
        self.vsizer = wx.BoxSizer(wx.VERTICAL)
        self.SetSizer(self.vsizer)
                
        self.container = ContainerPanel(self, size=self.GetSize())
        self.vsizer.Add(self.container, 1, wx.ALL|wx.EXPAND)
        
        self.cloudPanel = CloudPanel(self.container)
        self.container.addPanel("CLOUD", self.cloudPanel)
        
        self.kernelPanel = KernelPanel(self.container)
        self.container.addPanel("KERNEL", self.kernelPanel)
        
    def showPanel(self, label):
        self.container.showPanel(label)
        
        