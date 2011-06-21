'''
Created on Mar 10, 2011

@author: willmore
'''

import wx
from .ContainerPanel import ContainerPanel
from .ItemList import ItemList, ColumnMapper
from wx._controls import StaticText
       
class CloudListPanel(wx.Panel):    
    
    def __init__(self, *args, **kwargs):
        wx.Panel.__init__(self, *args, **kwargs)
 
        self.cloudList = ItemList(self, -1, style=wx.LC_REPORT, size=(-1, 330),
                                   mappers=[ColumnMapper('Cloud Name', lambda c: c.name)])
        self.addButton = wx.Button(self, wx.ID_ANY, 'Add New Cloud')
        self.doneButton = wx.Button(self, wx.ID_ANY, 'Close')
        
        self.sizer = wx.BoxSizer(wx.VERTICAL)
        self.hsizer = wx.BoxSizer(wx.HORIZONTAL) 
        self.SetSizer(self.sizer)
        
        self.sizer.Add(self.cloudList, 0, wx.EXPAND|wx.ALL, 5)
        self.hsizer.Add(self.addButton, 0, wx.ALIGN_RIGHT|wx.ALL, 5)
        self.hsizer.Add(self.doneButton, 0, wx.ALIGN_RIGHT|wx.ALL, 5)
        
        self.sizer.Add(self.hsizer,0,wx.ALIGN_RIGHT|wx.ALL,5)
        
class NewCloudPanel(wx.Panel):    
    
    def __init__(self, *args, **kwargs):
        wx.Panel.__init__(self, *args, **kwargs)
        
        self.sizer = wx.FlexGridSizer(7,2,0,0)
        self.sizer.AddGrowableCol(1, 1)
        self.SetSizer(self.sizer)
        
        addRoleTxt = wx.StaticText(self, -1, 'Configure Cloud')
        addRoleTxt.SetFont(wx.Font(10, wx.DEFAULT, wx.DEFAULT, wx.BOLD))
        self.sizer.Add(addRoleTxt, 0,wx.ALL,5)
        self.sizer.Add(wx.StaticText(self), wx.EXPAND)  
               
        self.sizer.Add(wx.StaticText(self, -1, 'Name'), 0,  wx.ALIGN_RIGHT|wx.ALL, 2)
        self.name = wx.TextCtrl(self) 
        self.sizer.Add(self.name, 0,  wx.ALIGN_CENTER_VERTICAL|wx.EXPAND|wx.ALL, 2) 
        
        self.sizer.Add(wx.StaticText(self, -1, 'Service URL'), 0, wx.ALIGN_RIGHT|wx.ALL, 2)
        self.serviceURL = wx.TextCtrl(self)
        self.sizer.Add(self.serviceURL, 0, wx.ALIGN_CENTER_VERTICAL|wx.EXPAND|wx.ALL, 2)
          
        self.sizer.Add(wx.StaticText(self, -1, 'Storage URL'), 0, wx.ALIGN_RIGHT|wx.ALL, 2)
        self.storageURL = wx.TextCtrl(self)
        self.sizer.Add(self.storageURL, 0, wx.ALIGN_CENTER_VERTICAL|wx.EXPAND|wx.ALL, 2)
        
       
        
        
        self.sizer.Add(wx.StaticText(self, -1, 'EC2 Certificate'), 0, wx.ALIGN_RIGHT|wx.ALL, 2)
        self.ec2Cert = wx.TextCtrl(self)
        self.sizer.Add(self.ec2Cert, 0, wx.ALIGN_CENTER_VERTICAL|wx.EXPAND|wx.ALL, 2)
        
        
        self.kernelList = ItemList(self, -1, style=wx.LC_REPORT, size=(-1, 100),
                                     mappers=[ColumnMapper('AKI', lambda k: k.aki),
                                              ColumnMapper('Architecture', lambda k: k.arch)])
        
        self.addKernelButton = wx.Button(self, wx.ID_ANY, 'Add Kernel', size=(110, -1))
        self.sizer.Add(self.addKernelButton, 0, wx.ALIGN_LEFT | wx.ALL, 5)
        self.addKernelButton.Disable()
        
        self.sizer.Add(self.kernelList, 0, wx.EXPAND|wx.ALL, 2)
               
        self.sizer.Add(wx.StaticText(self, -1, 'Kernel ID'), 0, wx.ALIGN_RIGHT|wx.ALL, 2)
        self.kernelId = wx.TextCtrl(self)
        self.sizer.Add(self.kernelId, 0, wx.ALIGN_CENTER_VERTICAL|wx.EXPAND|wx.ALL, 2)
        
        self.sizer.Add(wx.StaticText(self, -1, 'Kernel Data'), 0, wx.ALIGN_RIGHT|wx.ALL, 2)
        self.kernelData = wx.TextCtrl(self)
        self.sizer.Add(self.kernelData, 0, wx.ALIGN_CENTER_VERTICAL|wx.EXPAND|wx.ALL, 2)
        
        
        self.sizer.Add(wx.StaticText(self),wx.EXPAND)
        
        self.hsizer = wx.BoxSizer(wx.HORIZONTAL)
        self.saveButton = wx.Button(self, wx.ID_ANY, 'Save')
        self.saveButton.Disable()
        
        self.hsizer.Add(self.saveButton, 0, wx.ALIGN_RIGHT | wx.ALL, 5)
        
        self.closeButton = wx.Button(self, wx.ID_ANY, 'Cancel')
        self.hsizer.Add(self.closeButton, 0, wx.ALIGN_RIGHT | wx.ALL, 5)
        self.sizer.Add(self.hsizer,0,wx.ALIGN_RIGHT|wx.ALL,5)
        
    
    def clear(self):
        
        self.name.Clear()
        self.serviceURL.Clear()
        self.storageURL.Clear()
        self.ec2Cert.Clear()
        self.kernelList.clear()
        self.kernelId.Clear()
        self.kernelData.Clear()
        self.saveButton.Disable()
        self.addKernelButton.Disable()
    
    def __assertFieldNotEmpty(self, name, field):
        
        if field.GetValue() == "":
            raise Exception("Field %s is blank" % name)
    

class CloudWizard(wx.Dialog):
    
    def __init__(self, *args, **kwargs):
        wx.Dialog.__init__(self, *args, **kwargs)
        
        self.vsizer = wx.BoxSizer(wx.VERTICAL)
        self.SetSizer(self.vsizer)
                
        self.container = ContainerPanel(self, size=self.GetSize())
        self.vsizer.Add(self.container, 1, wx.ALL|wx.EXPAND)
        
        self.compCloudConfPanel = CloudListPanel(self.container)
        self.container.addPanel("MAIN", self.compCloudConfPanel)  
        
        self.newCloudPanel = NewCloudPanel(self.container)
        self.container.addPanel("NEW_CLOUD", self.newCloudPanel)
        
    def showPanel(self, label):
        self.container.showPanel(label)
        
        