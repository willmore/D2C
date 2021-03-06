import wx
from d2c.model.Role import Role
from d2c.model.SourceImage import AMI
from d2c.model.Deployment import Deployment
from d2c.model.InstanceType import InstanceType
from wx.lib.pubsub import Publisher
from d2c.model.Action import StartAction
from d2c.model.AsyncAction import AsyncAction
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
        self.asyncScripts = []
        self.endScripts = []
        self.dataCollectors = []
           
        p = self.view.panel
        self.setupFlexList(p.sw, p.sw.uploadScriptBox.boxSizer,self.uploadScripts, 2, ("Source", "Destination"),
                           initialValues=[(a.source, a.destination) for a in self.role.uploadActions])
           
        self.setupFlexList(p.sw, p.sw.startScriptBox.boxSizer,self.startScripts,
                           initialValues=[(a.command,) for a in self.role.startActions])
        
        self.setupFlexList(p.sw, p.sw.asyncScriptBox.boxSizer,self.asyncScripts,
                           initialValues=[(a.command,) for a in self.role.asyncStartActions])
        
        self.setupFlexList(p.sw, p.sw.endScriptBox.boxSizer, self.endScripts,
                           initialValues=[(a.fileName,) for a in self.role.finishedChecks])
        
        self.setupFlexList(p.sw, p.sw.dataBox.boxSizer, self.dataCollectors,
                           initialValues=[(a.source,) for a in self.role.dataCollectors])
        
        p.instancePicker.SetValue(role.instanceType.name)
        p.instancePicker.AppendItems([i.name for i in role.instanceType.cloud.instanceTypes])
        
        
        p.imagePicker.SetValue(role.image.amiId if isinstance(role.image, AMI) else str(role.image.id))
        p.imagePicker.AppendItems([i.amiId for i in self.dao.getAMIs()])

        p.countPicker.SetValue(role.count)
        
        self.view.panel.saveButton.Bind(wx.EVT_BUTTON, self.handleSave)
        self.view.panel.cancelButton.Bind(wx.EVT_BUTTON, self.handleCancel)

    def handleCancel(self, _):
        self.view.EndModal(wx.CANCEL)

    def handleSave(self, _):
        '''
        The simplest approach is to delete all pre-existing role actions and create new ones.
        '''
        
        self._clearCollections((self.role.dataCollectors, self.role.finishedChecks,
                               self.role.startActions, self.role.uploadActions))
        tmpCred = SSHCred(None, "placeholder", "foobar", "foobar")
        self.role.uploadActions = []
        for i in range(0, len(self.uploadScripts), 2):
            self.role.uploadActions.append(UploadAction(self.uploadScripts[i].GetValue(), 
                                             self.uploadScripts[i+1].GetValue(),
                                             tmpCred))        
        
        self.role.startActions = [StartAction(s.GetValue(), tmpCred) for s in self.startScripts] 
        self.role.asyncStartActions = [AsyncAction(s.GetValue(), tmpCred) for s in self.asyncScripts] 
        self.role.finishedChecks = [FileExistsFinishedCheck(f.GetValue(), tmpCred) for f in self.endScripts]
        self.role.dataCollectors = [DataCollector(d.GetValue(), tmpCred) for d in self.dataCollectors]
        p = self.view.panel
        
        self.role.count = p.countPicker.GetValue()
        
        #First test to see if it was an AMI
        image = self.dao.getAmi(p.imagePicker.GetValue())
        #otherwise it is simple image
        if image is None:
            image = self.dao.getSourceImage(p.imagePicker.GetValue())
        self.role.image = image
        self.role.instanceType = [i for i in self.role.instanceType.cloud.instanceTypes if i.name == p.instancePicker.GetValue()][0]
        self.dao.save(self.role)
        
        
        
        self.view.EndModal(wx.OK)
        
    def _clearCollections(self, collections):
        for collection in collections:
            for item in collection:
                self.dao.delete(item)
        self.dao.commit()
        
                
    def setupFlexList(self, parent, boxsizer, textControls, fieldCount=1, labels=(), initialValues=(), modifiable=True):
        
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
                for field in evtBtn.fields:
                    textControls.append(field)
                
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
            if self.view.modifiable:    
                if initialVals is None and self.view.modifiable:
                    btn = wx.Button(parent, -1, "Add")
                    btn.Bind(wx.EVT_BUTTON, handleAdd)
                else:
                    btn = wx.Button(parent, -1, "Remove")
                    btn.Bind(wx.EVT_BUTTON, handleRemove)
                    for i,val in enumerate(initialVals):
                        fields[i].SetValue(val)
                        textControls.append(fields[i])
            elif initialVals is not None:
                for i,val in enumerate(initialVals):
                        fields[i].SetValue(val)
                        textControls.append(fields[i])
                
            
            btn.fields = fields
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