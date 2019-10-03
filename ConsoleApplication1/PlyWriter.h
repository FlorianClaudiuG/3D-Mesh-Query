#pragma once

#include "include/UnstructuredGrid3D.h"
#include <fstream>

class PlyWriter
{
public:
	PlyWriter()
	{

	}
	~PlyWriter()
	{

	}

	void WritePlyFile(string filename, UnstructuredGrid3D* grid);
	void WriteHeader(ofstream& target, int nverts, int nfaces);
};

