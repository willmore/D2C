#! /bin/bash
src=/var/lib/collectd/rrd/cloud-hpcc

for i in ec2-46-137-38-14 ec2-46-137-37-134 ec2-46-137-21-152 ec2-46-137-49-253
do
	scp -r -i ~/.ssh/id_rsa_nopw hpcc-user@$i.eu-west-1.compute.amazonaws.com:$src $i
done
