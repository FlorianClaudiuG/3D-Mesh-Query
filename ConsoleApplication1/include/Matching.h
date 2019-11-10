#pragma once
#include "UnstructuredGrid3D.h"
#include "featureExtraction.h"
#include "Kdd.h"
#include "PSBClaParse.h"
#include <string>
#include <map>

//to store information about a model in a query result
struct distanceObject {
	string name;//filename
	string label;//class label
	string pathName;//pathname for drawing
	float dValue;//distance
	distanceObject(string n, string l, float d, string path) : name(n), label(l), dValue(d), pathName(path){}
	bool operator<(distanceObject const& a);
	void printDistance();
};

//important information for a class label
struct labelData {
	float value = 0;
	int nModels = 0;		//number of models for this class label
	int obsModels = 0;		//number of models actually observed
	float precision = 0;	//MAP
	float precisionAtk = 0;	//precision at k
	labelData(float f, int n) : value(f), nModels(n) {}
	labelData() {};
	void addValue(float f);
	void addPrecision(float p);
	void addprecisionAtk(float r);
};

class gridMatcher {
public:
	gridMatcher(string tableLocation, string databaseLocation, float* tweights);

	void matchAll(float* weights, int k);
	vector<distanceObject> matchSingle(UnstructuredGrid3D* g, string inputName, int k);
	
private:
	map<string, string> modelClasses;	//filename -> labelname
	map<string, labelData> labels;		//labelname -> labeldata
	map<string, string> filePaths;		//modelName -> pathName
	PSBCategoryList* categories;
	gridFeatures** grids;
	int nGrids = 1796;					//number of meshes in the feature table, doesnt change
	int nDims = 65;						//dimension of feature vector

	string databaseLocation;
	string tableLocation;

	KNNBuilder* knn;
	float* weights;

	vector<distanceObject> getDistances(UnstructuredGrid3D* g, string tableLocation, string inputName, int& totalDistances, int& totalInLable);
	vector<distanceObject> getKNNDistances(UnstructuredGrid3D* g, string tableLocation, string inputName, int& totalDistances, int& totalInLable, int k);

	void calculateAccuracy(vector<distanceObject>& distances, string inputLabel, int numberInClass, int totalDistances, int querySize, float& precision, float& recall);
	float calculateAVP(vector<distanceObject>& distances, string inputLabel, int numberInCLass, int totalDistances);

	void readLabelData();
	void getFileLocations();
	void readFeatureTable(int n);
	void matchGrid(UnstructuredGrid3D* g, float& prec, float& rec, string inputName, float* weights, int k);
};

void printAllDistance(vector<distanceObject>& dists);
