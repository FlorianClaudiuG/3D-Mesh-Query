#include "DatabaseReader.h"

void DatabaseReader::createDatabase(string path)
{
	namespace fs = std::experimental::filesystem;
	UnstructuredGrid3D* mesh;
	
	for (const auto& entry : fs::directory_iterator(path))
	{
		string directoryName = entry.path().filename().string();
		for (const auto& entry2 : fs::directory_iterator(entry.path()))
		{
			string directoryName2 = entry2.path().filename().string();
			string pathName = entry2.path().string();
			string fileName = pathName + "\\" + directoryName2 + ".off";

			OFFConverter* converter = new OFFConverter();
			converter->ConvertOFFToPLY(fileName);

			PlyReader rdr;
			mesh = rdr.read((pathName + "\\" + directoryName2 + ".ply").c_str());

			mesh->normalize();

			delete mesh;
			delete converter;
		}
	}
}

void DatabaseReader::processGrid(UnstructuredGrid3D& grid)
{
}
