#pragma once
#include "UnstructuredGrid3D.h"
#include <string>
#include <map>

struct labelData {
	float value = 0;
	int nModels = 0;//number of models for this class label
	int obsModels = 0;
	float precision = 0;
	float recall = 0;
	labelData(float f, int n) : value(f), nModels(n){}
	labelData() {};
	void addValue(float f);
	void addPrecision(float p);
	void addRecall(float r);
};

struct distanceObject {
	string name;//filename
	string label;//class label
	float dValue;//distance
	distanceObject(string n, string l, float d) : name(n), label(l), dValue(d) {}
	bool operator<(distanceObject const& a);
	void printDistance();
};

void calculateAccuracy(vector<distanceObject> &distances, string inputLabel, int numberInCLass, int totalDistances, int querySize, float &precision, float &recall);
void matchGrid(UnstructuredGrid3D* g, map<string, string> &modelClasses, map<string, labelData> &labels,float& prec, float& rec, string inputName);
void matchAll(string databaseLocation);
void printAllDistance(vector<distanceObject>& dists);
vector<distanceObject> getDistances(UnstructuredGrid3D* g, string tableLocation, map<string,string> labelDict, map<string,labelData> &labels, string inputName, int &totalDistances);