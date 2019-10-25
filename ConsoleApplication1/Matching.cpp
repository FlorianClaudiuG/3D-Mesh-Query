#include "include/UnstructuredGrid3D.h"
#include "include/OffReader.h"
#include <experimental/filesystem>							
#include "include/featureExtraction.h"
#include "include/Matching.h"
#include "PSBClaParse.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>

void labelData::addValue(float f) {
	value += (f / nModels);
}

void labelData::addPrecision(float p) {
	precision += (p / (obsModels+1));//divides by total number of models so that the total is the average
}

void labelData::addRecall(float r) {
	recall += (r / (obsModels+1));
}

void matchAll(string databaseLocation) {
	namespace fs = std::experimental::filesystem;
	UnstructuredGrid3D* mesh;
	OffReader ofr;

	char classFilePath1[50] = "../classification/v1/coarse1/coarse1.cla";
	char classFilePath2[50] = "../classification/v1/coarse1/coarse1Train.cla";

	map<string, string> modelClasses; //filename -> labelname
	map<string, labelData> labels; //labelname -> labeldata

	PSBCategoryList* categories = parseFile(classFilePath1);

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

	for (const auto& entry : fs::directory_iterator(databaseLocation))
	{
		string directoryName = entry.path().filename().string();
		for (const auto& entry2 : fs::directory_iterator(entry.path()))
		{
			string directoryName2 = entry2.path().filename().string();
			string pathName = entry2.path().string();
			string fileName = pathName + "\\" + directoryName2 + ".off";
			cout << fileName << endl;
			mesh = ofr.ReadOffFile(fileName.c_str());

			float prec;
			float rec;
			matchGrid(mesh, modelClasses, labels, prec, rec, directoryName2);
			cout << "precision: " << prec << "| recall: " << rec << endl;
			labels[modelClasses[directoryName2]].addPrecision(prec);
			labels[modelClasses[directoryName2]].addRecall(rec);

			delete mesh;
		}
	}

	for (int i = 0; i < categories->_numCategories; i++)
	{
		string labelName = categories->_categories[i]->_name;
		cout << labelName << ": " << labels[labelName].precision << " " << labels[labelName].recall << " nModels " << labels[labelName].nModels << endl;
	}
}

void matchGrid(UnstructuredGrid3D* g, map<string, string> &modelClasses, map<string, labelData> &labels, float &prec, float &rec, string inputName){
	string inputLable = modelClasses[inputName];
	int totalDistances = 0;//intialize to 0 for counting necessary because missing some models in our database
	labels[inputLable].obsModels = 0;//initialize to 0 for counting

	vector<distanceObject> distances = getDistances(g, "output/featureTable.txt", modelClasses, labels, inputName, totalDistances);
	int totalInLable = labels[inputLable].obsModels;
	
	float totalPrec = 0;
	float totalRec = 0;
	for (int i = 1; i <= totalInLable; i++) {
		float precision;
		float recall;
		calculateAccuracy(distances, inputLable, totalInLable, totalDistances, i, precision, recall);
		totalPrec += precision;
		totalRec += recall;
	}
	totalPrec /= (totalInLable - 1.0f);
	totalRec /= (totalInLable -1.0f);
	/*
	float precision;
	float recall;
	calculateAccuracy(distances, inputLable, totalInLable, totalDistances, totalInLable, precision, recall);*/
	cout << totalInLable << " " << totalDistances << endl;
	prec = totalPrec;
	rec = totalRec;
}

vector<distanceObject> getDistances(UnstructuredGrid3D* g, string tableLocation, map<string,string> labelDict, map<string,labelData> &labels, string inputName, int &totalDistances) {
	ifstream infile(tableLocation);
	string line;
	string name;
	char delimeter = ',';

	vector<distanceObject> distanceList;//store all distances with names

	gridFeatures* f1 = new gridFeatures(g, 20000, 12, 12, 12, 12, 12);

	while (getline(infile, line))
	{
		stringstream ss(line);
		getline(ss, name, delimeter);
		getline(ss, line);

		if (name == inputName)//ignore the file itself (distance would always be 0)
			continue;

		int lengths[5] = { 12,12,12,12,12 };
		gridFeatures* f2 = new gridFeatures(line, lengths, 5, 5);

		float d = featureVectorDistance(f1, f2);

		distanceList.push_back(distanceObject(name, labelDict[name], d));

		labels[labelDict[name]].addValue(d);
		labels[labelDict[name]].obsModels++;
		totalDistances++;

		delete f2;
	}

	/*add an additional feature, average of distance to the label
	for (vector<distanceObject>::iterator it = distanceList.begin(); it != distanceList.end(); ++it) {
		it->dValue += (labels[labelDict[it->name]].value / 11);
	}*/

	sort(distanceList.begin(),distanceList.end());

	delete f1;
	//printAllDistance(distanceList);
	return distanceList;
}

void calculateAccuracy(vector<distanceObject> &distances, string inputLabel, int numberInClass, int totalDistances, int querySize, float &precision, float &recall)
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
