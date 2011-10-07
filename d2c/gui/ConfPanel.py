from .ContainerPanel import ContainerPanel
from d2c.model.CloudCred import CloudCred
from d2c.model.AWSCred import AWSCred
from d2c.model.EC2Cred import EC2Cred

import wx
import random
import string

class CredDialog(wx.Dialog):
    
    def __init__(self, dao, *args, **kwargs):

        wx.Dialog.__init__(self, *args, **kwargs)
                
        self.dao = dao
        
        self.splitter = wx.SplitterWindow(self, -1)
        self.splitter.SetMinimumPaneSize(150)
        vbox = wx.BoxSizer(wx.VERTICAL)

        self.list = wx.ListCtrl(self.splitter, -1)
        self.list.InsertColumn(0, 'Name')
        vbox.Add(self.splitter, 1, wx.EXPAND)
        
        self.displayPanel = ContainerPanel(self.splitter, -1)
        
        self.splitter.SplitVertically(self.list, self.displayPanel)
        
        self.bottomPanel = wx.Panel(self, -1)
        self.bottomPanel.vbox = wx.BoxSizer(wx.VERTICAL)
        self.addButton = wx.Button(self.bottomPanel, -1, 'Add Credential')
        self.bottomPanel.vbox.Add(self.addButton, 0, wx.ALL, 5)
        self.bottomPanel.SetSizer(self.bottomPanel.vbox)
        self.bottomPanel.SetBackgroundColour('GREY')
        
        self.closeButton = wx.Button(self.bottomPanel, wx.ID_ANY, 'Close')
        self.bottomPanel.vbox.Add(self.closeButton, 0, wx.ALL, 5)
        
        vbox.Add(self.bottomPanel, 0, wx.EXPAND)
        
        self.SetSizer(vbox)
        self.loadCreds()
        self.Layout()   
                    
    def close(self):
        self.Close()
    
    def selectCred(self, evt):
        pass
    
    def addCred(self):
        self.addCredPanel(CredPanel(self.dao, CloudCred(name="New Credential"), self.displayPanel, -1))
    
    def loadCreds(self):
        for cred in self.dao.getCloudCreds():
            self.addCredPanel(CredPanel(self.dao, cred, self.displayPanel, -1))
        
    def addCredPanel(self, credPanel):
        n = self.list.GetItemCount()
        self.list.InsertStringItem(n, credPanel.cloudCred.name)
        self.displayPanel.addPanel(credPanel.cloudCred.name, credPanel)
        self.displayPanel.showPanel(credPanel.cloudCred.name)
        
       
