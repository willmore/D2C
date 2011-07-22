import guestfs
import pkg_resources




def setStaticIp(imageFile,IP):
        
        static_ip_file = pkg_resources.resource_filename('test', "virtualbox_xml/static_ip.txt")
        
        file = open(static_ip_file, "r")
        static_ip = file.read()
        file.close()

        static_ip=static_ip.replace('$ip',IP)
        
        print static_ip
        
        gf = guestfs.GuestFS ()
        gf.set_trace(1)
        gf.set_autosync(1)
        
        gf.add_drive(imageFile)    
        gf.launch() 
        
        roots = gf.inspect_os()
        assert (len(roots) == 1) #Only supporting one main partition for now
        rootDev = roots[0]
        gf.mount(rootDev, "/")
        
        gf.write("/etc/network/interfaces", static_ip)
        
        del gf #sync and shutdown
        
        
setStaticIp('/home/sina/VirtualBox VMs/ubuntu1004/ubuntu1004.vdi','192.168.152.123')