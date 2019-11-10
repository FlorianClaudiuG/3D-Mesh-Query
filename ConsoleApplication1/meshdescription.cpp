#include "include/meshDescription.h"
#include "include/UnstructuredGrid3D.h"
#include <string>
#include <iostream>
#include <fstream>

//----------------------------------------------------------------------
//	meshDescription constructor
//----------------------------------------------------------------------

meshDescription::meshDescription(UnstructuredGrid3D* g, string directName, string fileName)
{
	setNrFaces(g->numCells());
	setNrVerts(g->numPoints());
	directory = directName; file = fileName;

	g->getBoundingBox(minX, maxX, minY, maxY, minZ, maxZ);
	g->getAverages(avgX, avgY, avgZ);
}

//----------------------------------------------------------------------
//	returns a string with a summary of the grid
//----------------------------------------------------------------------

string meshDescription::getDescriptionString()
{
	string s;
	s += directory; s += ",";
	s += file; s += ",";
	s += to_string(nrVerts); s += ",";
	s += to_string(nrFaces); s += ",";
	s += to_string(avgX); s += ",";
	s += to_string(avgY); s += ",";
	s += to_string(avgZ); s += ",";
	s += to_string(minX); s += ",";	s += to_string(maxX); s += ",";
	s += to_string(minY); s += ",";	s += to_string(maxY); s += ",";
	s += to_string(minZ); s += ",";	s += to_string(maxZ);

	return s;
}

//----------------------------------------------------------------------
//	write the collumn header to a file
//----------------------------------------------------------------------

void meshDescription::writeColumnString(string outputDest)
{
	ofstream myfile;
	myfile.open(outputDest, ios::out | ios::trunc);
	myfile << "directory,filename,nrVertices,nrFaces,avgX,avgY,AvgZ,minX,maxX,minY,maxY,minZ,maxZ" << endl;
	myfile.close();
}

//----------------------------------------------------------------------
//	write the description of the mesh to a file
//----------------------------------------------------------------------

void meshDescription::writeDescriptionsToFile(string outputDest)
{
	ofstream myfile;
	myfile.open(outputDest, ios::out | ios::app);
	myfile << getDescriptionString() << endl;
	myfile.close();
}


