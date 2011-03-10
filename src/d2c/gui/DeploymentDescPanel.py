'''
Created on Mar 10, 2011

@author: willmore
'''

'''
Created on Mar 10, 2011

@author: willmore
'''

import wx
from .ContainerPanel import ContainerPanel

class DeploymentDescPanel(wx.Panel):    
    
    def __init__(self, *args):
        wx.Panel.__init__(self, *args)
        self.splitter = wx.SplitterWindow(self, -1)
        
        vbox = wx.BoxSizer(wx.VERTICAL)
 
        self._list = wx.ListCtrl(self.splitter, -1, style=wx.LC_REPORT, size=(110,300))
        
        hbox1 = wx.BoxSizer(wx.HORIZONTAL)
        hbox1.Add(self.splitter, 1, wx.EXPAND)
        
        self._list.InsertColumn(0, 'Name', width=110)
        self._list.InsertColumn(1, 'Created', width=110)
        
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
    
    def setDescs(self, descs): 
        self._list.DeleteAllItems()
        
        for d in descs:
            self._list.Append((d.name,))
