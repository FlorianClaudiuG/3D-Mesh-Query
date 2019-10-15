#include "OFFConverter.h"

using namespace std;

void OFFConverter::ConvertOFFToPLY(string filename)
{
	ifstream fin(filename);
	ofstream fout(filename.replace(filename.length() - 3, 3, "ply"));

	string line = "";

	getline(fin, line); //off
	string nverts = "";
	string nfaces = "";
	do
	{
		getline(fin, line);
		if (line.c_str()[0] >= '0' && line.c_str()[0] <= '9')
		{
			short delim = line.find(' ');
			nverts = line.substr(0, delim);
			cout << nverts;
			short delim2 = line.find(' ', delim);
			nfaces = line.substr(delim, delim2 + 1);
			cout << nfaces;
		}
	} while (line.c_str()[0] == '#' || line.c_str()[0] == ' ' || line.c_str()[0] == '\n');

	string plyHeader = "ply\nformat ascii 1.0\nelement vertex ";
	plyHeader.append(nverts);
	plyHeader.append("\nproperty float x\nproperty float y\nproperty float z\nelement face ");
	plyHeader.append(nfaces);
	plyHeader.append("\nproperty list uchar int vertex_indices\nend_header\n");

	fout << plyHeader;

	while (!fin.eof())
	{
		getline(fin, line);
		if (line.c_str()[0] != '#')
		{
			fout << line << "\n";
		}
	}
	fin.close();
	fout.close();
}

void OFFConverter::WriteFileOFF(UnstructuredGrid3D& grid, std::string outputPath)
{
	ofstream myfile;
	myfile.open(outputPath, ios::out | ios::trunc);


	myfile << "OFF\n";
	myfile << grid.numPoints() << " " << grid.numCells() << "  0\n";	//ignore edge count
	//vertices
	for (int i = 0; i < grid.numPoints(); i++)
	{
		float p[3];
		grid.getPoint(i, p);
		myfile << p[0] << " " << p[1] << " " << p[2] << "\n";
	}
	//assume triangles
	for (int i = 0; i < grid.numCells(); i++)
	{
		int c[3];
		grid.getCell(i, c);
		myfile << "3 " << c[0] << " " << c[1] << " " << c[2] << "\n";
	}

	myfile.close();
}
