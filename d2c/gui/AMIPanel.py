'''
Created on Mar 10, 2011

@author: willmore
'''

import wx
from .ContainerPanel import ContainerPanel

class AMIPanel(wx.Panel):    
    
    def __init__(self, *args):
        wx.Panel.__init__(self, *args)
        self.splitter = wx.SplitterWindow(self, -1)
        
        vbox = wx.BoxSizer(wx.VERTICAL)
 
        self._list = wx.ListCtrl(self.splitter, -1, style=wx.LC_REPORT, size=(110,250))
        
        hbox1 = wx.BoxSizer(wx.HORIZONTAL)
        hbox1.Add(self.splitter, 1, wx.EXPAND)
        
        self._list.InsertColumn(0, 'AMI', width=75)
        self._list.InsertColumn(1, 'Source Image', width=200)
        self._list.InsertColumn(2, 'Status', width=110)
        self._list.InsertColumn(3, 'Created', width=110)
        
        vbox.Add(hbox1, 0, wx.EXPAND)
        
        self._logPanel = ContainerPanel(self.splitter, -1, size=(200,100))
   
        self.splitter.SplitHorizontally(self._list, self._logPanel)
        
        self.SetSizer(vbox)
        
        self.Layout()   
        
        self.Bind(wx.EVT_RIGHT_DOWN, self.OnRightDown)
        self.Bind(wx.EVT_LEFT_DOWN, self.OnRightDown)
        
        self._list.Bind(wx.EVT_RIGHT_DOWN, self.OnRightDown)

    def OnRightDown(self, event):
        item, flags = self._list.HitTest(event.GetPosition())
        if flags == wx.NOT_FOUND:
            event.Skip()
            return
        self._list.Select(item)
        self.PopupMenu(MyPopupMenu('Test'), event.GetPosition())
         
    def addLogPanel(self, id):
        self._logPanel.addPanel(id, wx.TextCtrl(self._logPanel, -1, size=(100,100), style=wx.TE_MULTILINE )) #style=wx.TE_READONLY
        
    def showLogPanel(self, id):
        self._logPanel.showPanel(id)
        
    def appendLogPanelText(self, logPanelId, text):
        self._logPanel.getPanel(logPanelId).AppendText(str(text) + "\n")
        
    def addAMIEntry(self, name):
        self._list.Append((name,))
    
    def setAMIs(self, images): 
        self._list.DeleteAllItems()
        
        for ami in images:
            self._list.Append((ami.amiId,ami.srcImg))
    
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

