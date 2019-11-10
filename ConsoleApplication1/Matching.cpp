#include "include/UnstructuredGrid3D.h"
#include "include/OffReader.h"
#include <experimental/filesystem>							
#include "include/featureExtraction.h"
#include "include/Matching.h"
#include "PSBClaParse.h"
#include <iostream>
#include <fstream>
#include "include/Kdd.h"
#include <sstream>
#include <string>
#include <map>
#include "include/Matching.h"

//----------------------------------------------------------------------
//	gridMatcher constructor
//----------------------------------------------------------------------

gridMatcher::gridMatcher(string ntableLocation, string ndatabaseLocation, float* tweights) {
	tableLocation = ntableLocation;
	databaseLocation = ndatabaseLocation;
	weights = tweights;
	
	readLabelData();
	readFeatureTable(nGrids);
	getFileLocations();

	knn = new KNNBuilder(nGrids, nDims, grids, nGrids, weights);
}

//----------------------------------------------------------------------
// Initialize important data for matching.
//----------------------------------------------------------------------

//read all the data about the different labels
void gridMatcher::readLabelData(){
	//arrays for reading class lables
	char classFilePath1[50] = "../classification/v1/coarse1/coarse1.cla";
	char classFilePath2[50] = "../classification/v1/coarse1/coarse1Train.cla";

	//read all the class lables and add information to modelClasses and labels
	categories = parseFile(classFilePath1);
	for (int i = 0; i < categories->_numCategories; i++)
	{
		string labelName = categories->_categories[i]->_name;
		int nModels = categories->_categories[i]->_numModels;

		labels[labelName] = labelData(0, nModels);
		for (int j = 0; j < nModels; j++)
		{
			int intName = categories->_categories[i]->_models[j];
			string cName = "m" + to_string(intName);
			modelClasses[cName] = labelName;
		}
	}
}

//read in the feature table
void gridMatcher::readFeatureTable(int n){
	ifstream infile(tableLocation);
	string line;
	string name;
	char delimeter = ',';

	grids = new gridFeatures*[nGrids];

	int index = 0;
	while (getline(infile, line))
	{
		stringstream ss(line);
		getline(ss, name, delimeter);
		getline(ss, line);

		int lengths[5] = { 12,12,12,12,12 };
		gridFeatures* f2 = new gridFeatures(line, lengths, 5, 5);
		f2->modelName = name;
		
		grids[index] = f2;
		index++;
	}

}

//retrieves all paths to the models, used for displaying query results.
void  gridMatcher::getFileLocations() {
	namespace fs = std::experimental::filesystem;

	//go through all models in the dataset
	for (const auto& entry : fs::directory_iterator(databaseLocation))
	{
		string directoryName = entry.path().filename().string();
		for (const auto& entry2 : fs::directory_iterator(entry.path()))
		{
			//read the file for this folder
			string directoryName2 = entry2.path().filename().string();//filename, i.e. "m303"
			string pathName = entry2.path().string();
			string fileName = pathName + "\\" + directoryName2 + ".off";

			filePaths[directoryName2] = fileName;
		}
	}
}

//----------------------------------------------------------------------
//	Adding observed performance values for labels
//----------------------------------------------------------------------
void labelData::addValue(float f) {
	value += (f / nModels);
}

void labelData::addPrecision(float p) {
	//add precision of a model to its class
	precision += p;
}

void labelData::addprecisionAtk(float r) {
	//add recall of a model to its class
	precisionAtk += r;
}

//----------------------------------------------------------------------
//	Matching functions for single mesh or entire database
//----------------------------------------------------------------------

