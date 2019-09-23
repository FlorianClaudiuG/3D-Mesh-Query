#pragma once
#include <fstream>
#include <iostream>
#include <string>

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
};

