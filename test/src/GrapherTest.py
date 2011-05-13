'''
Created on May 13, 2011

@author: willmore
'''

from d2c.graph.Grapher import Grapher
import wx


grapher = Grapher("/home/willmore/workspace/d2c/experiments/cloud-hpcc/test_4hosts_run2/", "/tmp")

cpuImg = grapher.generateCPUGraphs2("20110510", "20110511")

class MyScrolledWindow(wx.Frame):
   def __init__(self, parent, id, title):
       wx.Frame.__init__(self, parent, id, title, size=(500, 400))

       sw = wx.ScrolledWindow(self)
       bmp = wx.Image(cpuImg,wx.BITMAP_TYPE_PNG).ConvertToBitmap()
       wx.StaticBitmap(sw, -1, bmp)
       sw.SetScrollbars(20,20,55,40)

class MyApp(wx.App):
   def OnInit(self):
       frame = MyScrolledWindow(None, -1, 'CPU')
       frame.Show(True)
       frame.Centre()
       return True

app = MyApp(0)
app.MainLoop()