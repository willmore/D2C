'''
Created on Mar 16, 2011

@author: willmore
'''
import wx

class AMIList(wx.ListCtrl):
    
    def __init__(self, *args, **kwargs):
        wx.ListCtrl.__init__(self, *args, **kwargs)
        
        self.InsertColumn(0, 'AMI', width=75)
        self.InsertColumn(1, 'Source Image', width=200)
        self.InsertColumn(2, 'Status', width=110)
        self.InsertColumn(3, 'Created', width=110)
        
        self.amis = {}
        
        if kwargs.has_key('amis'):
            self.setAMIs(kwargs['amis'])
        
    def addAMIEntry(self, ami):
        idx = self.Append((ami.amiId,ami.srcImg))
        self.amis[idx] = ami
    
    def setAMIs(self, amis): 
        self.DeleteAllItems()
        self.amis.clear()
        
        for ami in amis:
            self.addAMIEntry(ami)
            
    def getSelectedAMIs(self):
        #TODO return more than one ami if selected
        i = self.GetFirstSelected();
        
        if i < 0:
            return []
        else:
            return [self.amis[i]]