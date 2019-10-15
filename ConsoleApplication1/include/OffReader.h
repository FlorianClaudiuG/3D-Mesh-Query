#pragma once
#include "include/UnstructuredGrid3D.h"

class OffReader
{
public:

	OffReader() { }
	UnstructuredGrid3D* ReadOffFile(const char* filename);
};



