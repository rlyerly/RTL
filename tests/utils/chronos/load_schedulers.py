#!/usr/bin/python

import os
import sys
import subprocess

#Check for root access
if os.geteuid() != 0:
	print 'You must be root to load the modules!'
	sys.exit()

#Load all the ChronOS schedulers
subprocess.Popen(['modprobe','fifo_ra'], shell=False).wait()
subprocess.Popen(['modprobe','rma'], shell=False).wait()
subprocess.Popen(['modprobe','edf'], shell=False).wait()
subprocess.Popen(['modprobe','hvdf'], shell=False).wait()
subprocess.Popen(['modprobe','lbesa'], shell=False).wait()
subprocess.Popen(['modprobe','dasa_nd'], shell=False).wait()
subprocess.Popen(['modprobe','dasa'], shell=False).wait()
subprocess.Popen(['modprobe','gfifo'], shell=False).wait()
subprocess.Popen(['modprobe','grma'], shell=False).wait()
subprocess.Popen(['modprobe','gedf'], shell=False).wait()
subprocess.Popen(['modprobe','gnp_edf'], shell=False).wait()
subprocess.Popen(['modprobe','ghvdf'], shell=False).wait()
subprocess.Popen(['modprobe','gnp_hvdf'], shell=False).wait()
subprocess.Popen(['modprobe','gmua'], shell=False).wait()
subprocess.Popen(['modprobe','ggua'], shell=False).wait()
subprocess.Popen(['modprobe','nggua'], shell=False).wait()
subprocess.Popen(['modprobe','abort_shmem'], shell=False).wait()
