
import wx
from .ItemList import ColumnMapper, ItemList
from .ContainerPanel import ContainerPanel

class AMIPanel(wx.Panel):    
    
    def __init__(self, *args):
        wx.Panel.__init__(self, *args)
        self.splitter = wx.SplitterWindow(self, -1)
        self.splitter.SetMinimumPaneSize(150)
        vbox = wx.BoxSizer(wx.VERTICAL)

        self.list = ItemList(self.splitter, -1, style=wx.LC_REPORT,
                                 mappers=[ColumnMapper('AMI', lambda r: r.id, defaultWidth=wx.LIST_AUTOSIZE),
                                          ColumnMapper('SourceImage', lambda r: r.srcImg.path, defaultWidth=wx.LIST_AUTOSIZE),
                                          ColumnMapper('Status', lambda r: r.status, defaultWidth=wx.LIST_AUTOSIZE_USEHEADER),
                                          ColumnMapper('Cloud', lambda r: r.cloud.name, defaultWidth=wx.LIST_AUTOSIZE)]
                                 )

        vbox.Add(self.splitter, 1, wx.EXPAND)
        
        self._logPanel = ContainerPanel(self.splitter, -1)
   
        self.splitter.SplitHorizontally(self.list, self._logPanel)
        self.SetSizer(vbox)
        self.Layout()   

         
    def addLogPanel(self, id):
        self._logPanel.addPanel(id, wx.TextCtrl(self._logPanel, -1, size=(100,100), style=wx.TE_MULTILINE )) #style=wx.TE_READONLY
        
    def showLogPanel(self, id):
        self._logPanel.showPanel(id)
        self.Layout()
        self.Refresh()
        
    def appendLogPanelText(self, logPanelId, text):
        self._logPanel.getPanel(logPanelId).AppendText(str(text) + "\n")
        
    def addAMIEntry(self, ami):
        self.list.addItem(ami)
        
    def setAMIs(self, images): 
        self.list.setItems(images)
