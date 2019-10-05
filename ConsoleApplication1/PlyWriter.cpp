#include "PlyWriter.h"
#include <sstream>

void PlyWriter::WritePlyFile(string filename, UnstructuredGrid3D* grid)
{
	ofstream plyfile(filename.replace(filename.length() - 4, 4, "_cleaned.ply"));
	WriteHeader(plyfile, grid->numPoints(), grid->numCells());

	stringstream line;

	for (int i = 0; i < grid->numPoints(); i++)
	{
		float point[3];
		grid->getPoint(i, point);
		line << point[0] << " " << point[1] << " " << point[2] << "\n";
		plyfile << line.str();
		line.str("");
	}
	
	for (int i = 0; i < grid->numCells(); i++)
	{
		int vertices[3];
		grid->getCell(i, vertices);
		line << "3 " << vertices[0] << " " << vertices[1] << " " << vertices[2] << "\n";
		plyfile << line.str();
		line.str("");
	}

	plyfile.close();
}

void PlyWriter::WriteHeader(ofstream& target, int nverts, int nfaces)
{
	//Convert nverts and nfaces to string
	std::stringstream nv, nf;
	nv << nverts;
	nf << nfaces;

	string plyHeader = "ply\nformat ascii 1.0\nelement vertex ";
	plyHeader.append(nv.str());
	plyHeader.append("\nproperty float x\nproperty float y\nproperty float z\nelement face ");
	plyHeader.append(nf.str());
	plyHeader.append("\nproperty list uchar int vertex_indices\nend_header\n");

	target << plyHeader;
}
