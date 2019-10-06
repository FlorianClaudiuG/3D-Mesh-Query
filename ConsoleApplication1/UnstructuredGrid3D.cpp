#include "include/UnstructuredGrid3D.h"

void UnstructuredGrid3D::getPoint(int i,float* p)
{
	p[0] = pointsX[i];
	p[1] = pointsY[i];
	p[2] = pointsZ[i];
}


void UnstructuredGrid3D::setPoint(int i,float* p)
{
	if (i >= numPoints())
	{
		//Only works for increasing its size 1 at a time.
		pointsX.resize(numPoints() + 1);
		pointsY.resize(numPoints() + 1);
		pointsZ.resize(numPoints() + 1);
	}
	pointsX[i] = p[0];
	pointsY[i] = p[1];
	pointsZ[i] = p[2];
}

void UnstructuredGrid3D::getBoundingBox(float& minX, float& maxX, float& minY, float& maxY, float& minZ, float& maxZ)
{
	for (int i = 0; i < numPoints(); ++i)							//1. Determine the bounding-box of all points in the grid
	{
		float p[3];
		getPoint(i, p);
		minX = min(p[0], minX); maxX = max(p[0], maxX);
		minY = min(p[1], minY); maxY = max(p[1], maxY);
		minZ = min(p[2], minZ); maxZ = max(p[2], maxZ);
	}
}

void UnstructuredGrid3D::centerOnBary()
{
	float totalX = 0; float totalY = 0; float totalZ = 0;
	for (int i = 0; i < numPoints(); ++i)							//3. Use the scaling factor computed above to scale all grid
	{														//   points in the [-1,1] cube
		float p[3];
		getPoint(i, p);
		totalX += p[0]; totalY += p[1]; totalZ += p[2];
	}
	totalX /= numPoints(); totalY /= numPoints(); totalZ /= numPoints();

	for (int i = 0; i < numPoints(); ++i)							//3. Use the scaling factor computed above to scale all grid
	{														//   points in the [-1,1] cube
		float p[3];
		getPoint(i, p);
		p[0] = p[0] - totalX;
		p[1] = p[1] - totalY;
		p[2] = p[2] - totalZ;

		setPoint(i, p);
	}
	setAverages(totalX, totalY, totalZ);
}

void UnstructuredGrid3D::normalize()						//Normalize the grid in the [-1,1] cube
{
	float minX= 1.0e6,minY= 1.0e6,minZ= 1.0e6;
	float maxX=-1.0e6,maxY=-1.0e6,maxZ=-1.0e6;
	
	getBoundingBox(minX, maxX, minY, maxY, minZ, maxZ);

	float sizeX = maxX-minX;								//2. Compute a single scaling factor that best fits the grid
	sizeX = (sizeX)? 1/sizeX : 1;							//   in the [-1,1] cube. Using a single factor for x,y, and z
	float sizeY = maxY-minY;								//   ensures that the object is scaled while keeping its
	sizeY = (sizeY)? 1/sizeY : 1;							//   aspect ratio.
	float sizeZ = maxZ-minZ;
	sizeZ = (sizeZ)? 1/sizeZ : 1;
	
	float scale = min(sizeX,min(sizeY,sizeZ));
	
	float totalX = 0; float totalY = 0; float totalZ = 0; // Used to calculate average coordinates after normalization
	int n = numPoints();

	for(int i=0;i<numPoints();++i)							//3. Use the scaling factor computed above to scale all grid
	{														//   points in the [-1,1] cube
		float p[3];
		getPoint(i,p);
		
		p[0] = 2*((p[0]-minX)*scale-0.5);
		p[1] = 2*((p[1]-minY)*scale-0.5);
		p[2] = 2*((p[2]-minZ)*scale-0.5);

		totalX += p[0]; totalY += p[1]; totalZ += p[2];
		
		setPoint(i,p);
	}

	setAverages(totalX / n, totalY / n, totalZ / n);
}


Point3d cellNormal(int size,Point3d* p)						//Compute the normal of a cell whose three vertices are given in p[]
{
	Point3d edge1 = p[1]-p[0];								//We assume that the cell is a triangle. Then, the normal is the
	Point3d edge2 = p[2]-p[1];								//normalized (unit length) value of the cross-product of two cell edges.
	Point3d normal = edge1.cross(edge2);
	normal.normalize();
	return normal;
}	
	
	
void UnstructuredGrid3D::computeFaceNormals()				//Compute face normals for the grid. For this, we use the cellNormal()
{															//function presented above, for all grid triangles.
	for(int i=0;i<numCells();++i)
	{
		int cell[10];
		int size = getCell(i,cell);
		
		Point3d points[10];
		for(int j=0;j<size;++j)
		{
			float p[3];
		    getPoint(cell[j],p);
			points[j] = Point3d(p);
		}
		
		Point3d face_normal = cellNormal(size,points);
		
		faceNormals.setC0Vector(i,face_normal.data);
	}	
}	


void UnstructuredGrid3D::computeVertexNormals()				//Compute vertex normals for the grid. For this, we add, to each vertex,
{															//the normals of all cells that use that vertex. Next, we normalize the result.
	for(int i=0;i<numCells();++i)
	{
		int cell[10];
		int size = getCell(i,cell);

		Point3d face_normal = faceNormals.getC0Vector(i);
		
		for(int j=0;j<size;++j)
		{
			Point3d point_normal = pointNormals.getC0Vector(cell[j]);
			
			point_normal += face_normal;
			
			pointNormals.setC0Vector(cell[j],point_normal.data);
		}
	}
	
	for(int i=0;i<numPoints();++i)
	{
		Point3d point_normal = pointNormals.getC0Vector(i);
		point_normal.normalize();
		pointNormals.setC0Vector(i,point_normal.data);
	}
}

float UnstructuredGrid3D::getCellArea(int cell)
{
	int vertices[3]{};
	getCell(cell, vertices);
	
	float p1[3], p2[3], p3[3];
	getPoint(vertices[0], p1);
	getPoint(vertices[1], p2);
	getPoint(vertices[2], p3);

	Point3d A(p1);
	Point3d B(p2);
	Point3d C(p3);
	
	Point3d AB;
	AB.x = B.x - A.x;
	AB.y = B.y - A.y;
	AB.z = B.z - A.z;

	Point3d AC;
	AC.x = C.x - A.x;
	AC.y = C.y - A.y;
	AC.z = C.z - A.z;

	float normsum = (AB.norm() * AC.norm());
	float theta = acos(AB.dot(AC) / normsum);
	float area = 0.5f * normsum * sin(theta);

	return area;
}

void UnstructuredGrid3D::updateCellsByArea()
{
	struct Cell 
	{
		int index;
		float area;
	};

	vector<Cell> list;
	list.resize(numCells());

	for (int i = 0; i < numCells(); i++)
	{
		Cell temp;
		temp.index = i;
		temp.area = getCellArea(i);
		int len = list.size();
		if (len > 0)
		{
			for (int j = 0; j < len; j++)
			{
				if (temp.area < list[j].area)
				{
					list.insert(list.begin() + j -1, temp);
					break;
				}
			}
		}
		else
		{
			list.push_back(temp);
		}
		
		//list.push_back(temp);
		
	}
}