import os
import sys

# N, pi, sigma, avgLatency, avgChaffComputations, avgTxComputtions, avgForwarded, avgRetries

def extractStats(fname):
	configFileName = "config_" + fname + ".yaml"
	statsFileName = fname + "_stats.txt"

	n = 0
	pi = 0
	sigma = 0
	avgLatency = 0
	avgChaffComputations = 0
	avgTxComputations = 0
	avgForwarded = 0
	avgRetries = 0

	cf = open(configFileName, 'r')
	for l in cf:
		l = l.strip()
		data = l.split(":")
		if "numNodes" in l:
			n = int(data[1])
		elif "chaffGenRate" in l:
			pi = int(data[1])
		elif "txGenRate" in l:
			sigma = int(data[1])

	sf = open(statsFileName, 'r')
	l = sf.readline().strip()
	data = l.split(",")
	if not ("NaN" in l):
		avgLatency = float(data[5])
		avgChaffComputations = float(data[6])
		avgTxComputations = float(data[7])
		avgForwarded = float(data[8])
		avgRetries = float(data[9])

		# Display the results
		csv = str(n)
		csv = csv + " & " + str(pi)
		csv = csv + " & " + str(sigma)
		csv = csv + " & %.2f" % avgLatency
		csv = csv + " & %.2f" % avgChaffComputations
		csv = csv + " & %.2f" % avgTxComputations
		csv = csv + " & %.2f" % avgForwarded
		csv = csv + " & %.2f" % avgRetries
		csv = csv + " \\\\ "
		print(csv)

def main():
	fileListName = sys.argv[1]
	filelist = open(fileListName, 'r')
	print >> sys.stderr, "Reading from: " + fileListName
	for fname in filelist: 
		name = fname.strip()[0:fname.find("_")]
		print >> sys.stderr, name
		extractStats(name)
 
if __name__ =="__main__":
    main()
