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
        self.conn.setState(state)
    
    def getConnection(self):
        return self.conn
    
class DummyConn:
    
    def __init__(self):
        self.num = 0
        self.instances = []
        self.reservations = {}
    
    def get_all_instances(self, ids = None, filters={}):

        return filter(lambda r: r.id in filters['reservation-id'], 
                      self.reservations.values())
    
    def run_instances(self, *args, **kwargs):
        r = DummyReservation(kwargs['min_count'])
        self.reservations[r.id] = r
        return r
    
    def setState(self, state):
        for i in self.instances:
            print "Setting state"
            i.state = state 
        
        for r in self.reservations.values():
            r.setState(state)
            
class DummyReservation:
    
    ctr = 0
    
    def __init__(self, count):
        self.instances = [DummyInstance(None) for _ in range(count)]
        self.id = 'r-dummy_%d' % self.ctr 
        self.ctr += self.ctr
    
    def setState(self, state):
        for i in self.instances:
            print "Setting state"
            i.state = state 
        
    def update(self):
        pass

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
        self.deployment = Deployment(dName, roles = [Role(dName, "loner", ami, 2), 
                                        Role(dName, "loner2", ami, 2)])
        
    def tearDown(self):
        if hasattr(self, 'mon'):
            self.mon.stop()
    '''
    def test_getInstances(self):
        
        self.assertEquals(4, len(self.deployment.getInstances()))
    '''
        
    def testNotRunToRun(self):
        '''
        Test listeners are properly notified when deployment goes to running state.
        '''
        connFactory = DummyConnFactory()
        pollRate = 2
        hits = {}
            
        self.deployment.setEC2ConnFactory(connFactory)
        self.deployment.addStateChangeListener(DeploymentState.INSTANCES_LAUNCHED, 
                                               MicroMock(notify=lambda evt:hits.__setitem__('INSTANCES_LAUNCHED', True)))
        self.deployment.setPollRate(pollRate)
        
        self.deployment.start()
        
        time.sleep(2 * pollRate)
        connFactory.setState('running')
        time.sleep(2 * pollRate)
        self.assertTrue(hits.has_key('INSTANCES_LAUNCHED'))
        
        self.deployment.stop()
        
if __name__ == '__main__':
    unittest.main()



