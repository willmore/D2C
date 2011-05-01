

class Metric:
    
    def __init__(self, name, unit):
        self.name = name
        self.unit = unit
        
    def __str__(self):
        return "name: %s; unit: %s" % (self.name, self.unit)

class MetricValue:

    def __init__(self, value, time):
        self.value = value
        self.time = time
        
    def __str__(self):
        return "Value: %d; Time: %s" % (self.value, self.time)
    
class MetricList:
    
    def __init__(self, instanceId, metric, values):
        self.instanceId = instanceId
        self.metric = metric
        self.values = sorted(values, key=lambda v : v.time)
        
    def __str__(self):
        out = "Metric = %s\n" % self.metric
        out += "Values = "
        for v in self.values:
            out += "%s\n" % str(v)
        
        return out

class InstanceMetrics:
    
    def __init__(self, instanceId, metricLists):
        self.instanceId = instanceId
        self.metricLists = metricLists
        
    def __str__(self):
        out = "instanceId : " + self.instanceId
        out += "\nmetricLists : \n"
        
        for l in self.metricLists:
            out += "\t%s\n" % l
        
        return out
        
