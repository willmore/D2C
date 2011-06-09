'''
Created on Mar 10, 2011

@author: willmore
'''

import wx
  
class RegionList(wx.ListCtrl):
    
    def __init__(self, *args, **kwargs):
        wx.ListCtrl.__init__(self, *args, **kwargs)
        
        self.InsertColumn(0, 'Name', width=75)
        self.InsertColumn(1, 'Endpoint', width=200)
        self.InsertColumn(1, 'EC2 Cert', width=200)
        
        self.regions = {}
        
        if kwargs.has_key('regions'):
            self.setRegions(kwargs['regions'])
    
    def setRegions(self, regions):
        self.DeleteAllItems()
        self.regions.clear()
        
        for region in regions:
            self.addRegionEntry(region)
            
    def addRegionEntry(self, region):
        idx = self.Append((region.getName(),region.getEndpoint()))
        self.regions[idx] = region  
       
class CompCloudConfPanel(wx.Dialog):    
    
    def __init__(self, *args, **kwargs):
        wx.Dialog.__init__(self, *args, **kwargs)
 
        self.regionList = RegionList(self, -1, style=wx.LC_REPORT, size=(-1, 200))
        self.addButton = wx.Button(self, wx.ID_ANY, 'Add Computation Cloud', size=(190, -1))
        
        self.sizer = wx.BoxSizer(wx.VERTICAL)
        
        self.sizer.Add(self.regionList, 0, wx.EXPAND|wx.ALL, 5)
        self.sizer.Add(self.addButton, 0, wx.ALIGN_RIGHT|wx.ALL, 2)
        
        self.SetSizer(self.sizer)
        
    def setRegions(self, regions):
        self.regionList.setRegions(regions)
        
        