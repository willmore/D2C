
from d2c.model.Cloud import DesktopCloud
from d2c.model.SourceImage import DesktopImage, Image

import unittest

class ImageTest(unittest.TestCase):
    
    def testGetHostedImage(self):
        
        cloud = DesktopCloud(None, "MyCloud")
        path = "/foo/bar.vdi"
        image = Image(None, "TestImage", DesktopImage(None, None, cloud, path))
        
        srcImg = image.reals[0]
        self.assertEqual(srcImg.path, path)
        
        self.assertEqual(srcImg, image.getHostedSourceImage(cloud))
    



