#include "include/FourStepNorm.h"
#include "include/PCA/dataanalysis.h"
#include <algorithm> //for max()


void FourStepNorm::PCA(UnstructuredGrid3D* g)
{
	using namespace alglib;
	int numberPoints = g->numPoints();

	real_2d_array input; //Store all vertices as real_2d_array

	double* numbers = new double[numberPoints * 3];
	for (int i = 0; i < numberPoints; i++)
	{
		float V[3];
		g->getPoint(i, V);
		for (int j = 0; j < 3; j++)
		{
			numbers[i * 3 + j] = V[j];
		}
	}
	input.setcontent(numberPoints, 3, numbers);
	//Initialize output parameters
	ae_int_t info;
	real_1d_array eigenValues;
	real_2d_array eigenVector;
	eigenValues.setlength(3);
	eigenVector.setlength(3, 3);
	pcabuildbasis(input, numberPoints, 3, info, eigenValues, eigenVector);

	//Normal axis
	double X1[3] = { 1,0,0 };
	double X2[3] = { 0,1,0 };
	double X3[3] = { 0,0,1 };

	//Eigenvectors for translation
	double X1Prime[3] = { eigenVector[0][0],eigenVector[1][0],eigenVector[2][0] };
	double X2Prime[3] = { eigenVector[0][1], eigenVector[1][1],eigenVector[2][1] };
	double X3Prime[3] = { eigenVector[0][2], eigenVector[1][2],eigenVector[2][2] };

	//Create the translation matrix
	double M11 = vdotproduct(X1, X1Prime, 3);
	double	M12 = vdotproduct(X1, X2Prime, 3);
	double	M13 = vdotproduct(X1, X3Prime, 3);
	double	M21 = vdotproduct(X2, X1Prime, 3);
	double	M22 = vdotproduct(X2, X2Prime, 3);
	double	M23 = vdotproduct(X2, X3Prime, 3);
	double	M31 = vdotproduct(X3, X1Prime, 3);
	double	M32 = vdotproduct(X3, X2Prime, 3);
	double	M33 = vdotproduct(X3, X3Prime, 3);
	double ns[] = { M11,M12,M13,M21,M22,M23,M31,M32,M33 };

	//Alglib data structures for matrix multiplication
	real_1d_array A; //vector to be translated
	real_2d_array B; //translation matrix
	real_1d_array C; //new vector
	A.setlength(3);
	C.setlength(3);
	B.setcontent(9, 3, ns);
	densesolverreport r; //need this for some reason

	float totalX = 0; float totalY = 0; float totalZ = 0;

	for (int i = 0; i < numberPoints; i++)
	{	//Read the point and convert to real_1d_array, than do matrix multiplication to get new coordinate 
		float V[3];
		g->getPoint(i, V);
		A[0] = V[0];
		A[1] = V[1];
		A[2] = V[2];
		rmatrixsolve(B, 3, A, info, r, C); //Do matrix multiplication to get the new vertex coordinates
		float T[3] = { C[0],C[1],C[2] };
		totalX += C[0]; totalY += C[1]; totalZ += C[2];
		g->setPoint(i, T);
	}
	totalX /= g->numPoints(); totalY /= g->numPoints(); totalZ /= g->numPoints();
	cout << totalX << endl;
	cout << totalY << endl;
	cout << totalZ << endl;
}

void FourStepNorm::flipTest(UnstructuredGrid3D* g)
{
	using namespace alglib;

	float fx = 0; 
	float fy = 0;
	float fz = 0;
	for (int i = 0; i < g->numCells(); i++)
	{
		int vertexIndex[3];
		float V1[3];
		float V2[3];
		float V3[3];
		g->getCell(i, vertexIndex);
		g->getPoint(vertexIndex[0], V1);
		g->getPoint(vertexIndex[1], V2);
		g->getPoint(vertexIndex[2], V3);

		float avgX = (V1[0] + V2[0] + V3[0]) / 3; //get center coordinates of triangle
		float avgY = (V1[1] + V2[1] + V3[1]) / 3;
		float avgZ = (V1[2] + V2[2] + V3[2]) / 3;

		fx += sign(avgX) * (avgX * avgX); //sign(Cti)*(Cti)^2, where t is index of triangle and i is element of {x,y,z}
		fy += sign(avgY) * (avgY * avgY);
		fz += sign(avgZ) * (avgZ * avgZ);
	}

	//create matrix for flip test
	double ns[] = { sign(fx),	0,			0,
					0,			sign(fy),	0,
					0,			0,			sign(fz) };

	//Alglib data structures for matrix multiplication
	real_1d_array A; //vector to be translated
	real_2d_array B; //translation matrix
	real_1d_array C; //new vector
	A.setlength(3);
	C.setlength(3);
	B.setcontent(9, 3, ns);
	densesolverreport r; ae_int_t info; //need this for some reason
	
	for (int i = 0; i < g->numPoints(); i++)
	{	//Read the point and convert to real_1d_array, than do matrix multiplication to get new coordinate 
		float V[3];
		g->getPoint(i, V);
		A[0] = V[0];
		A[1] = V[1];
		A[2] = V[2];
		rmatrixsolve(B, 3, A, info, r, C); //Do matrix multiplication to get the new vertex coordinates
		float T[3] = { C[0],C[1],C[2] };
		g->setPoint(i, T);
	}
}

void FourStepNorm::centerOnBary(UnstructuredGrid3D* g)
{
	g->centerOnBary();
}

void FourStepNorm::normalizeInCube(UnstructuredGrid3D* g)
{
	float minX = 1.0e6, minY = 1.0e6, minZ = 1.0e6;
	float maxX = -1.0e6, maxY = -1.0e6, maxZ = -1.0e6;
	g->getBoundingBox(minX, maxX, minY, maxY, minZ, maxZ);

	float largestDim = max(maxX - minX, max(maxY - minY, maxZ - minZ));
	float scaleFactor = 1 / largestDim;

	cout << largestDim << endl;
	cout << scaleFactor << endl;
	for (int i = 0; i < g->numPoints(); ++i)							//3. Use the scaling factor computed above to scale all grid
	{														//   points in the [-0.5,0.5] cube
		float p[3];
		g->getPoint(i, p);

		p[0] = p[0] * scaleFactor;
		p[1] = p[1] * scaleFactor;
		p[2] = p[2] * scaleFactor;

		g->setPoint(i, p);
	}
}
