#pragma once

#include "UnstructuredGrid3D.h"
#include <string>



class feature {
public:
	string name = "";
	int nElements =0;
	float val =0;
	float* bins =0;

	virtual string getFeatureString() =0;
protected:
	feature() {}
};

class histFeature : public feature {
public:
	float minVal;
	float maxVal;
	
	histFeature(int n, float min, float max, float* values, int nvalues, string nName);
	histFeature(int nbins, float* binValues);
	~histFeature();
	string getFeatureString();
};

class scalarFeature : public feature {
public:
	

	scalarFeature(float v, string nName);
	string getFeatureString();
};

class gridFeatures {
public:
	feature** features;
	int nFeatures;

	gridFeatures(UnstructuredGrid3D* g, int samples, int binsA3, int binsD1, int binsD2, int binsD3, int binsD4);
	gridFeatures(string featString, int* binnrs, int nScalar, int nHist);
	~gridFeatures();
	string featuresToString();
	void stringToFeatures();
};

float distancePoint(float* a, float* b);
float calculateAngle(float* a, float* b, float* c);
float triangleArea(float* a, float* b, float* c);

//Scalar features
scalarFeature* surfaceArea(UnstructuredGrid3D* g);
scalarFeature* boundingBoxVolume(UnstructuredGrid3D* g);
scalarFeature* eccentricity(UnstructuredGrid3D* g);
scalarFeature* compactness(UnstructuredGrid3D* g);
scalarFeature* diameter(UnstructuredGrid3D* g);

//Histogram features
histFeature* D1(UnstructuredGrid3D* g, int n, int nBins);
histFeature* D2(UnstructuredGrid3D* g, int n, int nBins);
histFeature* D3(UnstructuredGrid3D* g, int n, int nBins);
histFeature* D4(UnstructuredGrid3D* g, int n, int nBins);
histFeature* A3(UnstructuredGrid3D* g, int n, int nBins);

//distance functions
float featureVectorDistance(gridFeatures* fv1, gridFeatures* fv2);
float featureDistance(scalarFeature* f1, scalarFeature* f2);
float featureDistance(histFeature* f1, histFeature* f2);



