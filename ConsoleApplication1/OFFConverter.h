#pragma once
#include <fstream>
#include <iostream>
#include <string>
#include "include/UnstructuredGrid3D.h"

class OFFConverter
{
	//read .off file
	//process it

public:
	OFFConverter() 
	{

	}
	~OFFConverter()
	{

	}
	void ConvertOFFToPLY(std::string filename);
	void WriteFileOFF(UnstructuredGrid3D& grid, std::string outputPath);
};

