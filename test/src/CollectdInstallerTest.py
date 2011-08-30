'''
Created on Aug 29, 2011

@author: willmore
'''

from d2c.model.SourceImage import DesktopImage
from d2c.model.CollectdInstaller import CollectdInstaller
from d2c.model.Cloud import DesktopCloud

cloud = DesktopCloud(0, 'None', list())
img = DesktopImage(None, None, cloud, "/home/willmore/images/worker.vdi")
installer = CollectdInstaller()
installer.install(img)