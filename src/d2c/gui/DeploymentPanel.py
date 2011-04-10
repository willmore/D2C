'''
Created on Mar 10, 2011

@author: willmore
'''

import wx
       
from .RoleList import RoleList

class DeploymentPanel(wx.Panel):    
    
    def __init__(self, deployment, *args, **kwargs):
        wx.Panel.__init__(self, *args, **kwargs)
        
        self.SetSizer(wx.BoxSizer(wx.VERTICAL))
        
        self.deployment = deployment
        
        label = wx.StaticText(self, -1, deployment.id)
        label.SetFont(wx.Font(20, wx.DEFAULT, wx.DEFAULT, wx.BOLD))
        self.GetSizer().Add(label, 0, wx.BOTTOM, 10)
        
        self.statusSizer = wx.BoxSizer(wx.HORIZONTAL)
        self.GetSizer().Add(self.statusSizer, 0, wx.BOTTOM, 5)
        label = wx.StaticText(self, -1, 'Status')
        label.SetFont(wx.Font(wx.DEFAULT, wx.DEFAULT, wx.DEFAULT, wx.BOLD))
        self.statusSizer.Add(label, 0, wx.RIGHT, 5)
        self.statusField = wx.StaticText(self, -1, deployment.state)
        self.statusSizer.Add(self.statusField, 0, wx.ALIGN_CENTER)
        
        
        label = wx.StaticText(self, -1, 'Roles')
        label.SetFont(wx.Font(wx.DEFAULT, wx.DEFAULT, wx.DEFAULT, wx.BOLD))
        self.GetSizer().Add(label, 0, wx.BOTTOM, 5)
        self.roles = RoleList(self, -1, roles=deployment.roles)
        self.GetSizer().Add(self.roles, 0, wx.BOTTOM | wx.EXPAND, 5)
        
        self.deployButton = wx.Button(self, wx.ID_ANY, 'Deploy', size=(110, -1))
        self.GetSizer().Add(self.deployButton)
        