'''
Created on Feb 9, 2011

@author: willmore
'''

import wx

app = wx.App()

class Gui(wx.Frame):    
    
    def __init__(self, parent=None, id=-1, title='D2C'):
        wx.Frame.__init__(self, parent, id, title, size=(250, 150))

        menubar = wx.MenuBar()
        file = wx.Menu()
        quit = wx.MenuItem(file, 1, '&Quit\tCtrl+Q')
        file.AppendItem(quit)

        menubar.Append(file, '&File')
        self.SetMenuBar(menubar)

        self.Bind(wx.EVT_MENU, self.OnQuit, id=1)
    
    def OnQuit(self, event):
        self.Close()

    
   




        
