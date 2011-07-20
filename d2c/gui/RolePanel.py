'''
Created on Jul 20, 2011

@author: willmore
'''
import wx

class RolePanel(wx.Panel):
    
    def __init__(self, *args, **kwargs):
        wx.Panel.__init__(self, *args, **kwargs)
                
        self.sizer = wx.BoxSizer(wx.VERTICAL)
        
        self.sw = wx.ScrolledWindow(self, style=wx.VSCROLL)
        self.sw.SetMinSize((-1, 300))
        self.sw.SetMaxSize((-1, 300))
        self.sw.SetScrollbars(1,1,1,1)
        self.sizer.Add(self.sw, 0, wx.ALL | wx.EXPAND, 5)
        self.sw.sizer = wx.BoxSizer(wx.VERTICAL)
        
        self.sw.uploadScriptBox = wx.StaticBox(self.sw, label="Uploads")
        self.sw.uploadScriptBox.boxSizer = wx.StaticBoxSizer(self.sw.uploadScriptBox, wx.VERTICAL)
        
        self.sw.sizer.Add(self.sw.uploadScriptBox.boxSizer, 0, wx.EXPAND| wx.ALL, 2)
        ###
        
        self.sw.startScriptBox = wx.StaticBox(self.sw, label="Start Scripts")
        self.sw.startScriptBox.boxSizer = wx.StaticBoxSizer(self.sw.startScriptBox, wx.VERTICAL)
        
        self.sw.sizer.Add(self.sw.startScriptBox.boxSizer, 0, wx.EXPAND| wx.ALL, 2)
        
        #####
        self.sw.endScriptBox = wx.StaticBox(self.sw, label="File Done Check")
        self.sw.endScriptBox.boxSizer = wx.StaticBoxSizer(self.sw.endScriptBox, wx.VERTICAL)
        self.sw.sizer.Add(self.sw.endScriptBox.boxSizer, 0, wx.EXPAND| wx.ALL, 2)
        
        ####
        self.sw.dataBox = wx.StaticBox(self.sw, label="Data to Collect")
        self.sw.dataBox.boxSizer = wx.StaticBoxSizer(self.sw.dataBox, wx.VERTICAL)      
        self.sw.sizer.Add(self.sw.dataBox.boxSizer, 0, wx.EXPAND | wx.ALL, 2)
        
        self.sw.SetSizer(self.sw.sizer)
        
        self.hsizer = wx.BoxSizer(wx.HORIZONTAL)
        
        self.saveButton = wx.Button(self, wx.ID_ANY, 'Save', style=wx.SAVE)
        self.hsizer.Add(self.saveButton,0, wx.ALIGN_RIGHT)
        
        self.cancelButton = wx.Button(self, wx.ID_ANY, 'Cancel')
        self.hsizer.Add(self.cancelButton,wx.ALIGN_RIGHT)
        
        self.sizer.Add(self.hsizer, 0, wx.ALIGN_RIGHT|wx.ALL, 2)
        
        self.SetSizer(self.sizer)
            
        
        
class RoleDialog(wx.Dialog):
    
    def __init__(self, *args, **kwargs):
        wx.Dialog.__init__(self, *args, **kwargs)
        
        self.panel = RolePanel(self, -1)