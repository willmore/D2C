'''
Created on Feb 9, 2011

@author: willmore
'''

import wx

app = wx.App()

class Gui(wx.Frame):    
    
    _ID_LISTBOX = 1
    _ID_ADD_IMAGE = 2
    
    def __init__(self, parent=None, id=-1, title='D2C'):
        wx.Frame.__init__(self, parent, id, title, size=(550, 550))

        # Menubar
        menubar = wx.MenuBar()
        file = wx.Menu()
        quit = wx.MenuItem(file, 1, '&Quit\tCtrl+Q')
        file.AppendItem(quit)

        menubar.Append(file, '&File')
        self.SetMenuBar(menubar)

        self.Bind(wx.EVT_MENU, self.OnQuit, id=1)
        
        gridSizer = wx.GridSizer(rows=1, cols=2, hgap=5, vgap=5)
        
        #left panel items 
        self._items = wx.ListBox(self, self._ID_LISTBOX, wx.DefaultPosition, (170, 130), ["Source Images", "Dummy", "Deployment Descriptors", "Deployments"], wx.LB_SINGLE)
        self._items.SetSelection(0)
        self.Bind(wx.EVT_LISTBOX, self.OnListSelect, id=self._ID_LISTBOX)

        gridSizer.Add(self._items, 0, wx.ALL|wx.EXPAND, 5)
        
        self._containerPanel = ContainerPanel(self)
        
        
        gridSizer.Add(self._containerPanel, 0, wx.ALL|wx.EXPAND, 5)
        
        self._containerPanel.addPanel("Source Images", RawImagePanel(self._containerPanel))
        self._containerPanel.addPanel("Dummy", DummyPanel(self._containerPanel))
        
        self.SetSizer(gridSizer)
 
    
    def OnQuit(self, event):
        self.Close()

    def OnListSelect(self, event):
        self._containerPanel.showPanel(self._items.GetStringSelection())

    
    def OnAddImage(self, event):
        print "Add Image"
    
class ContainerPanel(wx.Panel):
    
    _panels = {}

    def __init__(self, parent, id=-1):
        wx.Panel.__init__(self, parent, id)
        self._sizer = wx.BoxSizer(wx.VERTICAL)
        #self.sizer.Add(self.panel_one, 1, wx.EXPAND)
        #self.sizer.Add(self.panel_two, 1, wx.EXPAND)
        self.SetSizer(self._sizer)
    
    def addPanel(self, label, panel):
        assert panel.GetParent() == self
        self._panels[label] = panel
        self._sizer.Add(panel, 0, wx.ALL|wx.EXPAND, 0)
        
    def showPanel(self, label):
        for l, p in self._panels.items():
            if l == label:
                p.Show()
            else:
                p.Hide()
                

class RawImagePanel(wx.Panel):    
    
    def __init__(self, *args):
        wx.Panel.__init__(self, *args)

        self._addButton = wx.Button(self, wx.ID_ANY, 'Add Image', size=(110, -1))
        self.Bind(wx.EVT_BUTTON, self.OnAddImage, id=self._addButton.GetId())
    
    def OnAddImage(self, event):
        print "Add Image"


class DummyPanel(wx.Panel):    
    
    def __init__(self, parent, id=-1):
        wx.Panel.__init__(self, parent, id)
        
        wx.StaticText(self, -1, 'Dummy Panel')




        
