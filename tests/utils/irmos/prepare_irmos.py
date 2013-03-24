#!/usr/bin/python

import os
import sys
import subprocess

#Check for root access
if os.geteuid() != 0:
	print 'You must be root to run IRMOS tests!'
	sys.exit()

#Check command line arguments
if len(sys.argv) != 2:
	print 'Usage: sudo ./prepare_irmos.py <group>'
	print 'Note: the cgroup will be mounted in "/cg"'
	sys.exit()

cgroup_name = sys.argv[1]

#Mount the initial cgroup, if it isn't already mounted
if not os.path.exists('/cg'):
	os.mkdir('/cg')
	subprocess.Popen(['mount','-t','cgroup','-o','cpu,cpuacct','cgroup','/cg'], shell=True).wait()

#Reduce the runtime of all other tasks and make the IRMOS cgroup
#subprocess.Popen(['echo','100000','>','/cg/cpu.rt_task_runtime_us'], shell=False).wait()
#os.system('echo 100000 > /cg/cpu.rt_task_runtime_us')
os.system('echo 0 > /cg/cpu.rt_task_runtime_us')
cg_param_dir = '/cg/%s/' % cgroup_name
if not os.path.exists(cg_param_dir):
	os.mkdir(cg_param_dir)

#Set the other processes
os.system('echo 1000000 > %scpu.rt_period_us' % cg_param_dir)
#os.system('echo 500000 > %scpu.rt_runtime_us' % cg_param_dir)
os.system('echo 1000000 > %scpu.rt_runtime_us' % cg_param_dir)
os.system('echo 1000000 > %scpu.rt_task_period_us' % cg_param_dir)
#os.system('echo 500000 > %scpu.rt_task_runtime_us' % cg_param_dir)
os.system('echo 1000000 > %scpu.rt_task_runtime_us' % cg_param_dir)

print 'Mounted cgroup %s' % cg_param_dir
