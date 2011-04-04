'''
Created on Mar 10, 2011

@author: willmore
'''

import wx
       
class DeploymentPanel(wx.Panel):    
    
    def __init__(self, deployment, *args, **kwargs):
        wx.Panel.__init__(self, *args, **kwargs)
 
        self.deployment = deployment
        
        wx.StaticText(self, -1, '[DEPLOYMENT]' + deployment.id)