#pragma once
#include <string>
#include "include/KNN/src/ANN/ANN.h"
#include "featureExtraction.h"

class KNNBuilder {
public:
	int					nPts;					// actual number of data points
	int					nDims;					// number of dimensions
	double				eps = 0;
	ANNpointArray		dataPts;				// data points
	ANNkd_tree*			kdTree;					// search structure

	KNNBuilder(int pts, int dims, string tableLocation);
	KNNBuilder(int pts, int dims, gridFeatures** grids, int nGrids);

	//KNN search for the first k neighbours of featurevector f, returns list of indices.
	ANNidxArray KNNSearch(gridFeatures* f, int k);

private:
	void readPoint(ANNpoint p, gridFeatures* f);
	void readData(string tableLocation, ANNpointArray& dataPts);
	void readGridFeatures(gridFeatures** grids, int nGrids);
};
	
void readData(string tableLocation, ANNpointArray& dataPts);