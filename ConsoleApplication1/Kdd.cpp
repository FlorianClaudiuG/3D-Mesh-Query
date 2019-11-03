#include "include/Kdd.h"
#include "include/KNN/src/ANN/ANN.h"
#include "include/featureExtraction.h"
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

void printPt(ostream& out, ANNpoint p, int nDims)			// print point
{
	out << "(" << p[0];
	for (int i = 1; i < nDims; i++) {
		out << ", " << p[i];
	}
	out << ")\n";
}

KNNBuilder::KNNBuilder(int pts, int dims, string tableLocation) {
	nPts = pts;
	nDims = dims;

	dataPts = annAllocPts(nPts, nDims);			// allocate data points

	readData(tableLocation, dataPts);

	kdTree = new ANNkd_tree(					// build search structure
		dataPts,					// the data points
		nPts,						// number of points
		nDims);						// dimension of space
}

KNNBuilder::KNNBuilder(int pts, int dims, gridFeatures** grids, int nGrids)
{
	nPts = pts;
	nDims = dims;

	dataPts = annAllocPts(nPts, nDims);			// allocate data points

	readGridFeatures(grids, nGrids);

	kdTree = new ANNkd_tree(					// build search structure
		dataPts,					// the data points
		nPts,						// number of points
		nDims);						// dimension of space
}

void KNNBuilder::KNNSearch(gridFeatures* f, int k, ANNdistArray& dists, ANNidxArray& indices){
	//ANNidxArray			nnIdx;					// near neighbor indices
	//ANNdistArray		dists;					// near neighbor distances
	ANNpoint			queryPt;				// query point

	indices = new ANNidx[k];						// allocate near neigh indices
	dists = new ANNdist[k];
	queryPt = annAllocPt(nDims);					// allocate query point
		
	readPoint(queryPt, f);

	kdTree->annkSearch(						// search
		queryPt,						// query point
		k,								// number of near neighbors
		indices,							// nearest neighbors (returned)
		dists,							// distance (returned)
		eps);							// error bound

	//cout << "\tNN:\tIndex\tDistance\n";
	for (int i = 0; i < k; i++) {			// print summary
		dists[i] = sqrt(dists[i]);			// unsquare distance
		//cout << "\t" << i << "\t" << nnIdx[i] << "\t" << dists[i] << "\n";
	}
}

void KNNBuilder::readPoint(ANNpoint p, gridFeatures* f) {
	int dimIndex = 0;
	for (int i = 0; i < f->nFeatures; i++) {
		if (f->features[i]->nElements > 1) {
			for (int j = 0; j < f->features[i]->nElements; j++) {
				p[dimIndex] = f->features[i]->bins[j];
				dimIndex++;
			}
		}
		else {
			p[dimIndex] = f->features[i]->val;
			dimIndex++;
		}
	}
}

void KNNBuilder::readData(string tableLocation, ANNpointArray &dataPts) {
	ifstream infile(tableLocation);
	string line;
	string name;
	char delimeter = ',';

	int pointIndex = 0;
	while (getline(infile, line))
	{
		stringstream ss(line);
		getline(ss, name, delimeter);
		getline(ss, line);

		int lengths[5] = { 12,12,12,12,12 };
		gridFeatures* f2 = new gridFeatures(line, lengths, 5, 5);

		readPoint(dataPts[pointIndex],f2);

		pointIndex++;
		delete f2;
	}
}

void KNNBuilder::readGridFeatures(gridFeatures** grids, int nGrids) {
	for (int i = 0; i < nGrids; i++) {
		readPoint(dataPts[i], grids[i]);
	}
}