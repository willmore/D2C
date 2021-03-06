'''
Created on May 13, 2011

@author: willmore
'''

from d2c.graph.Grapher import Grapher
import wx


#grapher = Grapher("/home/willmore/workspace/d2c/experiments/cloud-hpcc/test_4hosts_run2/", "/tmp")

#grapher = Grapher({"vm":"/home/willmore/D2C-Experiments/cloud-hpcc/local/single_vm_1proc/collectd/vm0"}, "/tmp")


grapher = Grapher({"vm":"/home/willmore/.d2c/deployments/22.py/QCW7N1/31/i-4B450863/opt/collectd/var/lib/collectd/dirac.lan/",
                   "vm2":"/home/willmore/.d2c/deployments/22.py/QCW7N1/32/i-468F08BD/opt/collectd/var/lib/collectd/dirac.lan/"
                   
                   
                   
                   }, "/tmp")

cpuImg = grapher.generateCPUGraphsAverage("20110929 00:00", "20110930 23:00")
memImg = grapher.generateMemoryGraph("20110929 00:00", "20110930 23:00")
#netImg = grapher.generateNetworkGraph("20110929 00:00", "20110929 23:00")

class MyScrolledWindow(wx.Frame):
   def __init__(self, parent, id, title):
       wx.Frame.__init__(self, parent, id, title, size=(500, 400))

       sizer = wx.BoxSizer(wx.VERTICAL)
       sw = wx.ScrolledWindow(self)
       bmp = wx.Image(cpuImg,wx.BITMAP_TYPE_PNG).ConvertToBitmap()
       
       hsizer = wx.BoxSizer(wx.HORIZONTAL)
       hsizer.Add(wx.StaticBitmap(sw, -1, bmp))
       sizer.Add(hsizer)
       
       bmp = wx.Image(memImg,wx.BITMAP_TYPE_PNG).ConvertToBitmap()
       hsizer = wx.BoxSizer(wx.HORIZONTAL)
       hsizer.Add(wx.StaticBitmap(sw, -1, bmp))
       sizer.Add(hsizer)
       '''
       bmp = wx.Image(netImg,wx.BITMAP_TYPE_PNG).ConvertToBitmap()
       hsizer = wx.BoxSizer(wx.HORIZONTAL)
       hsizer.Add(wx.StaticBitmap(sw, -1, bmp))
       sizer.Add(hsizer)
       '''
       sw.SetScrollbars(20,20,55,40)
       sw.SetSizer(sizer)

class MyApp(wx.App):
   def OnInit(self):
       frame = MyScrolledWindow(None, -1, 'CPU')
       frame.Show(True)
       frame.Centre()
       return True

app = MyApp(0)
app.MainLoop()