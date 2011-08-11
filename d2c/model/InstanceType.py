
class Architecture(object):
    
    def __init__(self, arch):
        self.arch = arch

'''   
Architecture.X86 = Architecture('x86')
Architecture.X86_64 = Architecture('x86_64')
'''
        
class InstanceType(object):
    '''
    Represents EC2 instance type.
    http://aws.amazon.com/ec2/instance-types/
    '''
    
    def __init__(self, name, cpu, cpuCount, memory, disk, architectures, costPerHour):
        
        self.name = name
        self.cpu = cpu
        self.cpuCount = cpuCount
        self.memory = memory
        self.disk = disk
        self.architectures = list(architectures) if architectures is not None else list()
        self.costPerHour = costPerHour
    
'''        
InstanceType.T1_MICRO = InstanceType('t1.micro', 2, 2, 613, 0, (Architecture.X86, Architecture.X86_64), 0.025)
InstanceType.M1_SMALL = InstanceType('m1.small', 2, 1, 1700, 160, (Architecture.X86,), 0.095)
InstanceType.M1_LARGE = InstanceType('m1.large', 2, 2, 7500, 850, (Architecture.X86_64,), 0.038)
InstanceType.M1_XLARGE = InstanceType('m1.xlarge', 2, 4, 15000, 850, (Architecture.X86_64,), 0.76)


InstanceType.TYPES = {InstanceType.T1_MICRO.name: InstanceType.T1_MICRO, 
                      InstanceType.M1_SMALL.name: InstanceType.M1_SMALL,
                      InstanceType.M1_LARGE.name: InstanceType.M1_LARGE,
                      InstanceType.M1_XLARGE.name: InstanceType.M1_XLARGE}
'''
    