#include "include/UnstructuredGrid3D.h"
#include "include/featureExtraction.h"
#include "include/Matching.h"
#include <iostream>
#include <fstream>
#include <sstream>

void getDistances(UnstructuredGrid3D* g, string tableLocation) {
	ifstream infile(tableLocation);
	string line;
	string name;
	char delimeter = ',';

	vector<distanceObject> distanceList;//store all distances with names

	gridFeatures* f1 = new gridFeatures(g, 20000, 12, 12, 12, 12, 12);

	while (getline(infile, line))
	{
		stringstream ss(line);
		getline(ss, name, delimeter);
		getline(ss, line);
		int lengths[5] = { 12,12,12,12,12 };
		gridFeatures* f2 = new gridFeatures(line, lengths, 5, 5);

		float d = featureVectorDistance(f1, f2);

		distanceList.push_back(distanceObject(name, "L", d));
	}
	sort(distanceList.begin(),distanceList.end());
	printAllDistance(distanceList);
}

bool distanceObject::operator<(distanceObject const& a)
{
	return (dValue < a.dValue);
}

void printAllDistance(vector<distanceObject>& dists) {
	for (vector<distanceObject>::iterator it = dists.begin(); it != dists.end(); ++it) {
		it->printDistance();
	}
}

void distanceObject::printDistance()
{
	cout << name << "/" << label << ": " << dValue << endl;
}
