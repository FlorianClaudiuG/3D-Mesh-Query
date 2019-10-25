#include "include/MeshDecimation/SurfaceMesh.h"
#include "include/MeshDecimation/DifferentialGeometry.h"
#include "include/featureExtraction.h"
#include "include/PCA/dataanalysis.h"
#include "math.h"
#include <cmath>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>

using namespace pmp;

histFeature::histFeature(int n, float min, float max, float* values, int nvalues, string nName) {
	minVal = min; maxVal = max; nElements = n; name = nName;
	bins = new float[nElements] {0};
	

	float binsize = (maxVal - minVal) / nElements;
	int assignedValues = 0;//keep track of number assigned values

	for (int i = 0; i < nvalues; i++) {
		float threshold = minVal + binsize;
		bool assigned = false;
		int j = 0;
		while ((!assigned) & (j < nElements))
		{
			if ((values[i] <= threshold) & (values[i]>= minVal)) {
				bins[j] = bins[j] + 1;
				assigned = true;
				j++;
				assignedValues++;
			}
			else {
				threshold += binsize;
				j++;
			}
		}
	}
	for (int i = 0; i < nElements; i++) {
		bins[i] /= assignedValues;//normalize so that total value of histogram of 
	}
	delete[] values;//wont be needed anymore
	//cout << "should be: " << 1 << ", is really: " << nAssigned << endl;
};

histFeature::histFeature(int nbins, float* binValues) {
	nElements = nbins;
	minVal = 0; maxVal = 0;
	bins = new float[nbins] {0};
	for (int i = 0; i < nbins; i++) {
		bins[i] = binValues[i];
	}
}

gridFeatures::~gridFeatures(){
	for (int i = 0; i < nFeatures; i++){
		delete features[i];}
}

feature::~feature()
{
	delete[] bins;
}

string histFeature::getFeatureString()
{
	string s = "";
	for (int i = 0; i < nElements; i++) {
		s += to_string(bins[i]);
		s += ",";
	}
	return s;
}

gridFeatures::gridFeatures(UnstructuredGrid3D* g, int samples, int binsA3, int binsD1, int binsD2, int binsD3, int binsD4) {
	nFeatures = 10;
	features = new feature*[nFeatures];
	
	features[0] = surfaceArea(g);
	features[1] = boundingBoxVolume(g);
	features[2] = eccentricity(g);
	features[3] = compactness(g);
	features[4] = diameter(g);

	features[5] = A3(g, samples,binsA3);
	features[6] = D1(g, samples,binsD1);
	features[7] = D2(g, samples,binsD2);
	features[8] = D3(g, samples,binsD3);
	features[9] = D4(g, samples,binsD4);
}

gridFeatures::gridFeatures(string featString, int* binnrs, int nScalar, int nHist) {
	nFeatures = nScalar + nHist;
	features = new feature * [nFeatures];
	
	stringstream ss(featString);
	string item;//value to be added
	vector<std::string> splittedStrings;
	char delimeter = ',';

	for (int i = 0; i < nScalar; i++) {
		getline(ss, item, delimeter);
		features[i] = new scalarFeature(stof(item), "");
	}

	for (int i = nScalar; i < nHist+nScalar; i++) {
		int size = binnrs[i-nScalar];
		float* numbers = new float[size];
		for (int j = 0; j < size; j++) {
			getline(ss, item, delimeter);
			numbers[j] = stof(item);
		}
		features[i] = new histFeature(size, numbers);
	}
}

string gridFeatures::featuresToString() {
	string s = {};
	for (int i = 0; i < nFeatures; i++) {
		s += features[i]->getFeatureString();
	}
	return s;
}

bool checkDuplicates(int* v, int n) {
	for (int i = 0; i < n; i++) {
		for (int j = i + 1; j < n; j++) {
			if (v[i] == v[j]) {
				return true;
			}
		}
	}
	return false;
}

float standardization(float i, float avg, float stddev) {
	return ((i - avg) / stddev);
}

//calculate distan
float distancePoint(float* a, float* b) {
	Point pa = Point(a[0], a[1], a[2]);
	Point pb = Point(b[0], b[1], b[2]);
	float dist = distance(pa, pb);
	return dist;
}
//calculate angle between 3 points
float calculateAngle(float* a, float* b, float* c){
	Point pa = Point(b[0]-a[0], b[1]-a[1], b[2]-a[2]);
	Point pb = Point(c[0]-a[0], c[1]-a[1], c[2]-a[2]);
	float angl = angle(pa, pb);
	return angl;
}

//calculate area of triangle formed by points a,b ,c
float triangleArea(float* a, float* b, float* c) {
	Point pa = Point(a[0], a[1], a[2]);
	Point pb = Point(b[0], b[1], b[2]);
	Point pc = Point(c[0], c[1], c[2]);
	float area = triangle_area(pa, pb, pc);
	return area;
}

