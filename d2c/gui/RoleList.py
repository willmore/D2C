'''
Created on Mar 16, 2011

@author: willmore
'''
import wx
from .ItemList import ColumnMapper, ItemList

class RoleList(ItemList):
    
    def __init__(self, *args, **kwargs):
        
        kwargs['mappers'] = [ColumnMapper('Name', lambda r: r.name, defaultWidth=50),
                             ColumnMapper('AMI', lambda r: r.ami.id, defaultWidth=100),
                             ColumnMapper('Count', lambda r: r.count, defaultWidth=50),
                             ColumnMapper('Instance Type', lambda r: r.instanceType.name, defaultWidth=125)]
        
        kwargs['style'] =wx.LC_REPORT
       
        ItemList.__init__(self, *args, **kwargs)
        
        
                   
    