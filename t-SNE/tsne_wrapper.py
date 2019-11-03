from tsne import tsne
import pylab
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import sys
import mplcursors

perplexity = 70.0

fin = open("featureFinal.csv", 'r')
labelsFile = open("labels.txt", 'r')
classesFile = open("classes.txt", 'r')

labelsLines = labelsFile.readlines()
#labels = np.zeros(shape=1796)
labels = [None] * 1796

classMapRaw = {}
classMap = {}
allclasses = []

classesLines = classesFile.readlines()
for line in classesLines:
    splitLine = line.split(',')
    classMapRaw[splitLine[0]] = splitLine[1].split('\n')[0]
    if(splitLine[1].split('\n')[0] in allclasses):
        pass
    else:
        allclasses.append(splitLine[1].split('\n')[0])

for index, line in enumerate(labelsLines):
    labels[index] = line.split('\n')[0].lstrip('m')
    if str(labels[index]) in str(classMapRaw.keys()):
        classMap[labels[index]] = classMapRaw[labels[index]]

n = 1796
d = 65
theta = 0
perplexity = 30
nDims = 2

lines = fin.readlines()
X = np.zeros(shape=(n, d))

for index, line in enumerate(lines):
    tokens = line.split(',')
    lineList = []
    for i in range(1, 66):
        lineList.append(float(tokens[i]))
    X[index] = lineList

rawdf = []

for index, label in enumerate(labels):
    temp = (label, classMap[str(label)])
    rawdf.append(temp)

df = pd.DataFrame(data=rawdf, columns=['ModelNo', 'Class'])

print(df)

print(len(allclasses))


def read_tsneresult():
    tsnePrev = open("tsneresult.txt", 'r')
    text = tsnePrev.read()
    pointsArray = text.split(';')
    npoints = len(pointsArray)
    ndims = len(pointsArray[0].split(','))
    Y = np.zeros(shape=(npoints - 1, ndims))
    for index, point in enumerate(pointsArray):
        dims = point.split(',')
        if len(dims) == 2:
            Y[index, 0] = float(dims[0])
            Y[index, 1] = float(dims[1])
    return Y

useoldresults = input("Plot last tsne result? Yy/Nn")

if useoldresults == 'y' or useoldresults == 'Y':
    Y = read_tsneresult()
elif useoldresults == 'n' or useoldresults == 'N':
    Y = tsne(X, 2, 65, perplexity)
    tsneResultFile = open("tsneresult.txt", 'w')
    for point in Y:
        tsneResultFile.write(str(point[0]) + "," + str(point[1]))
        tsneResultFile.write(';')
    tsneResultFile.close()

g = sns.scatterplot(x=Y[:, 0], y=Y[:, 1], data=df, hue='Class', palette='gist_ncar')
h, l = g.axes.get_legend_handles_labels()
g.axes.legend_.remove()
g.legend(h,l,ncol=2,bbox_to_anchor=(1.02, 1), loc=2, borderaxespad=0.)

annot = []

for index, point in enumerate(Y):
    ann = g.annotate('m' + str(labels[index]) + " - " + classMap[str(labels[index])], xy=(point[0], point[1]), xytext=(-20, 20),
                            textcoords='offset points', ha='center', va='bottom',
                            bbox=dict(boxstyle='round,pad=0.2', fc='yellow', alpha=0.3),
                            arrowprops=dict(arrowstyle='->', connectionstyle='arc3,rad=0.5', color='black'))
    ann.set_visible(False)
    annot.append(ann)

def press(event):
    print('press', event.key)
    sys.stdout.flush()
    if(event.key == 'x'):
        for ann in annot:
            ann.set_visible(not ann.get_visible())
        g.figure.canvas.draw()

g.figure.canvas.mpl_connect("key_press_event", press)

plt.show()
