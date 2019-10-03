#pragma once
#include "include/UnstructuredGrid3D.h"

class FourStepNorm
{
public:
	void PCA(UnstructuredGrid3D* g);
	void flipTest(UnstructuredGrid3D* g);
	void centerOnBary(UnstructuredGrid3D* g);
	void normalizeInCube(UnstructuredGrid3D* g);
};