#pragma once

#include "UnstructuredGrid.h"
#include "VectorAttributes.h"
#include <iostream>
#include <algorithm>
#include <math.h>

using namespace std;

struct Cell
{
	int index;
	float area;
};

class UnstructuredGrid3D : public UnstructuredGrid
{
public: 
					UnstructuredGrid3D(int P,int C): UnstructuredGrid(P,C),pointNormals(P),faceNormals(C), pointsZ(P)
					{ }	 
					 	
void				getPoint(int i,float* p);

void				setPoint(int i,float* p);	

void				getBoundingBox(float& minX, float& maxX, float& minY, float& maxY, float& minZ, float& maxZ);

void				normalize();

void				centerOnBary();

void				computeFaceNormals();

void				computeVertexNormals();

VectorAttributes&	getFaceNormals()
					{ return faceNormals; }

VectorAttributes&	getPointNormals()
					{ return pointNormals; }

void				setAverages(float x, float y, float z) { avgX = x; avgY = y; avgZ = z; }
void				getAverages(float& x, float& y, float& z) { x = avgX; y = avgY; z = avgZ; }
float				getCellArea(int cell);
vector<Cell>		sortCellsByArea();	//returns a vector of cells sorted by area
float				computeCircularity();
float				getTotalArea();
float				getVolume();
float				signedVolumeOfTetrahedron(float* p1, float* p2, float* p3);
float				getDiameter(int samplePoints);
float				getDistance(int x, int y);

private:

float				avgX, avgY, avgZ;	//averages of coordinates 
vector<float>		pointsZ;
VectorAttributes    pointNormals;
VectorAttributes    faceNormals;

};



