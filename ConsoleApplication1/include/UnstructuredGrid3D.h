#pragma once

#include "UnstructuredGrid.h"
#include "VectorAttributes.h"

using namespace std;




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

private:

float				avgX, avgY, avgZ;	//averages of coordinates 
vector<float>		pointsZ;
VectorAttributes    pointNormals;
VectorAttributes    faceNormals;
};



