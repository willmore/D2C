'''
Created on Mar 16, 2011

@author: willmore
'''
import wx

class ColumnMapper:
    
    def __init__(self, name, mapFunc, defaultWidth=50):
        self.name = name
        self.map = mapFunc
        self.defaultWidth = defaultWidth

class ItemList(wx.ListCtrl):
    
    def __init__(self, *args, **kwargs):
        
        if not kwargs.has_key('mappers'):
            raise Exception("No Mappers")
        
        self.mappers = kwargs['mappers']
        del kwargs['mappers']
        
        items = []
        if kwargs.has_key('items'):
            items = kwargs['items']
            del kwargs['items']
        
        wx.ListCtrl.__init__(self, *args, **kwargs)
                                           
        for i,colMapper in enumerate(self.mappers):
            self.InsertColumn(i, colMapper.name, width=colMapper.defaultWidth) 

        self.items = {}
        
        self.setItems(items)
        
    def addItem(self, item):
        idx = self.Append([colMapper.map(item) for colMapper in self.mappers])
        self.items[idx] = item
    
    def getItems(self):
        return self.items.values()
    
    def clear(self):
        self.DeleteAllItems()
        self.items.clear()
    
    def setItems(self, items): 
        self.clear()
        
        for item in items:
            self.addItem(item)
            
    def getSelectedItems(self):
        #TODO return more than one item if selected
        i = self.GetFirstSelected();
        
        if i < 0:
            return []
        else:
            return [self.items[i]]