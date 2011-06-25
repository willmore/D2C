'''
Created on Mar 10, 2011

@author: willmore
'''
import wx
import os
from .ItemList import ColumnMapper, ItemList

class RawImagePanel(wx.Panel):    
    
    def __init__(self, *args):
        wx.Panel.__init__(self, *args)

        vbox = wx.BoxSizer(wx.VERTICAL)
        
        self._list = wx.ListCtrl(self, -1, style=wx.LC_REPORT, size=(110,200))
        
        self._list = ItemList(self, -1, style=wx.LC_REPORT, size=(110, 200),
                              mappers=[ColumnMapper('Path', lambda r: r.path, defaultWidth=wx.LIST_AUTOSIZE)])
        
        hbox1 = wx.BoxSizer(wx.HORIZONTAL)
        hbox1.Add(self._list, 1, wx.EXPAND)
      
        hbox2 = wx.BoxSizer(wx.HORIZONTAL)

        self._addButton = wx.Button(self, wx.ID_ANY, 'Add Image', size=(110, -1)) 
        self._newFile = wx.TextCtrl(self)
    
        self._findButton = wx.Button(self, wx.ID_ANY, 'Find Image', size=(110, -1))
        self._findButton.Bind(wx.EVT_BUTTON, self._OnFindImage)
    
        self.createAMIButton = wx.Button(self, wx.ID_ANY, 'Create AMI', size=(110, -1))
    
        hbox2.Add(self._addButton, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALL, 2)
        hbox2.Add(self._findButton, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALL, 2)
        hbox2.Add(self._newFile, 1, wx.ALIGN_CENTER_VERTICAL|wx.ALL, 2)
        
        vbox.Add(hbox1, 1, wx.EXPAND)
        vbox.Add(hbox2, 0, wx.EXPAND)
        vbox.Add(self.createAMIButton, 0, wx.ALL, 2)
        self.SetSizer(vbox)
        
    def SetImages(self, images): 
      
        self._list.setItems(images)
    
    def _OnFindImage(self, event):
        dlg = wx.FileDialog(self, "Choose an image", os.getcwd(), "", "*.*", wx.OPEN)
        if dlg.ShowModal() == wx.ID_OK:
            path = dlg.GetPath()
            self._newFile.SetValue(path)
        dlg.Destroy()