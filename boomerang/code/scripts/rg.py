import sys
import csv
import numpy as np
import networkx as nx
import matplotlib.pyplot as plt

# Load the matrix, and then render it...
adjMatrix = np.genfromtxt(sys.argv[1], delimiter=',')
print(adjMatrix)
G = nx.from_numpy_matrix(adjMatrix)
nx.draw_circular(G)
plt.savefig(sys.argv[1] + '.png')

# http://stackoverflow.com/questions/11804730/networkx-add-node-with-specific-position