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

gridMatcher::gridMatcher(string tableLocation) {
	readLabelData();
	readFeatureTable(nGrids, tableLocation);

	knn = new KNNBuilder(nGrids, nDims, grids, nGrids);
}

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

void gridMatcher::readFeatureTable(int n, string tableLocation){
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

void labelData::addValue(float f) {
	value += (f / nModels);
}

void labelData::addPrecision(float p) {
	//add precision of a model to its class
	precision += p;
}

void labelData::addRecall(float r) {
	//add recall of a model to its class
	recall += r;
}

void gridMatcher::matchAll(string databaseLocation, float* weights) {
	namespace fs = std::experimental::filesystem;
	UnstructuredGrid3D* mesh; //initalize mesh
	OffReader ofr; //initialize offreader

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

			float prec;//average precision over several query sizes
			float rec;//average recall over several query sizes

			matchGrid(mesh, prec, rec, directoryName2, weights);//matchgrid with entire dataset
			cout << "precision: " << prec << "| recall: " << rec << endl;

			labels[modelClasses[directoryName2]].addPrecision(prec);
			labels[modelClasses[directoryName2]].addRecall(rec);

			delete mesh;
		}
	}

	float totalPrec = 0;
	float totalRec = 0;
	float numberOfClasses = 0;
	float totalInClasses = 0;
	for (int i = 0; i < categories->_numCategories; i++)
	{
		string labelName = categories->_categories[i]->_name;
		int nObsModels = labels[labelName].obsModels;
		if (nObsModels > 0) {
			labels[labelName].precision /= nObsModels;
			labels[labelName].recall /= nObsModels;

			totalPrec += labels[labelName].precision * nObsModels;
			totalRec += labels[labelName].recall * nObsModels;
			totalInClasses += nObsModels;

			numberOfClasses++;

			cout << labelName << ": " << labels[labelName].precision << " " << labels[labelName].recall << " nModels: " << labels[labelName].nModels << " obsModels: " << labels[labelName].obsModels << endl;
		}
	}
	cout << "MAP: " << totalPrec / totalInClasses << endl;
}

void gridMatcher::matchGrid(UnstructuredGrid3D* g, float &prec, float &rec, string inputName, float* weights){
	string inputLable = modelClasses[inputName];
	int totalDistances = 0;//intialize to 0 for counting necessary because missing some models in our database
	int totalInLable = 0;//initialize to 1 for counting classes in labels (grid itself is part of its own class.
	
	vector<distanceObject> distances = getKNNDistances(g, "output/featureFinal.csv", inputName, totalDistances, totalInLable, weights);
	
	float avgPrec = calculateAVP(distances, inputLable, totalInLable, totalDistances);

	//wrong implementation MAP
	float totalPrec = 0;
	float totalRec = 0;
	for (int i = 1; i <= totalInLable; i++) {
		float precision;
		float recall;
		calculateAccuracy(distances, inputLable, totalInLable, totalDistances, i, precision, recall);
				totalPrec += precision;
		totalRec += recall;
	}
	totalPrec /= (totalInLable - 1.0f);//k different query sizes where k is all the class of a label, except the model itself, therefore total-1
	totalRec /= (totalInLable -1.0f);

	cout << totalInLable << " " << totalDistances << endl;
	prec = avgPrec;//MAP
	rec = totalRec;
}

vector<distanceObject> gridMatcher::getDistances(UnstructuredGrid3D* g, string tableLocation, string inputName, int &totalDistances, int &totalInLable, float* weights) {
	vector<distanceObject> distanceList;//store all distances with names

	gridFeatures* f1 = new gridFeatures(g, 100000, 12, 12, 12, 12, 12);

	for (int i = 0; i < nGrids; i++) {
		string name = grids[i]->modelName;
		if (name == inputName)//ignore the file itself (distance would always be 0)
			continue;

		float d = featureVectorDistance(f1, grids[i], weights);
		distanceList.push_back(distanceObject(name, modelClasses[name], d));

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

vector<distanceObject> gridMatcher::getKNNDistances(UnstructuredGrid3D* g, string tableLocation, string inputName, int& totalDistances, int& totalInLable, float* weights) {
	vector<distanceObject> distanceList;//store all distances with names

	gridFeatures* f1 = new gridFeatures(g, 100000, 12, 12, 12, 12, 12);
	ANNidxArray results = knn->KNNSearch(f1, nGrids);
	for (int i = 0; i < nGrids; i++) {
		int index = results[i];//index of the ith element of the resulting distance list
		string name = grids[index]->modelName;

		if (name == inputName)//ignore the file itself (distance would always be 0)
			continue;
		distanceList.push_back(distanceObject(name, modelClasses[name], 0));
		if (modelClasses[name] == modelClasses[inputName]) {
			totalInLable++;
		}
		totalDistances++;
	}

	return distanceList;
}


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
	float accuracy = (float)(truePositives + trueNegative) / (float)(truePositives + trueNegative + falsePositives + falseNegative);
	/*cout << "TP: " << truePositives << endl;
	cout << "FP: " << falsePositives << endl;
	cout << "TN: " << trueNegative << endl;
	cout << "FN: " << falseNegative << endl;
	cout << "precision: " << precision << " / recall: " << recall << endl;
	cout << "accuracy: " << accuracy << endl;*/
	precision = prec;
	recall = rec;
}

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

bool distanceObject::operator<(distanceObject const& a)
{
	return (dValue < a.dValue);
}

void printAllDistance(vector<distanceObject>& dists) {
	for (vector<distanceObject>::iterator it = dists.begin(); it != dists.end(); ++it) {
		it->printDistance();
	}
}

void distanceObject::printDistance()
{
	cout << name << "/" << label << ": " << dValue << endl;
}
