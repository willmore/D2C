'''
Created on Feb 28, 2011

@author: willmore
'''

import wx

class ContainerPanel(wx.Panel):
    """
    Contains multiple panels in same position, with only one visible
    """
    
    def __init__(self, parent, id=-1, size=wx.DefaultSize):
        wx.Panel.__init__(self, parent, id=id, size=size)
        self._sizer = wx.BoxSizer(wx.VERTICAL)
        self.SetSizer(self._sizer)
        self._panels = {}
    
    def addPanel(self, label, panel):
        assert panel.GetParent() == self
        self._panels[label] = panel
        self._sizer.Add(panel, 1, wx.ALL|wx.EXPAND, 0)
        panel.Hide()
        
    def showPanel(self, label):
        
        for l, p in self._panels.items():
            if l == label:
                p.Show()
            elif p.IsShown():
                p.Hide()
        self.Layout()
    
    def getPanel(self, label):
        return self._panels[label]