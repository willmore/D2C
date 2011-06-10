'''
Created on Mar 10, 2011

@author: willmore
'''

import wx
from .ContainerPanel import ContainerPanel
from d2c.model.Region import Region
from d2c.model.Storage import WalrusStorage
  
class ImageStoreList(wx.ListCtrl):
    
    def __init__(self, *args, **kwargs):
        wx.ListCtrl.__init__(self, *args, **kwargs)
        
        self.InsertColumn(0, 'Name', width=75)
        self.InsertColumn(1, 'Service URL', width=200)
        
        self.stores = {}
        
        if kwargs.has_key('stores'):
            self.setRegions(kwargs['stores'])
    
    def setStores(self, stores):
        self.DeleteAllItems()
        self.stores.clear()
        
        for store in stores:
            self.addStoreEntry(store)
            
    def addStoreEntry(self, store):
        idx = self.Append((store.name,store.serviceURL))
        self.stores[idx] = store  
        
    def getSelectedStores(self):
        #TODO return more than one store if selected
        i = self.GetFirstSelected();
        
        if i < 0:
            return []
        else:
            return [self.stores[i]]
       
class StoreConfPanel(wx.Panel):    
    
    def __init__(self, *args, **kwargs):
        wx.Panel.__init__(self, *args, **kwargs)
 
        self.storeList = ImageStoreList(self, -1, style=wx.LC_REPORT, size=(-1, 200))
        self.addButton = wx.Button(self, wx.ID_ANY, 'Add New Store', size=(190, -1))
        self.doneButton = wx.Button(self, wx.ID_ANY, 'Done', size=(190, -1))
        
        self.sizer = wx.BoxSizer(wx.VERTICAL) 
        self.SetSizer(self.sizer)
        
        self.sizer.Add(self.storeList, 0, wx.EXPAND|wx.ALL, 5)
        self.sizer.Add(self.addButton, 0, wx.ALIGN_RIGHT|wx.ALL, 2)
        self.sizer.Add(self.doneButton, 0, wx.ALIGN_RIGHT|wx.ALL, 2)     
        
    def setStores(self, stores):
        self.storeList.setStores(stores)
        
    def addStore(self, store):
        self.storeList.addStoreEntry(store)
        
class NewStorePanel(wx.Panel):    
    
    def __init__(self, *args, **kwargs):
        wx.Panel.__init__(self, *args, **kwargs)
        
        self.sizer = wx.BoxSizer(wx.VERTICAL)
        self.SetSizer(self.sizer)
        
        addStoreTxt = wx.StaticText(self, -1, 'Add Store')
        addStoreTxt.SetFont(wx.Font(15, wx.DEFAULT, wx.DEFAULT, wx.BOLD))
        self.sizer.Add(addStoreTxt, 0)
                
        self.name = wx.TextCtrl(self)
        
        self.serviceURL = wx.TextCtrl(self)
       
        fgs = wx.FlexGridSizer(2,2,0,0)
        fgs.AddGrowableCol(1, 1)
        
        fgs.AddMany([   (wx.StaticText(self, -1, 'Name'),0, wx.ALIGN_RIGHT|wx.ALL, 2),
                        (self.name, 0, wx.ALIGN_LEFT|wx.ALIGN_CENTER_VERTICAL|wx.EXPAND|wx.ALL, 2),
                       
                        (wx.StaticText(self, -1, 'Service URL'),0, wx.ALIGN_RIGHT|wx.ALL, 2),
                        (self.serviceURL, 0, wx.ALIGN_LEFT|wx.ALIGN_CENTER_VERTICAL|wx.EXPAND|wx.ALL, 2)
                    ])
        
        self.sizer.Add(fgs, 0, wx.ALL | wx.EXPAND, 5)
        
        self.addButton = wx.Button(self, wx.ID_ANY, 'Add New Store', size=(110, -1))
        self.sizer.Add(self.addButton, 0, wx.ALIGN_RIGHT | wx.ALL, 5)
    
    def getStore(self):
        
        self.__assertFieldNotEmpty("Name", self.name)
        self.__assertFieldNotEmpty("Service URL", self.serviceURL)
       
        return WalrusStorage(self.name.GetValue(), self.serviceURL.GetValue())
    
    def __assertFieldNotEmpty(self, name, field):
        
        if field.GetValue() == "":
            raise Exception("Field %s is blank" % name)
    
    def clear(self):
        self.name.Clear()
        self.serviceURL.Clear()

class ImageStoreWizard(wx.Dialog):
    
    def __init__(self, *args, **kwargs):
        wx.Dialog.__init__(self, *args, **kwargs)
        
        self.vsizer = wx.BoxSizer(wx.VERTICAL)
        self.SetSizer(self.vsizer)
                
        self.container = ContainerPanel(self, size=self.GetSize())
        self.vsizer.Add(self.container, 1, wx.ALL|wx.EXPAND)
        
        self.storeConfPanel = StoreConfPanel(self.container)
        self.container.addPanel("MAIN", self.storeConfPanel)  
        
        self.newStorePanel = NewStorePanel(self.container)
        self.container.addPanel("NEW_STORE", self.newStorePanel)
        
    def showPanel(self, label):
        self.container.showPanel(label)
        
        