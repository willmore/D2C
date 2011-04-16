
class Architecture:
    
    def __init__(self, arch):
        self.arch = arch
        
Architecture.X86 = Architecture('x86')
Architecture.X86_64 = Architecture('x86_64')

class InstanceType:
    '''
    Represents EC2 instance type.
    http://aws.amazon.com/ec2/instance-types/
    '''
    
    def __init__(self, name, cpu, memory, disk, architectures):
        
        self.name = name
        self.cpu = cpu
        self.memory = memory
        self.disk = disk
        self.architectures = architectures
    
        
InstanceType.T1_MICRO = InstanceType('t1.micro', 2, 613, 0, (Architecture.X86, Architecture.X86_64))
InstanceType.M1_SMALL = InstanceType('m1.small', 1, 1700, 160, (Architecture.X86))
    