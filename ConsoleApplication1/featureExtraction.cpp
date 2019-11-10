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

//----------------------------------------------------------------------
//	Histfeature object functions.
//----------------------------------------------------------------------

//constructor. given number of bins (n) the minimum (min) and maximum (max) of the histogram. Fill a histogram
//with an array of float values.
histFeature::histFeature(int n, float min, float max, float* values, int nvalues, string nName) {
	minVal = min; maxVal = max; nElements = n; name = nName;
	
	bins = new float[nElements] {0};

	float binsize = (maxVal - minVal) / nElements;	//each bin should have an equal range
	int assignedValues = 0;							//keep track of number assigned values

	for (int i = 0; i < nvalues; i++) {//for each value determine what bin it belongs to
		float threshold = minVal + binsize;	//threshold for bin nr 1.
		bool assigned = false;		
		int j = 0;
		while ((!assigned) & (j < nElements))		// loop until value is assigned, or the bins run out
		{
			if ((values[i] <= threshold) & (values[i]>= minVal)) {	//if value is below threshold it is the correct bin.
				bins[j] = bins[j] + 1;								//value is assigned to bin j
				assigned = true;
				j++;
				assignedValues++;
			}
			else {	// if it doesnt fit in the bin increase the threshold to check the next bin
				threshold += binsize;
				j++;
			}
		}
	}
	for (int i = 0; i < nElements; i++) {
		bins[i] /= assignedValues;	//normalize so that total value of histogram of 
	}
	delete[] values;	// value assigned so wont be needed anymore

	if (assignedValues != nvalues) {
		cout << "value didnt fit in bin, should be " << nvalues << ", is really " << assignedValues << endl;
	}
};

//Histfeature constructor for when the binValues are already known.
histFeature::histFeature(int nbins, float* binValues) {
	nElements = nbins;
	minVal = 0; maxVal = 0;
	bins = new float[nbins] {0};
	for (int i = 0; i < nbins; i++) {
		bins[i] = binValues[i];
	}
}

//convert all the bin values to strings and seperate them with comma's
string histFeature::getFeatureString()
{
	string s = "";
	for (int i = 0; i < nElements; i++) {
		s += to_string(bins[i]);
		s += ",";
	}
	return s;
}

//destructor for features, delete all the bins
feature::~feature()
{
	delete[] bins;
}

//----------------------------------------------------------------------
//	Scalarfeature object functions.
//----------------------------------------------------------------------

//construct a scalar feature for a value and name
scalarFeature::scalarFeature(float v, string nName) {
	name = nName;
	nElements = 1;
	val = v;
}

//convert feature value to string
string scalarFeature::getFeatureString() {
	string s = to_string(val);
	s += ",";
	return s;
}

//----------------------------------------------------------------------
//	gridFeatures object functions.
//----------------------------------------------------------------------

//destructor for gridFeatures, delete all feature objects
gridFeatures::~gridFeatures(){
	for (int i = 0; i < nFeatures; i++){
		delete features[i];}
}

//construct a gridFeature object for a mesh (g) with a given number of samples for the histogram features and
//bin sizes for the different histogram features.
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

//construct a gridFeatures object given an already calculated feature string with the number of bins
//the number of scalar values and number of histogram values
gridFeatures::gridFeatures(string featString, int* binnrs, int nScalar, int nHist) {
	nFeatures = nScalar + nHist;
	features = new feature * [nFeatures];
	
	stringstream ss(featString);
	string item;							//value to be added
	char delimiter = ',';

	for (int i = 0; i < nScalar; i++) {
		getline(ss, item, delimiter);		//get the next value from the string stream
		features[i] = new scalarFeature(stof(item), "");
	}

	for (int i = nScalar; i < nHist+nScalar; i++) {
		int size = binnrs[i-nScalar];
		float* numbers = new float[size];	//array for storing the bins
		for (int j = 0; j < size; j++) {
			getline(ss, item, delimiter);	//get the next value for bin nr j
			numbers[j] = stof(item);
		}
		features[i] = new histFeature(size, numbers);
	}
}

//return the complete feature string.
string gridFeatures::featuresToString() {
	string s = {};
	for (int i = 0; i < nFeatures; i++) {
		s += features[i]->getFeatureString();
	}
	return s;
}

//----------------------------------------------------------------------
//	Helper functions for creating the feature objects.
//----------------------------------------------------------------------

//returns true if any values in the array are identical, false otherwise.
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

//standardizes a value given the average and standard deviation of its distribution
float standardization(float i, float avg, float stddev) {
	return ((i - avg) / stddev);
}

//calculate distance  between two points
float distancePoint(float* a, float* b) {
	Point pa = Point(a[0], a[1], a[2]);
	Point pb = Point(b[0], b[1], b[2]);
	float dist = distance(pa, pb);
	return dist;
}

//calculate angle between the lines a-b and a-c
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

//calculate the volume of a tetrahedron formed by points a,b,c,d
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

//----------------------------------------------------------------------
//	Scalar features. each function returns a scalarFeature object.
//----------------------------------------------------------------------

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

	val = standardization(val, 0.148089259, 0.182812364);//hardcoded standardization values gained from analyzing the feature table

	scalarFeature* f = new scalarFeature(val, "Ecc");
	return f;
}

scalarFeature* compactness(UnstructuredGrid3D* g)
{	
	float C = g->computeCircularity();
	
	C = standardization(C, 0.075533213, 0.137364272);
	
	scalarFeature* f = new scalarFeature(C, "CO");
	return f;
}

scalarFeature* diameter(UnstructuredGrid3D* g) {
	float d = max(g->getDiameter(2000,0),g->getDiameter(2000,1));
	d = max(d, g->getDiameter(2000, 2));
	d = max(d, g->getDiameter(2000, 3));
	d = max(d, g->getDiameter(2000, 4));
	
	d = standardization(d, 1.055276205, 0.088772773);

	scalarFeature* f = new scalarFeature(d, "di");//get diameter for 2000 sample points of the grid
	return f;
}

//----------------------------------------------------------------------
//	Histogram features. Each function returns a histFeature object
//----------------------------------------------------------------------

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
	float binSize = tmax / 20.0f;//20 bins each bin is 1/20
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
	histFeature* h = new histFeature(nBins, binSize*6, PI, values, n, "A3");
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

//----------------------------------------------------------------------
//	Distance calculation between features.
//----------------------------------------------------------------------

// returns distance between two scalar features
float FeatureDistanceScalar(feature* f1, feature* f2)
{
	float d = abs(f1->val - f2->val);
	return d;
}

// returns distance between two histogram features
float FeatureDistanceHist(feature* f1, feature* f2)
{
	float d = 0;
	for (int i = 0; i < f1->nElements; i++) {
		float v1 = f1->bins[i];
		float v2 = f2->bins[i];
		float val = abs(v1 - v2);
		d += val*val;
	}
	return sqrt(d);
}

// returns distance between two features
float getFeatureDistance(feature* f1, feature* f2)
{
	if (f1->nElements == 1) {// if n elements is 1 its a scalar feature
		float  d = FeatureDistanceScalar(f1, f2);
		return d*d;
	}
	else {
		float  d = FeatureDistanceHist(f1, f2);
		return d*d;
	}
}

//returns the distance between two feature vectors (represented as gridFeatures objects given a weight vector
float featureVectorDistance(gridFeatures* fv1, gridFeatures* fv2, float* weights)
{
	float d = 0;
	for (int i = 0; i < fv1->nFeatures; i++) {
		d += (weights[i] * getFeatureDistance(fv1->features[i], fv2->features[i]));
	}
	return sqrt(d);
}