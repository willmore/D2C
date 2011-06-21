'''
Created on Mar 10, 2011

@author: willmore
'''

import wx
       
class ConfPanel(wx.Dialog):    
    
    def __init__(self, *args, **kwargs):
        wx.Dialog.__init__(self, *args, **kwargs)
 
        self.Title = "Amazon Configuration Panel"
        self.Size = (400,170)
 
        self.aws_user_id = wx.TextCtrl(self);
        self._aws_key_id = wx.TextCtrl(self);
        self._aws_secret_access_key = wx.TextCtrl(self);
        self._ec2_cert = wx.TextCtrl(self);
        self._ec2_private_key = wx.TextCtrl(self);   
        self._updateButton = wx.Button(self, wx.ID_ANY, 'Save Credentials')
        self._closeButton = wx.Button(self, wx.ID_ANY, 'Close')
        self._hsizer = wx.BoxSizer(wx.HORIZONTAL)
        self._hsizer.Add(self._updateButton,1,wx.RIGHT,5)
        self._hsizer.Add(self._closeButton,0,wx.RIGHT,3)
        
        
        fgs = wx.FlexGridSizer(7,2,0,0)
        fgs.AddGrowableCol(1, 1)
        
        fgs.AddMany([   (wx.StaticText(self, -1, 'AWS User ID'),0, wx.ALIGN_CENTER_VERTICAL|wx.ALIGN_RIGHT|wx.ALL, 2),
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
        
        
        
    def close(self):
        self.Close()