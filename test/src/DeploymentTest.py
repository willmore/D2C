import unittest
import string
from d2c.logger import StdOutLogger
from d2c.model.Deployment import *
from d2c.model.AMI import AMI
from MicroMock import MicroMock


class DummyConnFactory:
    
    def __init__(self):
        self.conn = DummyConn()
    
    def setState(self, state):
        print "Fack"
        self.conn.setState(state)
    
    def getConnection(self):
        return self.conn
    
class DummyConn:
    
    def __init__(self):
        self.num = 0
        self.instances = []
    
    def get_all_instances(self, ids):
        self.instances = [DummyInstance(x) for x in range(len(ids))]
        return self.instances
    
    def setState(self, state):
        print "Ack %s" % str(self.instances)
        for i in self.instances:
            print "Setting state"
            i.state = state 
    
class DummyInstance():
    
    def __init__(self, ami):
        self.ami = ami
        self.state = 'pending'
        
    def update(self):
        pass

class DeploymentTest(unittest.TestCase):
   
    def setUp(self):
        dName = "Dummy"
        ami = AMI("ami-123", "foobar.vdi")
        self.dummy1 = Deployment(dName, [Role(dName, "loner", ami, 2, 
                                             instances=(Instance(1), Instance(2))), 
                                        Role(dName, "loner2", ami, 2,
                                             instances=(Instance(3), Instance(4)))])
        
    def tearDown(self):
        if hasattr(self, 'mon'):
            self.mon.stop()
   
    def test_getInstances(self):
        
        instances = self.dummy1.getInstances()
        self.assertEquals(4, len(instances))
        
    def test_monitor(self):

        connFactory = DummyConnFactory()

        pollRate = 1
        self.mon = self.dummy1.getMonitor(connFactory, pollRate)
        self.assertNotEqual(None, self.mon)
        
        hits = {}
        
        self.mon.addStateChangeListener(DeploymentState.RUNNING, MicroMock(notify=lambda:hits.__setitem__('RUNNING', True)))
        
        self.mon.start()
        
        time.sleep(2 * pollRate)
        connFactory.setState('running')
        time.sleep(2 * pollRate)
        self.assertTrue(hits.has_key('RUNNING'))
        
if __name__ == '__main__':
    unittest.main()



