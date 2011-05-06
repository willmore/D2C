#http://libguestfs.org/guestfs-python.3.html
import guestfs

#disk = '/usr/lib/guestfs/supermin.d/base.img'
disk = '/home/willmore/Downloads/dsl-4.4.10-x86.vdi'

g = guestfs.GuestFS ()
g.set_qemu('/usr/local/bin/qemu-system-x86_64')
# Attach the disk image read-only to libguestfs.
g.add_drive_opts (disk, readonly=1)
 
# Run the libguestfs back-end.
g.launch ()
 
# Ask libguestfs to inspect for operating systems.
roots = g.inspect_os ()
if len (roots) == 0:
    raise (Exception ("inspect_vm: no operating systems found"))
 
for root in roots:
    print "Root device: %s" % root
 
    # Print basic information about the operating system.
    print "  Product name: %s" % (g.inspect_get_product_name (root))
    print "  Version:      %d.%d" % \
         (g.inspect_get_major_version (root),
          g.inspect_get_minor_version (root))
    print "  Type:         %s" % (g.inspect_get_type (root))
    print "  Distro:       %s" % (g.inspect_get_distro (root))
    
    
for partition in g.list_partitions():
    print "Partition: %s" % partition
    
