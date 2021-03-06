import wx

def __activateOnNotEmpty(control, *fields):
    '''
    Iterates through all fields.
    If all fields' values are not empty, control is enabled, else disabled.
    '''
    for f in fields:
        if __empty(f):
            control.Disable()
            return False
           
    control.Enable() 
    return True
    
def __empty(field):
    if isinstance(field, (wx.TextCtrl, wx.ComboBox)):
        return len(field.GetValue()) == 0
    if isinstance(field, wx.ListCtrl):
        return field.GetSelectedItemCount() == 0
    
    return False
    
def createEmptyChecker(control, *fields):
    checker = lambda _: __activateOnNotEmpty(control, *fields)
    for field in fields:
        if isinstance(field, (wx.TextCtrl, wx.ComboBox)):
            field.Bind(wx.EVT_TEXT, checker)
        if isinstance(field, wx.ListCtrl):
            field.Bind(wx.EVT_LIST_ITEM_SELECTED, checker)
            field.Bind(wx.EVT_LIST_ITEM_DESELECTED, checker)
    checker(None)
    