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
 
        self._list = wx.ListCtrl(self.splitter, -1, style=wx.LC_REPORT, size=(110,300))
        
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
    


