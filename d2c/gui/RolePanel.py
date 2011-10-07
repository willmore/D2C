import wx

class RolePanel(wx.Panel):
    
    def __init__(self, modifiable, *args, **kwargs):
        wx.Panel.__init__(self, *args, **kwargs)
        
        self.modifiable = modifiable
                
        self.sizer = wx.BoxSizer(wx.VERTICAL)
        
        self.sw = wx.ScrolledWindow(self, style=wx.VSCROLL)
        self.sw.SetMinSize((-1, 300))

        self.sw.SetScrollbars(1,1,1,1)
        self.sizer.Add(self.sw, 1, wx.ALL | wx.EXPAND, 5)
        self.sw.sizer = wx.BoxSizer(wx.VERTICAL)
        
        hbox = wx.BoxSizer(wx.HORIZONTAL)
        self.instancePicker = wx.ComboBox(self, -1, style=wx.CB_READONLY)
        hbox.Add(self.instancePicker)
        
        self.imagePicker = wx.ComboBox(self, -1, style=wx.CB_READONLY)
        hbox.Add(self.imagePicker)
        
        self.countPicker = wx.SpinCtrl(self, -1, min=0, max=120)
        hbox.Add(self.countPicker)
        
        self.sizer.Add(hbox)
        
        self.sw.uploadScriptBox = wx.StaticBox(self.sw, label="Uploads")
        self.sw.uploadScriptBox.boxSizer = wx.StaticBoxSizer(self.sw.uploadScriptBox, wx.VERTICAL)
        
        self.sw.sizer.Add(self.sw.uploadScriptBox.boxSizer, 0, wx.EXPAND| wx.ALL, 2)
        ###
        
        self.sw.startScriptBox = wx.StaticBox(self.sw, label="Start Scripts")
        self.sw.startScriptBox.boxSizer = wx.StaticBoxSizer(self.sw.startScriptBox, wx.VERTICAL)
        
        self.sw.sizer.Add(self.sw.startScriptBox.boxSizer, 0, wx.EXPAND| wx.ALL, 2)
        
        #####
        self.sw.asyncScriptBox = wx.StaticBox(self.sw, label="Async Start Scripts")
        self.sw.asyncScriptBox.boxSizer = wx.StaticBoxSizer(self.sw.asyncScriptBox, wx.VERTICAL)
        
        self.sw.sizer.Add(self.sw.asyncScriptBox.boxSizer, 0, wx.EXPAND| wx.ALL, 2)
        
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
        
        if not modifiable:
            self.saveButton.Hide()
        
        self.cancelButton = wx.Button(self, wx.ID_ANY, 'Cancel' if modifiable else 'Close')
        self.hsizer.Add(self.cancelButton,wx.ALIGN_RIGHT)
        
        self.sizer.Add(self.hsizer, 0, wx.ALIGN_RIGHT | wx.BOTTOM | wx.RIGHT, 10)
        
        self.SetSizer(self.sizer)
            
        
        
class RoleDialog(wx.Dialog):
    
    def __init__(self, modifiable=True, *args, **kwargs):
        wx.Dialog.__init__(self, *args, **kwargs)
        self.modifiable = modifiable
        self.panel = RolePanel(modifiable, self, -1)