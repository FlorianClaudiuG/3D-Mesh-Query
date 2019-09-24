#pragma once

#include "include/UnstructuredGrid3D.h"
#include <string>

class meshDescription
{
public:
	meshDescription(UnstructuredGrid3D* g, string directName, string fileName);
	string getFeatureString();
	static void writeColumnString(string outputDest);

	void writeDescriptionsToFile(string outputDest);
	
	void setNrVerts(int n) { nrVerts = n; };
	void setNrFaces(int n) { nrFaces = n; };
private:
	string a; string directory; string file;
	int nrVerts;
	int nrFaces;
	float avgX, avgY, avgZ;
	float minX = 1.0e6, minY = 1.0e6, minZ = 1.0e6;
	float maxX = -1.0e6, maxY = -1.0e6, maxZ = -1.0e6;
}; 
