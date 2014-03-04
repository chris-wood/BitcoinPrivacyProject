import sys
import subprocess
import os
import shutil
import glob

# Declare variables for the delta+1 and delta+2 output
filedump = ""

if (len(sys.argv) != 2):
	print "Usage: python runSimulations.py fileListFile"
else:
	filedump = sys.argv[1]
	filedumpfile = open(filedump, 'r')
	for fname in filedumpfile:
		print >> sys.stderr, "Running: " + fname
		prefix = os.path.basename(fname).split(".")[0] # drop extension
		p = subprocess.Popen('java Boomerang ' + fname, shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
		fout = open(prefix + ".out", 'w')
		for line in p.stdout.readlines():
			fout.write(line + "\n")
			