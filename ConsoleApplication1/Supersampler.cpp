#include "Supersampler.h"

void Supersampler::supersample(UnstructuredGrid3D& grid, int target)
{
	if (grid.numPoints() >= target)
	{
		return;
	}
	
	vector<Cell> vec = grid.sortCellsByArea();

	while(grid.numPoints() < target)
	{
		vector<Cell> newCells = addTriangle(grid, (vec.back()).index);
		vec.pop_back();
		//cout << "\n --" << vec.size() << " : " << grid.numCells() << "-- \n\n";
		int size = newCells.size();
		for (int i = 0; i < size; i++)
		{
			bool inserted = false;
			if (vec.empty())
			{
				vec.push_back(newCells[i]);
			}
			else
			{
				for (int j = 0; j < vec.size(); j++)
				{
					if ((newCells[i].area <= vec[j].area))
					{
						vec.insert(vec.begin() + j, newCells[i]);
						inserted = true;
						//for (int x = 0; x < vec.size(); x++)
						{
							//cout <<"[" << vec[x].area << "] ";
						}
						//cout << "\n";
						break;
					}
				}
				if (!inserted)
				{
					vec.push_back(newCells[i]);
				}
			}
		}
	}
}

vector<Cell> Supersampler::addTriangle(UnstructuredGrid3D& grid, int cell)
{
	vector<Cell> newCells;
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

	grid.setCell(cell, newcell);

	Cell temp0;
	temp0.index = cell;
	temp0.area = grid.getCellArea(cell);

	newCells.push_back(temp0);

	newcell[0] = vertices[0];
	newcell[1] = i1;
	newcell[2] = i3;

	int index = grid.numCells();
	grid.setCell(index, newcell);

	Cell temp1;
	temp1.index = index;
	temp1.area = grid.getCellArea(index);

	newCells.push_back(temp1);

	newcell[0] = i1;
	newcell[1] = vertices[1];
	newcell[2] = i2;

	grid.setCell(grid.numCells(), newcell);

	Cell temp2;
	temp2.index = ++index;
	temp2.area = grid.getCellArea(index);

	newCells.push_back(temp2);

	newcell[0] = i2;
	newcell[1] = vertices[2];
	newcell[2] = i3;

	grid.setCell(grid.numCells(), newcell);

	Cell temp3;
	temp3.index = ++index;
	temp3.area = grid.getCellArea(index);

	newCells.push_back(temp3);

	return newCells;
}

