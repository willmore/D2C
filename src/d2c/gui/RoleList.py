'''
Created on Mar 16, 2011

@author: willmore
'''
import wx

class RoleList(wx.ListCtrl):
    
    def __init__(self, *args, **kwargs):
        
        kwargs['style'] =wx.LC_REPORT
        self.roles = {}
        roles = None
        if kwargs.has_key('roles'):
            roles = kwargs['roles']
            del kwargs['roles']
        
        wx.ListCtrl.__init__(self, *args, **kwargs)
        
        self.InsertColumn(0, 'Name', width=75)
        self.InsertColumn(1, 'AMI', width=75)
        self.InsertColumn(2, 'Count', width=75)
        
        if roles is not None:
            self.setRoles(roles) 
                   
        
    def addRole(self, role):
        idx = self.Append((role.name,role.ami.amiId, role.count))
        self.roles[idx] = role
    
    def setRoles(self, roles): 
        self.DeleteAllItems()
        self.roles.clear()
        
        for role in roles:
            self.addRole(role)
            
    def getRoles(self):
        return self.roles.values()
    
    def getSelectedRoles(self):
        #TODO return more than one if selected
        i = self.GetFirstSelected();
        
        if i < 0:
            return []
        else:
            return self.roles[i]