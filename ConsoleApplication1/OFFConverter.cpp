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
	} while (line.c_str()[0] == '#');

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
