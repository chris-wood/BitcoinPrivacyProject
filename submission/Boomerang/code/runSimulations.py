import sys
import subprocess
import os
import shutil
import glob

# Declare variables for the delta+1 and delta+2 output
filedump = ""

# Compile!
p = subprocess.Popen('javac -cp "jyaml-1.3.jar:." *.java', shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)

if (len(sys.argv) != 2):
	print "Usage: python runSimulations.py fileListFile"
else:
	filedump = sys.argv[1]
	filedumpfile = open(filedump, 'r')
	for fname in filedumpfile:
		fname = str(fname.strip())
		print >> sys.stderr, "Running: " + fname
		prefix = str(os.path.basename(fname).split(".")[0]) # drop extension
		p = subprocess.Popen('java -Xmx12g -cp "jyaml-1.3.jar:." Boomerang ' + str(fname), shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
		fout = open(prefix + ".out", 'w')
		for line in p.stdout.readlines():
			line = line.strip()
			fout.write(line + "\n")
			