//matches each model in the database to all different models and prints the evaluation results in console
//the k parameter is used for the performance metric 'precision at k'
void gridMatcher::matchAll(float* weights, int k) {
	namespace fs = std::experimental::filesystem;
	UnstructuredGrid3D* mesh;			//initalize mesh
	OffReader ofr;						//initialize offreader

	//go through all models in the dataset
	for (const auto& entry : fs::directory_iterator(databaseLocation))
	{
		string directoryName = entry.path().filename().string();
		for (const auto& entry2 : fs::directory_iterator(entry.path()))
		{
			//read the file for this folder
			string directoryName2 = entry2.path().filename().string();//filename, i.e. "m303"
			string pathName = entry2.path().string();
			string fileName = pathName + "\\" + directoryName2 + ".off";
			cout << fileName << endl;
			mesh = ofr.ReadOffFile(fileName.c_str());

			string labelName = modelClasses[directoryName2];
			labels[labelName].obsModels++;//count this model in the relevant class

			float prec;				//average precision for the total query result for mesh
			float precisionAtk;		//precision for the first k models in the result query

			matchGrid(mesh, prec, precisionAtk, directoryName2, weights, k);//matchgrid with entire dataset
			cout << "avgP: " << prec << "| precAtK: " << precisionAtk << endl;

			labels[modelClasses[directoryName2]].addPrecision(prec);
			labels[modelClasses[directoryName2]].addprecisionAtk(precisionAtk);

			delete mesh;
		}
	}

	float totalPrec = 0;
	float totalprecisionAtk = 0;
	float numberOfClasses = 0;
	float totalInClasses = 0;
	for (int i = 0; i < categories->_numCategories; i++)
	{
		string labelName = categories->_categories[i]->_name;
		int nObsModels = labels[labelName].obsModels;
		if (nObsModels > 0) {
			labels[labelName].precision /= nObsModels;
			labels[labelName].precisionAtk /= nObsModels;

			totalPrec += labels[labelName].precision * nObsModels;
			totalprecisionAtk += labels[labelName].precisionAtk * nObsModels;
			
			totalInClasses += nObsModels;
			numberOfClasses++;

			cout << labelName << " & " << labels[labelName].precision << " & " << labels[labelName].precisionAtk << " & " << labels[labelName].obsModels << " \\\\ \\hline" << endl;
			//cout << labelName << ": " << labels[labelName].precision << " " << labels[labelName].precisionAtk << " nModels: " << labels[labelName].nModels << " obsModels: " << labels[labelName].obsModels << endl;
		}
	}
	cout << totalInClasses << " " << numberOfClasses << endl;
	cout << "MAP: " << totalPrec / totalInClasses << " | Precision at k: " << totalprecisionAtk /totalInClasses << endl;
}

//match a single grid against the rest of the database. return the k nearest neighbours
vector<distanceObject> gridMatcher::matchSingle(UnstructuredGrid3D* g, string inputName, int k)
{
	int totalDistances, totalLabel;
	return getKNNDistances(g, tableLocation, inputName, totalDistances, totalLabel, k);
}

//----------------------------------------------------------------------
//	MatchGrid private function used by matchall
//----------------------------------------------------------------------

//matches a grid with the rest of the dataSet and calculates the MAP and precision at k
void gridMatcher::matchGrid(UnstructuredGrid3D* g, float &prec, float &precisionAtk, string inputName, float* weights, int k){
	string inputLable = modelClasses[inputName];
	int totalDistances = 0;//intialize to 0 for counting necessary because missing some models in our database
	int totalInLable = 0;//initialize to 1 for counting classes in labels (grid itself is part of its own class.
	
	//vector<distanceObject> distances = getKNNDistances(g, "output/featureFinal.csv", inputName, totalDistances, totalInLable, weights);
	vector<distanceObject> distances = getDistances(g, "output/featureFinal.csv", inputName, totalDistances, totalInLable);

	//average precision for MAP calculation
	float avgPrec = calculateAVP(distances, inputLable, totalInLable, totalDistances);

	//precision at k for second performance measure
	float totalRec = 0;
	calculateAccuracy(distances, inputLable, totalInLable, totalDistances,k,precisionAtk,totalRec);

	cout << totalInLable << " " << totalDistances << endl;
	prec = avgPrec;//MAP
}

//----------------------------------------------------------------------
//	Retrieving the list of distances
//----------------------------------------------------------------------

