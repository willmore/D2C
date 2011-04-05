'''
Created on Mar 10, 2011

@author: willmore
'''

import wx
       
from .RoleList import RoleList

class DeploymentPanel(wx.Panel):    
    
    def __init__(self, deployment, *args, **kwargs):
        wx.Panel.__init__(self, *args, **kwargs)
 
        self.deployment = deployment
        
        wx.StaticText(self, -1, '[DEPLOYMENT]' + deployment.id)
        self.roles = RoleList(self, -1, roles=deployment.roles)