scalarFeature* surfaceArea(UnstructuredGrid3D* g){
	float totalArea = 0;
	for (int i = 0; i < g->numCells(); i++) {
		int vertexIndex[3];//indexes of points
		float a[3];
		float b[3];
		float c[3];
		g->getCell(i, vertexIndex);
		g->getPoint(vertexIndex[0], a);
		g->getPoint(vertexIndex[1], b);
		g->getPoint(vertexIndex[2], c);

		totalArea += triangleArea(a, b, c);
	}
	totalArea = standardization(totalArea, 1.53768275, 1.648700958);

	scalarFeature* f = new scalarFeature(totalArea, "SA");
	return f;
}

scalarFeature* boundingBoxVolume(UnstructuredGrid3D* g)
{
	float minX = 1.0e6, minY = 1.0e6, minZ = 1.0e6;
	float maxX = -1.0e6, maxY = -1.0e6, maxZ = -1.0e6;

	g->getBoundingBox(minX, maxX, minY, maxY, minZ, maxZ);
	float a = maxX - minX;
	float b = maxY - minY;
	float c = maxZ - minZ;
	float volume = (a * b * c);
	
	volume = standardization(volume, 0.261523862, 0.2203638);

	scalarFeature* f = new scalarFeature(volume, "BV");
	return f;
}

scalarFeature* eccentricity(UnstructuredGrid3D* g){
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
	float val = eigenValues[2] / eigenValues[0];

	val = standardization(val, 0.148089259, 0.182812364);

	scalarFeature* f = new scalarFeature(val, "Ecc");
	return f;
}

scalarFeature* compactness(UnstructuredGrid3D* g)
{	
	float totalArea = 0;
	for (int i = 0; i < g->numCells(); i++) {
		int vertexIndex[3];//indexes of points
		float a[3];
		float b[3];
		float c[3];
		g->getCell(i, vertexIndex);
		g->getPoint(vertexIndex[0], a);
		g->getPoint(vertexIndex[1], b);
		g->getPoint(vertexIndex[2], c);

		totalArea += triangleArea(a, b, c);
	}
	float A = totalArea;
	float n = g->numCells();
	float z = cbrt(n);
	float C = (n - (A / 6.0f)) / (n - (z * z));
	
	scalarFeature* f = new scalarFeature(C, "CO");
	return f;
}

scalarFeature* diameter(UnstructuredGrid3D* g) {
	scalarFeature* f = new scalarFeature(1, "di");//g->getDiameter(2000)
	return f;
}

float tetraArea(float* a, float* b, float* c, float* d) {
	Point pa = Point(a[0], a[1], a[2]);
	Point pb = Point(b[0], b[1], b[2]);
	Point pc = Point(c[0], c[1], c[2]);
	Point pd = Point(d[0], d[1], d[2]);
	
	//https://en.wikipedia.org/wiki/Tetrahedron general properties -> volume function
	float nom = abs(dot((pa - pd), (cross(pb - pd, pc - pd))));
	float V = nom / 6.0f;
	//cout << to_string(V) << endl;
	return V;
}

histFeature* D1(UnstructuredGrid3D* g, int n, int nBins) {
	int nPoints = g->numPoints();

	float* values = new float[n];//distance values
	srand(0);//set seed
	for (int i = 0; i < n; i++)
	{
		int newIndex = rand() % nPoints;

		float p[3]; //random vertex
		float o[3] = { 0,0,0 }; //barycenter
		g->getPoint(newIndex, p);
		
		float newValue = distancePoint(p,o);//calculate distance

		values[i] = newValue;
	}
	float tmax = sqrt(2);
	float binSize = tmax / 20;//20 bins each bin is 1/20
	histFeature* h = new histFeature(nBins, 0, binSize*15, values, n,"D1");//*15 because only first 15 bins used
	
	return h;
}

histFeature* D2(UnstructuredGrid3D* g, int n, int nBins) {
	int nPoints = g->numPoints();

	float* values = new float[n];//distance values
	srand(0);//set seed

	for (int i = 0; i < n; i++)
	{
		int newIndex1 = 0;
		int newIndex2 = 0;
		bool dup = true;
		while (dup) {
			newIndex1 = rand() % nPoints;
			newIndex2 = rand() % nPoints;
			int indexes[2] = { newIndex1,newIndex2 };
			dup = checkDuplicates(indexes, 2);
		}

		float p[3]; //random vertex
		float q[3];
		
		g->getPoint(newIndex1, p);
		g->getPoint(newIndex2, q);

		float newValue = distancePoint(p, q);//calculate distance
		values[i] = newValue;
	}
	
	histFeature* h = new histFeature(nBins, 0, sqrt(2), values, n,"D2");
	return h;
}

