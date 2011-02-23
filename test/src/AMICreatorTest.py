import unittest
import d2c.AMICreator

class AMICreatorTest(unittest.TestCase):


    def test_extractMainPartition(self):
        fullImg = "../data/test.img"
        partImg = "/tmp/test.img"
        
        d2c.AMICreator.extractMainPartition(fullImg, partImg)        

if __name__ == '__main__':
    unittest.main()
