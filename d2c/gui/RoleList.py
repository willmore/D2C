'''
Created on Mar 16, 2011

@author: willmore
'''
import wx
from .ItemList import ColumnMapper, ItemList

class RoleList(ItemList):
    
    def __init__(self, *args, **kwargs):
        
        kwargs['mappers'] = [ColumnMapper('Name', lambda r: r.name, defaultWidth=wx.LIST_AUTOSIZE),
                             ColumnMapper('AMI', lambda r: r.ami.id, defaultWidth=wx.LIST_AUTOSIZE),
                             ColumnMapper('Count', lambda r: r.count, defaultWidth=wx.LIST_AUTOSIZE_USEHEADER),
                             ColumnMapper('Instance Type', lambda r: r.instanceType.name, defaultWidth=wx.LIST_AUTOSIZE_USEHEADER)]
        
        kwargs['style'] =wx.LC_REPORT
       
        ItemList.__init__(self, *args, **kwargs)
        
        
                   
    