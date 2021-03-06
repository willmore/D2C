
import wx
from .ItemList import ColumnMapper, ItemList

class RoleList(ItemList):
    
    def __init__(self, *args, **kwargs):
        
        kwargs['mappers'] = [ColumnMapper('Name', lambda r: r.template.name, defaultWidth=wx.LIST_AUTOSIZE),
                             ColumnMapper('Image', lambda r: r.image.getDisplayName(), defaultWidth=wx.LIST_AUTOSIZE),
                             ColumnMapper('Count', lambda r: r.count, defaultWidth=wx.LIST_AUTOSIZE_USEHEADER),
                             ColumnMapper('Instance Type', lambda r: '' if r.instanceType is None else r.instanceType.name, defaultWidth=wx.LIST_AUTOSIZE_USEHEADER),
                             ColumnMapper('Output Data', lambda r: 'Click', defaultWidth=wx.LIST_AUTOSIZE_USEHEADER)]
        
        kwargs['style'] =wx.LC_REPORT
        ItemList.__init__(self, *args, **kwargs)
        
class RoleTemplateList(ItemList):
    
    def __init__(self, *args, **kwargs):
        
        kwargs['mappers'] = [ColumnMapper('Name', lambda r: r.name, defaultWidth=wx.LIST_AUTOSIZE)]
        kwargs['style'] =wx.LC_REPORT
       
        ItemList.__init__(self, *args, **kwargs)
        
        
                   
    