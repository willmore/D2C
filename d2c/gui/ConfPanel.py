'''
Created on Mar 10, 2011

@author: willmore
'''

import wx
       
class ConfPanel(wx.Panel):    
    
    def __init__(self, parent, id=-1):
        wx.Panel.__init__(self, parent, id)
 
        
        self.ec2_tool_home = wx.TextCtrl(self);
        self.aws_user_id = wx.TextCtrl(self);
        self._aws_key_id = wx.TextCtrl(self);
        self._aws_secret_access_key = wx.TextCtrl(self);
        self._ec2_cert = wx.TextCtrl(self);
        self._ec2_private_key = wx.TextCtrl(self);   
        self._updateButton = wx.Button(self, wx.ID_ANY, 'Save Credentials', size=(130, -1))
        
        
        fgs = wx.FlexGridSizer(7,2,0,0)
        fgs.AddGrowableCol(1, 1)
        
        fgs.AddMany([   (wx.StaticText(self, -1, 'EC2 Tool Home'),0, wx.ALIGN_RIGHT),
                        (self.ec2_tool_home, 0, wx.ALIGN_LEFT|wx.ALIGN_CENTER_VERTICAL|wx.EXPAND),
                        
                        (wx.StaticText(self, -1, 'AWS User ID'),0, wx.ALIGN_RIGHT),
                        (self.aws_user_id, 0, wx.ALIGN_LEFT|wx.ALIGN_CENTER_VERTICAL|wx.EXPAND),
                     
                           (wx.StaticText(self, -1, 'AWS Key ID'),0, wx.ALIGN_RIGHT),
                           (self._aws_key_id, 0, wx.ALIGN_LEFT|wx.ALIGN_CENTER_VERTICAL|wx.EXPAND),
                           
                           (wx.StaticText(self, -1, 'AWS Secret Access Key'),0, wx.ALIGN_RIGHT),
                           (self._aws_secret_access_key,0, wx.ALIGN_LEFT|wx.ALIGN_CENTER_VERTICAL|wx.EXPAND),
                           
                           (wx.StaticText(self, -1, 'EC2 Certificate'),0, wx.ALIGN_RIGHT),
                           (self._ec2_cert, 0, wx.ALIGN_LEFT|wx.ALIGN_CENTER_VERTICAL|wx.EXPAND),
                           
                           (wx.StaticText(self, -1, 'EC2 Private Key'),0, wx.ALIGN_RIGHT),
                           (self._ec2_private_key,0, wx.ALIGN_LEFT|wx.ALIGN_CENTER_VERTICAL|wx.EXPAND),
                            
                           (self._updateButton, 1)])
        
        self.SetSizer(fgs)