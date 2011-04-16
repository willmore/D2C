from d2c.data.CredStore import CredStore
from TestConfig import TestConfig
from d2c.data.DAO import DAO
from d2c.CloudWatchConnectionFactory import CloudWatchConnectionFactory
from d2c.data.CloudWatchClient import CloudWatchClient
import datetime
import os
from mockito import *

DAO._SQLITE_FILE = "%s/.d2c_test/metrics_test.sqlite" % os.path.expanduser('~') 
if os.path.exists(DAO._SQLITE_FILE):
    print "Deleting existing DB"
    os.unlink(DAO._SQLITE_FILE)

dao = DAO()
conf = TestConfig("/home/willmore/test.conf")

dao.saveConfiguration(conf)
client = CloudWatchClient(CloudWatchConnectionFactory(CredStore(dao)), dao)

start = datetime.datetime(2011, 4, 13, 0, 0, 0)
end = datetime.datetime(2011, 4, 13, 21, 35, 0)

iid = 'i-0351ff75'
dao.getMetrics()
metrics = client.getInstanceMetrics(iid, start, end)

dao.saveInstanceMetrics(metrics)
print dao.getInstanceMetrics(iid)