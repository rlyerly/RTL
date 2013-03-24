#!/usr/bin/python

import os
import sys
import subprocess

#Check for root access
if os.geteuid() != 0:
	print 'You must be root to attach a task to the IRMOS group!'
	sys.exit()

if len(sys.argv) != 3:
	print 'Usage: sudo ./attach_task.py <group name> <process>'
	sys.exit()

cgroup_name = sys.argv[1]
process = sys.argv[2]

#Make sure cgroup already exists
if not os.path.exists('/cg/%s' % cgroup_name):
	print 'IRMOS group %s isn\'t mounted...did you mount it with prepare_irmos.py?' % cgroup_name
	sys.exit()

#Run the program and attach it to the group
group = '/cg/%s/' % cgroup_name
program = subprocess.Popen(process, shell=True)
os.system('echo ' + str(program.pid) + ' > %s/tasks' % group)
program.wait()
