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
    
    def run_instances(self, *args, **kwargs):
        pass
    
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
        self.deployment = Deployment(dName, roles = [Role(dName, "loner", ami, 2, 
                                             instances=(Instance(1), Instance(2))), 
                                        Role(dName, "loner2", ami, 2,
                                             instances=(Instance(3), Instance(4)))])
        
    def tearDown(self):
        if hasattr(self, 'mon'):
            self.mon.stop()
    
    def test_getInstances(self):
        
        self.assertEquals(4, len(self.deployment.getInstances()))
    
        
    def testNotRunToRun(self):
        '''
        Test listeners are properly notified when deployment goes to running state.
        '''
        connFactory = DummyConnFactory()
        pollRate = 2
        hits = {}
            
        self.deployment.setEC2ConnFactory(connFactory)
        self.deployment.setMonitorPollRate(pollRate)
        self.deployment.addStateChangeListener(DeploymentState.RUNNING, MicroMock(notify=lambda evt:hits.__setitem__('RUNNING', True)))
        
        self.deployment.run()
        
        time.sleep(2 * pollRate)
        connFactory.setState('running')
        time.sleep(2 * pollRate)
        self.assertTrue(hits.has_key('RUNNING'))
        
        self.deployment.stopMonitoring()
        
if __name__ == '__main__':
    unittest.main()



