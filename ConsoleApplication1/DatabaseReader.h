#include <iostream>
#include <fstream>
#include "include/UnstructuredGrid3D.h"
#include <experimental/filesystem>
#include "OFFConverter.h"
#include "include/PlyReader.h"

using namespace std;

#pragma once
class DatabaseReader
{
	//read each file in DB
	//process it
	//save it in output
public:
	static void createDatabase(string path);
	static void processGrid(UnstructuredGrid3D& grid);
};

