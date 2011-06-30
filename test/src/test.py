#!/usr/bin/python

# notebook.py

import wx
import wx.lib.sheet as sheet

class MySheet(sheet.CSheet):
    def __init__(self, parent):
        sheet.CSheet.__init__(self, parent)

        self.SetLabelBackgroundColour('#DBD4D4')
        self.SetNumberRows(50)
        self.SetNumberCols(50)

class Notebook(wx.Frame):
    def __init__(self, parent, id, title):
        wx.Frame.__init__(self, parent, id, title, size=(600, 500))

        menubar = wx.MenuBar()
        file = wx.Menu()
        file.Append(101, 'Quit', '' )
        menubar.Append(file, "&File")
        self.SetMenuBar(menubar)
        wx.EVT_MENU(self, 101, self.OnQuit)
        nb = wx.Notebook(self, -1, style=wx.NB_BOTTOM)
        self.sheet1 = MySheet(nb)
        self.sheet2 = MySheet(nb)
        self.sheet3 = MySheet(nb)
        nb.AddPage(self.sheet1, "Sheet1")
        nb.AddPage(self.sheet2, "Sheet2")
        nb.AddPage(self.sheet3, "Sheet3")
        self.sheet1.SetFocus()
        self.StatusBar()

    def StatusBar(self):
        self.statusbar = self.CreateStatusBar()

    def OnQuit(self, event):
        self.Close()

class MyApp(wx.App):
    def OnInit(self):
         frame = Notebook(None, -1, 'notebook.py')
         frame.Show(True)
         frame.Centre()
         return True

app = MyApp(0)
app.MainLoop()