import wx


class CompCloudConfController:
    
    def __init__(self, view, dao):
        
        self._view = view
        self._dao = dao
         
        self._view.compCloudConfPanel.setRegions(dao.getRegions())
        self._view.showPanel("MAIN") 
        
        self._view.container.getPanel("MAIN").addButton.Bind(wx.EVT_BUTTON, self.showPanel("NEW_CLOUD"))
        self._view.container.getPanel("NEW_CLOUD").addButton.Bind(wx.EVT_BUTTON, self.addCloud)
    
    def showPanel(self, label):
        return lambda _: self._view.showPanel(label)
    
    def addCloud(self, _):
        
        newCloudPanel = self._view.container.getPanel("NEW_CLOUD")
        try:
            region = newCloudPanel.getRegion()
        except Exception as x:
            wx.MessageBox(x.message, 'Info')
            return
        
        newCloudPanel.clear()
        
        self._dao.addRegion(region)
        self._view.container.getPanel("MAIN").addRegion(region)
        self._view.showPanel("MAIN")