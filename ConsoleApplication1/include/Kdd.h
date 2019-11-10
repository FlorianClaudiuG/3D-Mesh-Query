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
	float* weights;								// weights for the feature vector

	KNNBuilder(int pts, int dims, gridFeatures** grids, int nGrids, float* tweights);

	//KNN search for the first k neighbours of featurevector f.
	void KNNSearch(gridFeatures* f, int k, ANNdistArray &dists, ANNidxArray &indices);

private:
	void readPoint(ANNpoint p, gridFeatures* f);
	void readGridFeatures(gridFeatures** grids, int nGrids);
};