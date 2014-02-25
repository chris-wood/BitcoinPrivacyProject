import math
import sys

def badCircuitProb(n, d, bad):
	# do the computation
	total = 0
	for i in range(0, d):
		prod = float(bad) / (n - 1 - i)
		innerProd = 1.0
		if i >= 1:
			for j in range(0, i):
				innerProd = innerProd * (1.0 - (float(bad) / (n - 1 - j)))
			total = total + (prod * innerProd)
		else:
			total = prod
	print str(n) + "," + str(d) + "," + str(bad) + "," + str(total)

def main():
	# parse cmd lines
	nlow = int(sys.argv[1])
	nhigh = int(sys.argv[2])
	nskip = int(sys.argv[3])
	dlow = int(sys.argv[4])
	dhigh = int(sys.argv[5])

	for n in range(nlow, nhigh + 1, nskip):
		for d in range(dlow, dhigh + 1):
			for bad in range(1, n/2 + 1):
				badCircuitProb(n, d, bad)

if __name__ == "__main__":
	main()
