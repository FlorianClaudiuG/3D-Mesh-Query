import struct
from tsne import tsne
import numpy as np
import pylab

fin = open("featureFinal.csv", 'r')
fout = open("data.dat", 'wb')
labels = open("labels.txt", 'w')
lines = fin.readlines()

n = 1796
d = 65
theta = 0
perplexity = 30
nDims = 2

entry = struct.pack("<iiddi", n, d, theta, perplexity, nDims)
fout.write(entry)

for line in lines:
    tokens = line.split(',')
    labels.write(tokens[0] + "\n")
    for i in range(1, 66):
        entry = struct.pack("<d", float(tokens[i]))
        fout.write(entry)

fout.close()
labels.close()

X = np.loadtxt("data.dat")
labels = np.loadtxt("labels.txt")
Y = tsne(X, 2, 50, 20.0)
pylab.scatter(Y[:, 0], Y[:, 1], 20, labels)
pylab.show()