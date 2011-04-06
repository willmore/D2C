import unittest
import string
from d2c.logger import StdOutLogger
from d2c.model.Deployment import *
from d2c.model.AMI import AMI


class DeploymentTest(unittest.TestCase):
   
    def test_getInstances(self):
        dName = "Dummy"
        ami = AMI("ami-123", "foobar.vdi")
        deployment = Deployment(dName, [Role(dName, "loner", ami, 2, 
                                             instances=(Instance(1), Instance(2))), 
                                        Role(dName, "loner2", ami, 2,
                                             instances=(Instance(3), Instance(4)))])
        instances = deployment.getInstances()
        self.assertEquals(4, len(instances))

if __name__ == '__main__':
    unittest.main()



