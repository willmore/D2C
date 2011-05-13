'''
Created on May 13, 2011

@author: willmore
'''
import sys
import rrdtool, tempfile

DAY = 86400
YEAR = 365 * DAY
fd,path = tempfile.mkstemp('.png')
rrdCpu = "/home/willmore/workspace/d2c/experiments/cloud-hpcc/test_4hosts_run2/ec2-46-137-21-152/cpu-%d/cpu-user.rrd"

rrdtool.graph(path,
             '--imgformat', 'PNG',
             '--width', '540',
             '--height', '100',
             '--start', "20110510",
             '--end', "20110511",
             '--vertical-label', 'CPU',
             '--title', 'CPU',
             '--lower-limit', '0',
             'DEF:cpu%d=/home/willmore/workspace/d2c/experiments/cloud-hpcc/test_4hosts_run2/ec2-46-137-21-152/cpu-%d/cpu-user.rrd:value:AVERAGE' % (1,1),
             'DEF:cpu%d=/home/willmore/workspace/d2c/experiments/cloud-hpcc/test_4hosts_run2/ec2-46-137-21-152/cpu-%d/cpu-user.rrd:value:AVERAGE' % (2,2),
             'DEF:cpu%d=/home/willmore/workspace/d2c/experiments/cloud-hpcc/test_4hosts_run2/ec2-46-137-21-152/cpu-%d/cpu-user.rrd:value:AVERAGE' % (3,3),
             'DEF:cpu%d=/home/willmore/workspace/d2c/experiments/cloud-hpcc/test_4hosts_run2/ec2-46-137-21-152/cpu-%d/cpu-user.rrd:value:AVERAGE' % (0,0),
             "LINE1:cpu0#0000FF:cpu0",
             "LINE2:cpu1#00FF00:cpu1",
             "LINE3:cpu2#FF0000:cpu2",
             "LINE4:cpu3#00FFFF:cpu3")

print "new image is %s" % path

import wx

class MyScrolledWindow(wx.Frame):
   def __init__(self, parent, id, title):
       wx.Frame.__init__(self, parent, id, title, size=(500, 400))

       sw = wx.ScrolledWindow(self)
       bmp = wx.Image(path,wx.BITMAP_TYPE_PNG).ConvertToBitmap()
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

#info = rrdtool.info('downloads.rrd')
#print info['last_update']
#print info['ds[downloads].minimal_heartbeat']


