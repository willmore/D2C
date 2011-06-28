import wx

class MyFrame(wx.Frame):
   def __init__(self, parent, ID, title):
       wx.Frame.__init__(self, parent, ID, title, size=(300, 250))

       panel1 = wx.Panel(self,-1, style=wx.SUNKEN_BORDER)
       panel2 = wx.Panel(self,-1, style=wx.SUNKEN_BORDER)

       panel1.SetBackgroundColour("BLUE")
       panel2.SetBackgroundColour("RED")

       box = wx.BoxSizer(wx.VERTICAL)
       box.Add(panel1, 2, wx.EXPAND)
       box.Add(panel2, 1, wx.EXPAND)

       self.SetAutoLayout(True)
       self.SetSizer(box)
       self.Layout()


app = wx.PySimpleApp()
frame = MyFrame(None, -1, "Sizer Test")
frame.Show()
app.MainLoop()