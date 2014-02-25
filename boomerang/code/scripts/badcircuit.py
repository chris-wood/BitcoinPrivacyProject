import math
import sys

n = int(sys.argv[1])
d = int(sys.argv[2])
bad = int(sys.argv[3])
print n, d, bad

total = 0
for i in range(0, d):
	# print("i = " + str(i))
	prod = float(bad) / (n - 1 - i)
	# print("i = " + str(i) + ", prod = " + str(prod))
	innerProd = 1.0
	if i >= 1:
		for j in range(0, i):
			# print("j = " + str(j))
			innerProd = innerProd * (1.0 - (float(bad) / (n - 1 - j)))
		total = total + (prod * innerProd)
	else:
		total = prod

	# print "\n",
	# print(prod)
	# print(innerProd)
	# print(total)

print "\n",
print(total)
