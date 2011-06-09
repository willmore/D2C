'''
Created on Mar 10, 2011

@author: willmore
'''

import wx
from .ContainerPanel import ContainerPanel
from d2c.model.Region import Region
  
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
        
    def addRegion(self, region):
        self.regionList.addRegionEntry(region)
        
class NewCloudPanel(wx.Panel):    
    
    def __init__(self, *args, **kwargs):
        wx.Panel.__init__(self, *args, **kwargs)
        
        self.sizer = wx.BoxSizer(wx.VERTICAL)
        self.SetSizer(self.sizer)
        
        addRoleTxt = wx.StaticText(self, -1, 'Add Computation Cloud')
        addRoleTxt.SetFont(wx.Font(15, wx.DEFAULT, wx.DEFAULT, wx.BOLD))
        self.sizer.Add(addRoleTxt, 0)
                
        self.name = wx.TextCtrl(self)
        
        self.endpoint = wx.TextCtrl(self)
        self.ec2Cert = wx.TextCtrl(self)
       
        fgs = wx.FlexGridSizer(7,2,0,0)
        fgs.AddGrowableCol(1, 1)
        
        fgs.AddMany([   (wx.StaticText(self, -1, 'Name'),0, wx.ALIGN_RIGHT|wx.ALL, 2),
                        (self.name, 0, wx.ALIGN_LEFT|wx.ALIGN_CENTER_VERTICAL|wx.EXPAND|wx.ALL, 2),
                       
                        (wx.StaticText(self, -1, 'Endpoint'),0, wx.ALIGN_RIGHT|wx.ALL, 2),
                        (self.endpoint, 0, wx.ALIGN_LEFT|wx.ALIGN_CENTER_VERTICAL|wx.EXPAND|wx.ALL, 2),
                        
                        (wx.StaticText(self, -1, 'EC2 Cert'),0, wx.ALIGN_RIGHT|wx.ALL, 2),
                        (self.ec2Cert, 0, wx.ALIGN_LEFT|wx.ALIGN_CENTER_VERTICAL|wx.EXPAND|wx.ALL, 2)
                    ])
        
        self.sizer.Add(fgs, 0, wx.ALL | wx.EXPAND, 5)
        
        self.addButton = wx.Button(self, wx.ID_ANY, 'Add New Computation Cloud', size=(110, -1))
        self.sizer.Add(self.addButton, 0, wx.ALIGN_RIGHT | wx.ALL, 5)
    
    def getRegion(self):
        
        self.assertFieldNotEmpty("Name", self.name)
        self.assertFieldNotEmpty("End Point", self.endpoint)
        self.assertFieldNotEmpty("EC2 Cert", self.ec2Cert)
        
        return Region(self.name.GetValue(), self.endpoint.GetValue(), self.ec2Cert.GetValue())
    
    def assertFieldNotEmpty(self, name, field):
       
        
        if field.GetValue() == "":
            raise Exception("Field %s is blank" % name)
    
    def clear(self):
        self.name.Clear()
        self.endpoint.Clear()
        self.ec2Cert.Clear()

class CompCloudWizard(wx.Dialog):
    
    def __init__(self, *args, **kwargs):
        wx.Dialog.__init__(self, *args, **kwargs)
        
        self.vsizer = wx.BoxSizer(wx.VERTICAL)
        self.SetSizer(self.vsizer)
                
        self.container = ContainerPanel(self, size=self.GetSize())
        self.vsizer.Add(self.container, 1, wx.ALL|wx.EXPAND)
        
        self.compCloudConfPanel = CompCloudConfPanel(self.container)
        self.container.addPanel("MAIN", self.compCloudConfPanel)  
        
        self.newCloudPanel = NewCloudPanel(self.container)
        self.container.addPanel("NEW_CLOUD", self.newCloudPanel)
        
    def showPanel(self, label):
        self.container.showPanel(label)
        
        