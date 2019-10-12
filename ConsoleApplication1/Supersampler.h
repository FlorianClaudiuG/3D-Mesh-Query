#include "include/UnstructuredGrid3D.h"

#pragma once

class Supersampler
{
public:
	Supersampler()
	{

	}
	~Supersampler()
	{

	}

	void supersample(UnstructuredGrid3D& grid, int target);
	vector<Cell> addTriangle(UnstructuredGrid3D& grid, int cell);
};

