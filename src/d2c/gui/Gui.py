'''
Created on Feb 9, 2011

@author: willmore
'''

import wx

app = wx.App()

class Gui(wx.Frame):    
    
    _ID_LISTBOX = 1000
    
    def __init__(self, parent=None, id=-1, title='D2C'):
        wx.Frame.__init__(self, parent, id, title, size=(250, 150))

        # Menubar
        menubar = wx.MenuBar()
        file = wx.Menu()
        quit = wx.MenuItem(file, 1, '&Quit\tCtrl+Q')
        file.AppendItem(quit)

        menubar.Append(file, '&File')
        self.SetMenuBar(menubar)

        self.Bind(wx.EVT_MENU, self.OnQuit, id=1)
        
        mainPanel = wx.Panel(self, -1)

        
        #left panel items
        
        
        self._items = wx.ListBox(mainPanel, self._ID_LISTBOX, wx.DefaultPosition, (170, 130), ["Source Images", "Deployment Descriptors", "Deployments"], wx.LB_SINGLE)
        self._items.SetSelection(0)
        self.Bind(wx.EVT_LISTBOX, self.OnSelect, id=self._ID_LISTBOX)

    
    def OnQuit(self, event):
        self.Close()

    def OnSelect(self, event):
        print event
    
   




        
