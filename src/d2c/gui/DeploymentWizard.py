
import wx
from ContainerPanel import ContainerPanel
from ConfPanel import ConfPanel


class AMISelectorPanel(wx.Panel):
    def __init__(self, parent, id=-1, size=wx.DefaultSize):
        wx.Panel.__init__(self, parent, id=id, size=size)
        self.sizer = wx.BoxSizer(wx.VERTICAL)
        self.SetSizer(self.sizer)
        
        self.amiList = wx.ListBox(self, -1, wx.DefaultPosition, (170, 130), ("foo","bar","baz"), wx.LB_SINGLE)
        self.sizer.Add(self.amiList, 0, wx.ALL|wx.EXPAND)
        
class RolePanel(wx.Panel):
    def __init__(self, parent, id=-1, size=wx.DefaultSize):
        wx.Panel.__init__(self, parent, id=id, size=size)
        self.sizer = wx.BoxSizer(wx.VERTICAL)
        self.SetSizer(self.sizer)
        
        self.roleList = wx.ListCtrl(self, -1, style=wx.LC_REPORT)
        self.roleList.InsertColumn(0, 'Name')
        self.roleList.InsertColumn(1, 'AMI')
        self.roleList.InsertColumn(2, 'Count')

        self.sizer.Add(self.roleList, 0, wx.ALL|wx.EXPAND)
        
        self.addRoleButton = wx.Button(self, wx.ID_ANY, 'Add New Role', size=(110, -1))
        self.sizer.Add(self.addRoleButton)


class DeploymentWizard(wx.Dialog):
    
    def __init__(self, parent, id, title):
        wx.Dialog.__init__(self, parent, id, title)
        self.container = ContainerPanel(self)
        self.container.addPanel("1", RolePanel(self.container))
        self.container.showPanel("1")

if __name__ == '__main__':

    app = wx.PySimpleApp()  # Start the application

    # Create wizard and add any kind pages you'd like
    mywiz = DeploymentWizard(None, -1, 'Simple Wizard')
    """  page1 = WizardPage(mywiz, 'Page 1')  # Create a first page
    page1.add_stuff(wx.StaticText(page1, -1, 'Hola'))
    mywiz.addPage(page1)

    # Add some more pages
    mywiz.addPage( WizardPage(mywiz, 'Page 2') )
    mywiz.addPage( WizardPage(mywiz, 'Page 3') )
    """
    #mywiz.run() # Show the main window

    # Cleanup
    mywiz.Show()
    app.MainLoop()

    