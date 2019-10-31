#pragma once
#include "UnstructuredGrid3D.h"
#include "featureExtraction.h"
#include "Kdd.h"
#include "PSBClaParse.h"
#include <string>
#include <map>

struct distanceObject {
	string name;//filename
	string label;//class label
	float dValue;//distance
	distanceObject(string n, string l, float d) : name(n), label(l), dValue(d) {}
	bool operator<(distanceObject const& a);
	void printDistance();
};

struct labelData {
	float value = 0;
	int nModels = 0;//number of models for this class label
	int obsModels = 0;
	float precision = 0;
	float recall = 0;
	labelData(float f, int n) : value(f), nModels(n) {}
	labelData() {};
	void addValue(float f);
	void addPrecision(float p);
	void addRecall(float r);
};

class gridMatcher {
public:
	map<string, string> modelClasses; //filename -> labelname
	map<string, labelData> labels; //labelname -> labeldata
	PSBCategoryList* categories;
	gridFeatures **grids;
	int nGrids = 1796;//number of meshes in the feature table, doesnt change
	int nDims = 65;//dimension of feature vector

	//KNN tree
	KNNBuilder* knn;

	gridMatcher(string tableLocation);

	void matchAll(string databaseLocation, float* weights);
	void matchGrid(UnstructuredGrid3D* g, float& prec, float& rec, string inputName, float* weights);
	
	vector<distanceObject> getDistances(UnstructuredGrid3D* g, string tableLocation, string inputName, int& totalDistances, int& totalInLable, float* weights);
	vector<distanceObject> getKNNDistances(UnstructuredGrid3D* g, string tableLocation, string inputName, int& totalDistances, int& totalInLable, float* weights);


	void calculateAccuracy(vector<distanceObject>& distances, string inputLabel, int numberInClass, int totalDistances, int querySize, float& precision, float& recall);
	float calculateAVP(vector<distanceObject>& distances, string inputLabel, int numberInCLass, int totalDistances);

private:
	void readLabelData();
	void readFeatureTable(int n, string tableLocation);
};

void printAllDistance(vector<distanceObject>& dists);
