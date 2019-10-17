#pragma once
#include "UnstructuredGrid3D.h"
#include <string>

struct distanceObject {
	string name;//filename
	string label;//class label
	float dValue;//distance
	distanceObject(string n, string l, float d) : name(n), label(l), dValue(d) {}
	bool operator<(distanceObject const& a);
	void printDistance();
};


void printAllDistance(vector<distanceObject>& dists);
void getDistances(UnstructuredGrid3D* g, string tableLocation);