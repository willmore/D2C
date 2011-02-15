'''
Created on Feb 9, 2011

@author: willmore
'''

import wx
import os

from d2c.data.dao import DAO


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
        
        gridSizer = wx.FlexGridSizer(rows=1, cols=2, hgap=5, vgap=5)
        gridSizer.AddGrowableCol(1, 1)
        
        
        #left panel items 
        labels = []
        self._containerPanel = ContainerPanel(self)
        for (label, panel) in [("Credentials", CredentialPanel(self._containerPanel)),
                               ("Source Images", RawImagePanel(self._containerPanel))
                               ]:
            self._containerPanel.addPanel(label, panel)
            labels.append(label)
        
        self._items = wx.ListBox(self, self._ID_LISTBOX, wx.DefaultPosition, (170, 130), labels, wx.LB_SINGLE)
        self._items.SetSelection(0)
        self.Bind(wx.EVT_LISTBOX, self.OnListSelect, id=self._ID_LISTBOX)

        gridSizer.Add(self._items, 0, wx.ALL|wx.EXPAND, 5) 
        gridSizer.Add(self._containerPanel, 0, wx.ALL|wx.EXPAND, 5)
        
        self.SetSizer(gridSizer)
        
    def getCredentialPanel(self):
        return self._containerPanel.getPanel("Credentials")
    
    def OnQuit(self, event):
        self.Close()

    def OnListSelect(self, event):
        self._containerPanel.showPanel(self._items.GetStringSelection())

    
    def OnAddImage(self, event):
        print "Add Image"
    
class ContainerPanel(wx.Panel):
    "Contains multiple panels in same position, with only one visible"
    
    _panels = {}

    def __init__(self, parent):
        wx.Panel.__init__(self, parent)
        self._sizer = wx.BoxSizer(wx.VERTICAL)
        self.SetSizer(self._sizer)
    
    def addPanel(self, label, panel):
        assert panel.GetParent() == self
        self._panels[label] = panel
        self._sizer.Add(panel, 0, wx.ALL|wx.EXPAND, 0)
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

class RawImagePanel(wx.Panel):    
    
    def __init__(self, *args):
        wx.Panel.__init__(self, *args)

        vbox = wx.BoxSizer(wx.VERTICAL)
        self.SetSizer(vbox)
        
        hbox1 = wx.BoxSizer(wx.HORIZONTAL)
        hbox1.Add(wx.TextCtrl(self), 1, wx.EXPAND)
        
        hbox2 = wx.BoxSizer(wx.HORIZONTAL)

        self._addButton = wx.Button(self, wx.ID_ANY, 'Add Image', size=(110, -1))        
        self._newFile = wx.TextCtrl(self)
    
    
        self._findButton = wx.Button(self, wx.ID_ANY, 'Find Image', size=(110, -1))
        self._findButton.Bind(wx.EVT_BUTTON, self.OnFindImage)
    
    
        hbox2.Add(self._addButton, 0, wx.ALIGN_CENTER_VERTICAL)
        hbox2.Add(self._newFile, 1, wx.ALIGN_CENTER_VERTICAL)
        
        vbox.Add(hbox1, 1, wx.EXPAND)
        vbox.Add(self._findButton, 0)
        vbox.Add(hbox2, 0, wx.EXPAND)
        
    def OnFindImage(self, event):
        dlg = wx.FileDialog(self, "Choose an image", os.getcwd(), "", "*.*", wx.OPEN)
        if dlg.ShowModal() == wx.ID_OK:
            path = dlg.GetPath()
            self._newFile.SetValue(path)
        dlg.Destroy()



class CredentialPanel(wx.Panel):    
    
    def __init__(self, parent, id=-1):
        wx.Panel.__init__(self, parent, id)
 
        self._aws_key_id = wx.TextCtrl(self);
        self._aws_secret_access_key = wx.TextCtrl(self);
        self._ec2_cert = wx.TextCtrl(self);
        self._ec2_private_key = wx.TextCtrl(self);   
        self._updateButton = wx.Button(self, wx.ID_ANY, 'Save Credentials', size=(130, -1))
        
        
        fgs = wx.FlexGridSizer(5,2,0,0)
        fgs.AddGrowableCol(1, 1)
        
        fgs.AddMany([ (wx.StaticText(self, -1, 'AWS Key ID'),0, wx.ALIGN_CENTER),
                           (self._aws_key_id, 0, wx.ALIGN_LEFT|wx.ALIGN_CENTER_VERTICAL|wx.EXPAND),
                           
                           (wx.StaticText(self, -1, 'AWS Secret Access Key'),0, wx.ALIGN_CENTER_HORIZONTAL),
                           (self._aws_secret_access_key,0, wx.ALIGN_LEFT|wx.ALIGN_CENTER_VERTICAL|wx.EXPAND),
                           
                           (wx.StaticText(self, -1, 'EC2 Certificate'),0, wx.ALIGN_CENTER),
                           (self._ec2_cert, 0, wx.ALIGN_LEFT|wx.ALIGN_CENTER_VERTICAL|wx.EXPAND),
                           
                           (wx.StaticText(self, -1, 'EC2 Private Key'),0, wx.ALIGN_CENTER_HORIZONTAL),
                           (self._ec2_private_key,0, wx.ALIGN_LEFT|wx.ALIGN_CENTER_VERTICAL|wx.EXPAND),
                            
                           (self._updateButton, 1)])
        
        self.SetSizer(fgs)



        
