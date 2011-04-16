
from d2c.data.InstanceMetrics import Metric, MetricValue, MetricList, InstanceMetrics

class CloudWatchClient:


    def __init__(self, cwConnFactory, dao):
        
        self.cwConnFactory = cwConnFactory
        self.dao = dao
        
    
    def getInstanceMetrics(self, instanceId, startTime, endTime):    
        conn = self.cwConnFactory.getConnection()
    
        mets = []
        
        for metric in self.dao.getMetrics():
            mResp = conn.get_metric_statistics(period=300, 
                                       start_time=startTime, 
                                       end_time=endTime, 
                                       metric_name=metric.name, 
                                       namespace="AWS/EC2", 
                                       statistics='Sum', 
                                       dimensions={"InstanceId":instanceId}, 
                                       unit=metric.unit)
            mets.append(self.__map(instanceId, metric, mResp))
          
        return InstanceMetrics(instanceId, mets)
                
    def __map(self, instanceId, metric, mResp):
        '''
        Map a metric api response to a metric list
        '''
        values = []
        for slice in mResp:
            values.append(MetricValue(slice['Sum'], slice['Timestamp']))
        
        return MetricList(instanceId, metric, values)
            
            
            