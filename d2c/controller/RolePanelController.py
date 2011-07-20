import wx
from d2c.model.Role import Role
from d2c.model.Deployment import Deployment
from d2c.model.InstanceType import InstanceType
from wx.lib.pubsub import Publisher
from d2c.model.Action import Action
from d2c.model.FileExistsFinishedCheck import FileExistsFinishedCheck
from d2c.model.DataCollector import DataCollector
from d2c.model.SSHCred import SSHCred
from d2c.model.UploadAction import UploadAction
from d2c.controller.util import createEmptyChecker
import sys

def fieldsNotEmpty(self, *fields):
    for f in fields:
        if len(f.GetValue()) == 0:
            return False         
    return True
    
    
class RolePanelController:
    
    def __init__(self, view, role, dao):
        
        self.role = role
        self.dao = dao
        self.view = view
        
        self.uploadScripts = []
        self.startScripts = []
        self.endScripts = []
        self.dataCollectors = []
           
        p = self.view.panel
        self.setupFlexList(p.sw, p.sw.uploadScriptBox.boxSizer,self.uploadScripts, 2, ("Source", "Destination"),
                           initialValues=[(a.source, a.destination) for a in self.role.uploadActions])
           
        self.setupFlexList(p.sw, p.sw.startScriptBox.boxSizer,self.startScripts,
                           initialValues=[(a.command,) for a in self.role.startActions])
        
        self.setupFlexList(p.sw, p.sw.endScriptBox.boxSizer, self.endScripts,
                           initialValues=[(a.fileName,) for a in self.role.finishedChecks])
        
        self.setupFlexList(p.sw, p.sw.dataBox.boxSizer, self.dataCollectors,
                           initialValues=[(a.source,) for a in self.role.dataCollectors])
            
    def setupFlexList(self, parent, boxsizer, textControls, fieldCount=1, labels=(), initialValues=()):
        
        def handleRemove(evt): 
            for c in evt.GetEventObject().GetParent().GetChildren():
                if c in textControls:
                    textControls.remove(c)

            sizer = evt.GetEventObject().GetContainingSizer()
            sizer.Clear(deleteWindows=True)
            
            #Im not sure how to get rid of the sizer itself,
            #as calling sizer.Destroy() stops the program...
            #sizer.Destroy()
            
            self.view.panel.Layout()
        
        def handleAdd(evt=None, initialVals=None):
            if (evt is not None):
                evtBtn = evt.GetEventObject()
                evtBtn.SetLabel("Remove")
                evtBtn.Bind(wx.EVT_BUTTON, handleRemove)
            
            sizer = wx.BoxSizer(wx.HORIZONTAL)
            
            fields = []
            for i in range(fieldCount): 
                
                if len(labels) > i:
                    label = wx.StaticText(parent, -1, label=labels[i])
                    sizer.Add(label,0)
                
                field = wx.TextCtrl(parent, -1)
                sizer.Add(field, 1)
                fields.append(field)
                textControls.append(field)
                
            if initialVals is None:
                btn = wx.Button(parent, -1, "Add")
                btn.Bind(wx.EVT_BUTTON, handleAdd)
            else:
                btn = wx.Button(parent, -1, "Remove")
                btn.Bind(wx.EVT_BUTTON, handleRemove)
                for i,val in enumerate(initialVals):
                    fields[i].SetValue(val)
                
            sizer.Add(btn, 0)
            
            boxsizer.Add(sizer, 0, wx.EXPAND)
            
            createEmptyChecker(btn, *fields)
            self.view.panel.Layout()
            
        #Load up our list with initial values
        
        for vals in initialValues:
            handleAdd(None, vals)
        
        #Create last empty row
        handleAdd(None, None)   
 
if __name__ == "__main__":
    from d2c.gui.RolePanel import RolePanel
    app = wx.App()
    frame = wx.Frame(None, -1, 'blah', size=(750, 350))
    controller = RolePanelController(RolePanel(frame, -1), None)
    frame.Show()
    
    app.MainLoop()