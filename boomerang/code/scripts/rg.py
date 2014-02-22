import sys
import csv
import numpy as np
import networkx as nx

# Load the matrix, and then render it...
adjMatrix = np.genfromtxt(sys.argv[1], delimiter=',')
print(adjMatrix)
G = nx.from_numpy_matrix(adjMatrix)
nx.draw(G)

# http://stackoverflow.com/questions/11804730/networkx-add-node-with-specific-position