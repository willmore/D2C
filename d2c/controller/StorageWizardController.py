import wx


class StorageWizardController:
    
    def __init__(self, view, dao):
        
        self._view = view
        self._dao = dao
         
        self._view.container.getPanel("MAIN").setStores(dao.getImageStores())
        self._view.showPanel("MAIN") 
        
        self._view.container.getPanel("MAIN").addButton.Bind(wx.EVT_BUTTON, self.showPanel("NEW_STORE"))
        self._view.container.getPanel("NEW_STORE").addButton.Bind(wx.EVT_BUTTON, self.addStore)
        
        self._view.container.getPanel("MAIN").doneButton.Bind(wx.EVT_BUTTON, lambda _: self._view.EndModal(wx.ID_OK))
    
    def showPanel(self, label):
        return lambda _: self._view.showPanel(label)
    
    def addStore(self, _):
        
        newStorePanel = self._view.container.getPanel("NEW_STORE")
        try:
            store = newStorePanel.getStore()
        except Exception as x:
            wx.MessageBox(x.message, 'Info')
            return
        
        newStorePanel.clear()
        
        self._dao.addImageStore(store)
        self._view.container.getPanel("MAIN").addStore(store)
        self._view.showPanel("MAIN")