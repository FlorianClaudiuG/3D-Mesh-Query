#include "OFFConverter.h"

using namespace std;

void OFFConverter::ConvertOFFToPLY(string filename)
{
	ifstream fin(filename);
	ofstream fout(filename.replace(filename.length() - 3, 3, "ply"));

	string line = "";

	getline(fin, line); //off

	do
	{
		getline(fin, line);
		if (line.c_str()[0] >= '0' && line.c_str()[0] <= '9')
		{
			short delim = line.find(' ');
			string nverts = line.substr(0, delim);
			cout << nverts;
			short delim2 = line.find(' ', delim);
			string nfaces = line.substr(delim, delim2);
			cout << nfaces;
		}
	} while (line.c_str()[0] == '#');


	while (!fin.eof())
	{
		getline(fin, line);
		if (line.c_str()[0] != '#')
		{
			fout << line << "\n";
		}
	}
	fout.close();
}
