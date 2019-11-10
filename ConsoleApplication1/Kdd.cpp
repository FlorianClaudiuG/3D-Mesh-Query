#include "include/Kdd.h"
#include "include/KNN/src/ANN/ANN.h"
#include "include/featureExtraction.h"
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

//----------------------------------------------------------------------
//	KNNBuilder constructor
//----------------------------------------------------------------------

KNNBuilder::KNNBuilder(int pts, int dims, gridFeatures** grids, int nGrids, float* tweights)
{
	nPts = pts;
	nDims = dims;
	weights = tweights;

	dataPts = annAllocPts(nPts, nDims);			// allocate data points

	readGridFeatures(grids, nGrids);

	kdTree = new ANNkd_tree(		// build search structure
		dataPts,					// the data points
		nPts,						// number of points
		nDims);						// dimension of space
}

//----------------------------------------------------------------------
//	Search for k nearest neighbours on the created kd tree.
//----------------------------------------------------------------------

void KNNBuilder::KNNSearch(gridFeatures* f, int k, ANNdistArray& dists, ANNidxArray& indices){
	//ANNidxArray			nnIdx;				// near neighbor indices
	//ANNdistArray		dists;					// near neighbor distances
	ANNpoint			queryPt;				// query point

	indices = new ANNidx[k];					// allocate near neigh indices
	dists = new ANNdist[k];
	queryPt = annAllocPt(nDims);				// allocate query point
		
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

//----------------------------------------------------------------------
//	Transform a feature vector to a ANNpoint.
//----------------------------------------------------------------------

void KNNBuilder::readPoint(ANNpoint p, gridFeatures* f) {
	int dimIndex = 0;
	for (int i = 0; i < f->nFeatures; i++) {
		if (f->features[i]->nElements > 1) {
			for (int j = 0; j < f->features[i]->nElements; j++) {
				p[dimIndex] = f->features[i]->bins[j] * weights[i];
				dimIndex++;
			}
		}
		else {
			p[dimIndex] = f->features[i]->val * weights[i];
			dimIndex++;
		}
	}
}

//----------------------------------------------------------------------
//	Read in the entire feature table
//----------------------------------------------------------------------

void KNNBuilder::readGridFeatures(gridFeatures** grids, int nGrids) {
	for (int i = 0; i < nGrids; i++) {
		readPoint(dataPts[i], grids[i]);
	}
}