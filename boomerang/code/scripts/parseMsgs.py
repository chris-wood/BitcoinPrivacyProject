import sys
import os
import numpy as np
np.set_printoptions(threshold='nan')

def genStats(fname, fname_prefix):
	nodes = []

	# Build node collection
	numMessages = 0
	fin = open(fname, 'r')
	for line in fin:
		numMessages = numMessages + 1
		data = line.split("&")
		hops = data[1].split(",")

		for n in hops:
			nid = n.split("-")[1]
			if not (nid in nodes):
				nodes.append(nid)

	fin.close()
	n = int(max(nodes)) + 1
	print >> sys.stderr, "Number of messages: " + str(numMessages)
	print >> sys.stderr, "Number of nodes: " + str(n)	

	# Build matrices
	completeAM = np.zeros((n, n), dtype=np.int)
	completeChaffAM = np.zeros((n, n), dtype=np.int)
	completeTxAM = np.zeros((n, n), dtype=np.int)

	# Walk each message hop circuit and populate the matrices
	fin = open(fname, 'r')
	for line in fin:
		data = line.split("&")
		hops = data[1].split(",")

		# print(hops)

		for h in range(0, len(hops) - 1):
			src = int(hops[h].split("-")[1])
			dst = int(hops[h + 1].split("-")[1])

			# print("Before: " + str(completeAM[src, dst]))
			completeAM[src, dst] = completeAM[src, dst] + 1
			# print("After: " + str(completeAM[src, dst]))

			if "CHAFF" in data[0]:
				completeChaffAM[src, dst] = completeChaffAM[src, dst] + 1				
			if "TX" in data[0]:
				completeTxAM[src, dst] = completeTxAM[src, dst] + 1

	fout = open(fname_prefix + "_complete.mat", 'w')
	for i in range(0, n):
		for j in range(0, n - 1):
			fout.write(str(completeAM[i,j]) + ",")
		fout.write(str(completeAM[i, n-1]) + "\n")

	fout = open(fname_prefix + "_chaff.mat", 'w')
	for i in range(0, n):
		for j in range(0, n - 1):
			fout.write(str(completeChaffAM[i,j]) + ",")
		fout.write(str(completeChaffAM[i, n-1]) + "\n")

	fout = open(fname_prefix + "_tx.mat", 'w')
	for i in range(0, n):
		for j in range(0, n - 1):
			fout.write(str(completeTxAM[i,j]) + ",")
		fout.write(str(completeTxAM[i, n-1]) + "\n")

def main():
	for i in range(1, len(sys.argv)):
		fname = sys.argv[i]
		fout = os.path.basename(fname).split(".")[0] # drop extension
		genStats(fname, fout)

if __name__ =="__main__":
	main()