class CredPanel(wx.Panel):    
    
    def __init__(self, dao, cloudCred, *args, **kwargs):
        wx.Panel.__init__(self, *args, **kwargs)
 
 
        self.dao = dao
        #self.Title = "Credential Configuration Panel"
        self.cloudCred = cloudCred
        self.name = wx.TextCtrl(self)
        self.name.SetValue(cloudCred.name)
        self.aws_user_id = wx.TextCtrl(self)
        self._aws_key_id = wx.TextCtrl(self)
        self._aws_secret_access_key = wx.TextCtrl(self)
        self._ec2_cert = wx.TextCtrl(self)
        self._ec2_private_key = wx.TextCtrl(self)  
        self.saveButton = wx.Button(self, wx.ID_ANY, 'Save')
        self.saveButton.Bind(wx.EVT_BUTTON, self._onSave)
        
        if self.cloudCred.awsUserId is not None:
            self.aws_user_id.SetValue(self.cloudCred.awsUserId)
        
        if self.cloudCred.awsCred is not None:
            self._aws_key_id.SetValue(self.cloudCred.awsCred.access_key_id)
            self._aws_secret_access_key.SetValue(self.cloudCred.awsCred.secret_access_key)
            
        if self.cloudCred.ec2Cred is not None:
            self._ec2_cert.SetValue(self.cloudCred.ec2Cred.cert)
            self._ec2_private_key.SetValue(self.cloudCred.ec2Cred.private_key)
        
        self._hsizer = wx.BoxSizer(wx.HORIZONTAL)
        self._hsizer.Add(self.saveButton)
        self.saveButton.Bind(wx.EVT_BUTTON, self._onSave)
        
        fgs = wx.FlexGridSizer(7,2,0,0)
        fgs.AddGrowableCol(1, 1)
        
        fgs.AddMany([   
                        (wx.StaticText(self, -1, 'Name'),0, wx.ALIGN_CENTER_VERTICAL|wx.ALIGN_RIGHT|wx.ALL, 2),
                        (self.name, 0, wx.ALIGN_LEFT|wx.ALIGN_CENTER_VERTICAL|wx.EXPAND|wx.ALL, 2),
                     
                        (wx.StaticText(self, -1, 'AWS User ID'),0, wx.ALIGN_CENTER_VERTICAL|wx.ALIGN_RIGHT|wx.ALL, 2),
                        (self.aws_user_id, 0, wx.ALIGN_LEFT|wx.ALIGN_CENTER_VERTICAL|wx.EXPAND|wx.ALL, 2),
                     
                           (wx.StaticText(self, -1, 'AWS Key ID'),0, wx.ALIGN_CENTER_VERTICAL|wx.ALIGN_RIGHT|wx.ALL, 2),
                           (self._aws_key_id, 0, wx.ALIGN_LEFT|wx.ALIGN_CENTER_VERTICAL|wx.EXPAND|wx.ALL, 2),
                           
                           (wx.StaticText(self, -1, 'AWS Secret Access Key'),0, wx.ALIGN_CENTER_VERTICAL|wx.ALIGN_RIGHT|wx.ALL, 2),
                           (self._aws_secret_access_key,0, wx.ALIGN_LEFT|wx.ALIGN_CENTER_VERTICAL|wx.EXPAND|wx.ALL, 2),
                           
                           (wx.StaticText(self, -1, 'EC2 Certificate'),0, wx.ALIGN_CENTER_VERTICAL|wx.ALIGN_RIGHT|wx.ALL, 2),
                           (self._ec2_cert, 0, wx.ALIGN_LEFT|wx.ALIGN_CENTER_VERTICAL|wx.EXPAND|wx.ALL, 2),
                           
                           (wx.StaticText(self, -1, 'EC2 Private Key'),0, wx.ALIGN_CENTER_VERTICAL|wx.ALIGN_RIGHT|wx.ALL, 2),
                           (self._ec2_private_key,0, wx.ALIGN_LEFT|wx.ALIGN_CENTER_VERTICAL|wx.EXPAND|wx.ALL, 2),
                          
                           (wx.StaticText(self), wx.EXPAND), 
                           (self._hsizer, 1, wx.ALIGN_RIGHT|wx.ALL, 5)])
        
        self.SetSizer(fgs)
        
    def _onSave(self, _):
        print "Save"
        self.cloudCred.name = self.name.GetValue()
        self.cloudCred.awsUserId = self.aws_user_id.GetValue()
        
        if (self.cloudCred.awsCred is None and self.cloudCred.ec2Cred is None):
            name = "".join(random.choice(string.ascii_uppercase + string.digits) for _ in range(6))
            self.cloudCred.awsCred = AWSCred(name, self._aws_key_id.GetValue(), self._aws_secret_access_key.GetValue())
            
            name = "".join(random.choice(string.ascii_uppercase + string.digits) for _ in range(6))
            self.cloudCred.ec2Cred = EC2Cred(name, self._ec2_cert.GetValue(), self._ec2_private_key.GetValue())
        
        else:
            self.cloudCred.awsCred.access_key_id = self._aws_key_id.GetValue()
            self.cloudCred.awsCred.secret_access_key = self._aws_secret_access_key.GetValue()

            self.cloudCred.ec2Cred.cert = self._ec2_cert.GetValue()
            self.cloudCred.ec2Cred.private_key = self._ec2_private_key.GetValue()
        
        self.dao.add(self.cloudCred)
        
        
        
    