'''
Created on Mar 10, 2011

@author: willmore
'''

import wx
from .ItemList import ColumnMapper, ItemList
from .ContainerPanel import ContainerPanel

class AMIPanel(wx.Panel):    
    
    def __init__(self, *args):
        wx.Panel.__init__(self, *args)
        self.splitter = wx.SplitterWindow(self, -1)
        
        vbox = wx.BoxSizer(wx.VERTICAL)

        self._list = ItemList(self.splitter, -1, style=wx.LC_REPORT, size=(110,110),
                                 mappers=[ColumnMapper('AMI', lambda r: r.id, defaultWidth=wx.LIST_AUTOSIZE),
                                          ColumnMapper('SourceImage', lambda r: r.srcImg.path, defaultWidth=wx.LIST_AUTOSIZE),
                                          ColumnMapper('Status', lambda r: '', defaultWidth=wx.LIST_AUTOSIZE_USEHEADER),
                                          ColumnMapper('Created', lambda r: '', defaultWidth=wx.LIST_AUTOSIZE_USEHEADER)]
                                 )
        
        hbox1 = wx.BoxSizer(wx.HORIZONTAL)
        hbox1.Add(self.splitter, 1, wx.EXPAND)
        
       
        
        vbox.Add(hbox1, 0, wx.EXPAND)
        
        self._logPanel = ContainerPanel(self.splitter, -1, size=(200,100))
   
        self.splitter.SplitHorizontally(self._list, self._logPanel)
        
        self.SetSizer(vbox)
        
        self.Layout()   
        
        #self.Bind(wx.EVT_RIGHT_DOWN, self.OnRightDown)
        #self.Bind(wx.EVT_LEFT_DOWN, self.OnRightDown)
        
        #self._list.Bind(wx.EVT_RIGHT_DOWN, self.OnRightDown)
    
    '''
    def OnRightDown(self, event):
        item, flags = self._list.HitTest(event.GetPosition())
        if flags == wx.NOT_FOUND:
            event.Skip()
            return
        self._list.Select(item)
        self.PopupMenu(MyPopupMenu('Test'), event.GetPosition())
    '''
         
    def addLogPanel(self, id):
        self._logPanel.addPanel(id, wx.TextCtrl(self._logPanel, -1, size=(100,100), style=wx.TE_MULTILINE )) #style=wx.TE_READONLY
        
    def showLogPanel(self, id):
        self._logPanel.showPanel(id)
        self.Layout()
        self.Refresh()
        
    def appendLogPanelText(self, logPanelId, text):
        self._logPanel.getPanel(logPanelId).AppendText(str(text) + "\n")
        
    def addAMIEntry(self, ami):
        self._list.addItem(ami)
        
    
    def setAMIs(self, images): 
        self._list.setItems(images)
    
class MyPopupMenu(wx.Menu):
    def __init__(self, WinName):
        wx.Menu.__init__(self)

        self.WinName = WinName
        item = wx.MenuItem(self, wx.NewId(), "Item One")
        self.AppendItem(item)
        self.Bind(wx.EVT_MENU, self.OnItem1, item)
        item = wx.MenuItem(self, wx.NewId(),"Item Two")
        self.AppendItem(item)
        self.Bind(wx.EVT_MENU, self.OnItem2, item)
        item = wx.MenuItem(self, wx.NewId(),"Item Three")
        self.AppendItem(item)
        self.Bind(wx.EVT_MENU, self.OnItem3, item)

    def OnItem1(self, event):
        print "Item One selected in the %s window"%self.WinName

    def OnItem2(self, event):
        print "Item Two selected in the %s window"%self.WinName

    def OnItem3(self, event):
        print "Item Three selected in the %s window"%self.WinName


