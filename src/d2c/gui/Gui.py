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
        wx.Frame.__init__(self, parent, id, title, size=(250, 150))

        # Menubar
        menubar = wx.MenuBar()
        file = wx.Menu()
        quit = wx.MenuItem(file, 1, '&Quit\tCtrl+Q')
        file.AppendItem(quit)

        menubar.Append(file, '&File')
        self.SetMenuBar(menubar)

        self.Bind(wx.EVT_MENU, self.OnQuit, id=1)
        
        hbox1 = wx.BoxSizer(wx.HORIZONTAL)
        
        #left panel items 
        self._items = wx.ListBox(self, self._ID_LISTBOX, wx.DefaultPosition, (170, 130), ["Source Images", "Deployment Descriptors", "Deployments"], wx.LB_SINGLE)
        self._items.SetSelection(0)
        self.Bind(wx.EVT_LISTBOX, self.OnSelect, id=self._ID_LISTBOX)

        hbox1.Add(self._items, 0, wx.ALL, 5)


        self._sourceImagesPanel = wx.Panel(self, -1)
        wx.Button(self._sourceImagesPanel, self._ID_ADD_IMAGE, 'Add Image', (150, 130), (110, -1))
        self.Bind(wx.EVT_BUTTON, self.OnAddImage, id=self._ID_ADD_IMAGE)
        
        hbox1.Add(self._sourceImagesPanel, 1, wx.ALL, 5)
        
        #self._deploymentDescPanel = wx.Panel(self, -1)
        
        self.SetSizer(hbox1)
    
    def OnQuit(self, event):
        self.Close()

    def OnSelect(self, event):
        print event
    
    def OnAddImage(self, event):
        print "Add Image"
    
   




        
