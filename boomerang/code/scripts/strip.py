import sys

fname = sys.argv[1]
# dTarget = sys.argv[2]

fin = open(fname, 'r')

lineNum = 0
for l in fin:
	if (lineNum % 1000 == 0):
		data = l.strip().split(",")
		print >> sys.stderr, data
		# if (dTarget == data[1]):
		n = float(data[0])
		d = int(data[1])
		bad = float(data[2])
		prob = float(data[3])

		print >> sys.stderr, str(bad/n) + "," + str(d) + "," + str(prob)
		print str(bad/n) + "," + str(d) + "," + str(prob)
	lineNum = lineNum + 1

# http://stackoverflow.com/questions/7083044/mathematica-csv-to-multidimensional-charts
# data = Import["/Users/caw/Desktop/out_20140225.csv", "CSV"]