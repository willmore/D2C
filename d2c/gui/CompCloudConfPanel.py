'''
Created on Mar 10, 2011

@author: willmore
'''

import wx
from .ContainerPanel import ContainerPanel
  
class RegionList(wx.ListCtrl):
    
    def __init__(self, *args, **kwargs):
        wx.ListCtrl.__init__(self, *args, **kwargs)
        
        self.InsertColumn(0, 'Name', width=75)
        self.InsertColumn(1, 'Endpoint', width=200)
        self.InsertColumn(2, 'EC2 Cert', width=200)
        
        self.regions = {}
        
        if kwargs.has_key('regions'):
            self.setRegions(kwargs['regions'])
    
    def setRegions(self, regions):
        self.DeleteAllItems()
        self.regions.clear()
        
        for region in regions:
            self.addRegionEntry(region)
            
    def addRegionEntry(self, region):
        idx = self.Append((region.getName(),region.getEndpoint(), region.getEC2Cert()))
        self.regions[idx] = region  
       
class CompCloudConfPanel(wx.Panel):    
    
    def __init__(self, *args, **kwargs):
        wx.Panel.__init__(self, *args, **kwargs)
 
        self.regionList = RegionList(self, -1, style=wx.LC_REPORT, size=(-1, 200))
        self.addButton = wx.Button(self, wx.ID_ANY, 'Add Computation Cloud', size=(190, -1))
        
        self.sizer = wx.BoxSizer(wx.VERTICAL) 
        self.SetSizer(self.sizer)
        
        self.sizer.Add(self.regionList, 0, wx.EXPAND|wx.ALL, 5)
        self.sizer.Add(self.addButton, 0, wx.ALIGN_RIGHT|wx.ALL, 2)
        
    def setRegions(self, regions):
        self.regionList.setRegions(regions)
        

class CompCloudWizard(wx.Dialog):
    
    def __init__(self, *args, **kwargs):
        wx.Dialog.__init__(self, *args, **kwargs)
        
        self.vsizer = wx.BoxSizer(wx.VERTICAL)
        self.SetSizer(self.vsizer)
                
        self.container = ContainerPanel(self, size=self.GetSize())
        self.vsizer.Add(self.container, 1, wx.ALL|wx.EXPAND)
        
        self.compCloudConfPanel = CompCloudConfPanel(self.container)
        self.container.addPanel("MAIN", self.compCloudConfPanel)  
        
        
    def showPanel(self, label):
        self.container.showPanel(label)
        
        