//getDistances retrieves distances to all models. Does not use a KD-tree because all distances are required.
vector<distanceObject> gridMatcher::getDistances(UnstructuredGrid3D* g, string tableLocation, string inputName, int &totalDistances, int &totalInLable) {
	vector<distanceObject> distanceList;//store all distances with names

	gridFeatures* f1 = new gridFeatures(g, 100000, 12, 12, 12, 12, 12);

	for (int i = 0; i < nGrids; i++) {
		string name = grids[i]->modelName;
		if (name == inputName)//ignore the file itself (distance would always be 0)
			continue;

		float d = featureVectorDistance(f1, grids[i], weights);
		distanceList.push_back(distanceObject(name, modelClasses[name], d,filePaths[name]));

		labels[modelClasses[name]].addValue(d);
		if (modelClasses[name] == modelClasses[inputName]) {
			totalInLable++;
		}
		totalDistances++;
	}
	sort(distanceList.begin(),distanceList.end());

	delete f1;
	return distanceList;
}

//retrieves the k nearest neighbours using the initialized kd-tree
vector<distanceObject> gridMatcher::getKNNDistances(UnstructuredGrid3D* g, string tableLocation, string inputName, int& totalDistances, int& totalInLable, int k) {
	vector<distanceObject> distanceList;//store all distances with names

	gridFeatures* f1 = new gridFeatures(g, 100000, 12, 12, 12, 12, 12);
	ANNdistArray distResults;
	ANNidxArray indexResults;
	knn->KNNSearch(f1, k,distResults,indexResults);
	for (int i = 0; i < k; i++) {
		int index = indexResults[i];//index of the ith element of the resulting distance list
		string name = grids[index]->modelName;

		if (name == inputName)//ignore the file itself (distance would always be 0)
			continue;
		distanceList.push_back(distanceObject(name, modelClasses[name], distResults[i],filePaths[name]));
		if (modelClasses[name] == modelClasses[inputName]) {
			totalInLable++;
		}
		totalDistances++;
	}
	delete f1;
	return distanceList;
}

//----------------------------------------------------------------------
//	performance metric calculations
//----------------------------------------------------------------------

//calculate the precision at k
void gridMatcher::calculateAccuracy(vector<distanceObject> &distances, string inputLabel, int numberInClass, int totalDistances, int querySize, float &precision, float &recall)
{
	int truePositives = 0;
	
	for (int i = 0; i < querySize; i++) {
		if (distances[i].label == inputLabel)
			truePositives++;
	}
	int falsePositives = querySize - truePositives;//remaining classified are the false positives
	int falseNegative = numberInClass - truePositives;
	int trueNegative = totalDistances - falseNegative - falsePositives - truePositives;
	float prec = (float)truePositives / (float)(truePositives + falsePositives);
	float rec = (float)truePositives / (float)(truePositives + falseNegative);

	precision = prec;
	recall = rec;
}

//calculate the average precision
float gridMatcher::calculateAVP(vector<distanceObject>& distances, string inputLabel, int numberInCLass, int totalDistances)
{
	int index = 0;
	int inClassFound = 0;
	float avgPrec = 0;
	while ((index < totalDistances) & (inClassFound < numberInCLass)) {
		if (distances[index].label == inputLabel) {
			inClassFound++;
			int TP = inClassFound;
			int FP = index + 1 - TP;
			float prec = (float)TP / float(TP + FP);
			avgPrec += prec;
			//cout << "TP " << TP << "|FP " << FP << "|total " << index+1 << "|prec " << prec << endl;
		}
		index++;
	}
	avgPrec /= (float)inClassFound;
	return avgPrec;
}

//----------------------------------------------------------------------
//	distanceObject member functions
//----------------------------------------------------------------------

// overloaded operator for sorting a list of distanceObjects
bool distanceObject::operator<(distanceObject const& a)
{
	return (dValue < a.dValue);
}

//print the distance for a distance object
void distanceObject::printDistance()
{
	cout << name << "/" << label << ": " << dValue << endl;
}
//print all the distances in a query result
void printAllDistance(vector<distanceObject>& dists) {
	for (vector<distanceObject>::iterator it = dists.begin(); it != dists.end(); ++it) {
		it->printDistance();
	}
}




