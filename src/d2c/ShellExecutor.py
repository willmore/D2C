from logger import StdOutLogger
from subprocess import Popen
import subprocess

class ShellExecutor:

    def __init__(self, logger=StdOutLogger()):
        self.__logger = logger
       
    def run(self, cmd):
        
        self.__logger.write("Executing: " + cmd)    
        
        p = Popen(cmd, shell=True,
                  stdout=subprocess.PIPE, 
                  stderr=subprocess.STDOUT, 
                  close_fds=True)
        
        while True:
            line = p.stdout.readline()
            if not line: break
            self.__logger.write(line)
    
        # This call will block until process finishes and p.returncode is set.
        p.wait()
        
        if 0 != p.returncode:
            raise Exception("Command failed with code %d '" % p.returncode)