histFeature* A3(UnstructuredGrid3D* g, int n, int nBins) {
	int nPoints = g->numPoints();

	float* values = new float[n];//distance values
	srand(0);//set seed

	for (int i = 0; i < n; i++)
	{
		int newIndex1 = 0;
		int newIndex2 = 0;
		int newIndex3 = 0;
		bool dup = true;
		while (dup) {
			newIndex1 = rand() % nPoints;
			newIndex2 = rand() % nPoints;
			newIndex3 = rand() % nPoints;
			int indexes[3] = { newIndex1,newIndex2,newIndex3};
			dup = checkDuplicates(indexes, 3);
		}
		
		float p[3]; //random vertex
		float q[3];
		float v[3];
		g->getPoint(newIndex1, p);
		g->getPoint(newIndex2, q);
		g->getPoint(newIndex3, v);

		float newValue = max(calculateAngle(p, q, v),max(calculateAngle(q,p,v),calculateAngle(v,q,p)));//calculate distance
		values[i] = newValue;
	}
	
	double PI = 3.14159265358979323846;  /* pi */
	double binSize = PI / 20;
	histFeature* h = new histFeature(nBins, 6*binSize, PI, values, n, "A3");
	return h;
}

histFeature* D3(UnstructuredGrid3D* g, int n, int nBins) {
	int nPoints = g->numPoints();

	float* values = new float[n];//distance values
	srand(0);//set seed

	for (int i = 0; i < n; i++)
	{
		int newIndex1 = 0;
		int newIndex2 = 0;
		int newIndex3 = 0;
		bool dup = true;
		while (dup) {
			newIndex1 = rand() % nPoints;
			newIndex2 = rand() % nPoints;
			newIndex3 = rand() % nPoints;
			int indexes[3] = { newIndex1,newIndex2,newIndex3 };
			dup = checkDuplicates(indexes, 3);
		}

		float p[3]; //random vertex
		float q[3];
		float v[3];
		g->getPoint(newIndex1, p);
		g->getPoint(newIndex2, q);
		g->getPoint(newIndex3, v);

		float newValue = sqrt(triangleArea(p,q,v));//
		values[i] = newValue;
	}
	float tmax = sqrt(2);
	float binSize = tmax / 20;
	histFeature* h = new histFeature(nBins, 0, binSize*12, values, n, "D3");//*12 because takes first 12 bins
	return h;
}

histFeature* D4(UnstructuredGrid3D* g, int n, int nBins)
{
	int nPoints = g->numPoints();

	float* values = new float[n];//distance values
	srand(0);//set seed

	for (int i = 0; i < n; i++)
	{
		int newIndex1 = 0;
		int newIndex2 = 0;
		int newIndex3 = 0;
		int newIndex4 = 0;
		bool dup = true;
		while (dup) {
			newIndex1 = rand() % nPoints;
			newIndex2 = rand() % nPoints;
			newIndex3 = rand() % nPoints;
			newIndex4 = rand() % nPoints;

			int indexes[4] = { newIndex1,newIndex2,newIndex3 ,newIndex4};
			dup = checkDuplicates(indexes, 4);
		}
		float p[3]; //random vertex
		float q[3];
		float v[3];
		float w[3];
		g->getPoint(newIndex1, p);
		g->getPoint(newIndex2, q);
		g->getPoint(newIndex3, v);
		g->getPoint(newIndex4, w);


		float newValue = cbrt(tetraArea(p, q, v,w));//
		values[i] = newValue;
	}
	float tmax = sqrt(2);
	float binSize = tmax / 20;
	histFeature* h = new histFeature(nBins, 0, binSize*8, values, n, "D4");
	return h;
}

scalarFeature::scalarFeature(float v, string nName){
	name = nName;
	nElements = 1;
	val = v;
}

string scalarFeature::getFeatureString() {
	string s = to_string(val);
	s += ",";
	return s;
}

float FeatureDistanceScalar(feature* f1, feature* f2)
{
	float d = abs(f1->val - f2->val);
	return d;
}

float FeatureDistanceHist(feature* f1, feature* f2)
{
	float d = 0;
	for (int i = 0; i < f1->nElements; i++) {
		float v1 = f1->bins[i];
		float v2 = f2->bins[i];
		float val = abs(v1 - v2);
		d += val*val;
	}
	//d = d / 2.0;//maximum distance between two normalized is 2. diving by 2 means that max distance is 1, 0-1 is the range in which
	//most of the features are calculated.
	return sqrt(d);
}

float getFeatureDistance(feature* f1, feature* f2)
{
	if (f1->nElements == 1) {
		float  d = FeatureDistanceScalar(f1, f2);
		return d*d;
	}
	else {
		float  d = FeatureDistanceHist(f1, f2);
		return d*d;
	}
}

float featureVectorDistance(gridFeatures* fv1, gridFeatures* fv2)
{
	float d = 0;
	for (int i = 0; i < fv1->nFeatures; i++) {
		d += getFeatureDistance(fv1->features[i], fv2->features[i]);
		//cout <<to_string(i) << ": " <<  to_string(getFeatureDistance(fv1->features[i], fv2->features[i])) << endl;
	}
	return sqrt(d);
}