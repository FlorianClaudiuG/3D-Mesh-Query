#include "Supersampler.h"

void Supersampler::supersample(UnstructuredGrid3D& grid, int target)
{
	if (grid.numPoints() >= target)
	{
		return;
	}
	else
	{
		//use a datastructure to store all cells sorted by their area

	}

	int index = 0;

	while(grid.numPoints() < target)
	{

	}
}

void Supersampler::addTriangle(UnstructuredGrid3D& grid, int cell)
{
	int vertices[3];
	float p1[3], p2[3], p3[3];

	grid.getCell(cell, vertices);
	grid.getPoint(vertices[0], p1);
	grid.getPoint(vertices[1], p2);
	grid.getPoint(vertices[2], p3);

	float mid1[3], mid2[3], mid3[3];

	mid1[0] = (p1[0] + p2[0]) / 2;
	mid1[1] = (p1[1] + p2[1]) / 2;
	mid1[2] = (p1[2] + p2[2]) / 2;

	mid2[0] = (p2[0] + p3[0]) / 2;
	mid2[1] = (p2[1] + p3[1]) / 2;
	mid2[2] = (p2[2] + p3[2]) / 2;

	mid3[0] = (p3[0] + p1[0]) / 2;
	mid3[1] = (p3[1] + p1[1]) / 2;
	mid3[2] = (p3[2] + p1[2]) / 2;

	int i1, i2, i3;
	
	i1 = grid.numPoints();
	grid.setPoint(i1, mid1);

	i2 = grid.numPoints();
	grid.setPoint(i2, mid2);

	i3 = grid.numPoints();
	grid.setPoint(i3, mid3);

	int newcell[3];
	newcell[0] = i1;
	newcell[1] = i2;
	newcell[2] = i3;

	grid.setCell(grid.numCells() - 1, newcell);

	newcell[0] = vertices[0];
	newcell[1] = i1;
	newcell[2] = i3;

	grid.setCell(grid.numCells(), newcell);

	newcell[0] = i1;
	newcell[1] = vertices[1];
	newcell[2] = i2;

	grid.setCell(grid.numCells(), newcell);

	newcell[0] = i2;
	newcell[1] = vertices[2];
	newcell[2] = i3;

	grid.setCell(grid.numCells(), newcell);
}

