#include <GL/freeglut.h>									//GLUT library
#include <experimental/filesystem>							//for looping through the database directories 
#include <iostream>
#include <fstream>
#include "include/UnstructuredGrid3D.h"							//the 3D unstructured grid class
#include "include/MeshRenderer.h"								//a simple renderer for 3D unstructured grids, demonstrating flat/smooth shading
#include "include/PlyReader.h"									//a reader that initializes UnstructuredGrid3D objects with meshes stored in the PLY file format
#include "include/zpr.h"										//library for interactively manipulating the OpenGL camera (viewpoint) using the mouse
#include "include/meshDescription.h"
#include "OFFConverter.h"
#include "PlyWriter.h"
#include "include/OffReader.h"
#include "include/FourStepNorm.h"
#include "Supersampler.h"
#include "PSBClaParse.h"
#include "include/Matching.h"
#include "include/featureExtraction.h"
#include "include/Kdd.h"
#include "math.h"
#include <cmath>
#include <algorithm>

//Files for pmp (used for mesh decimation)
#include "include/MeshDecimation/SurfaceMesh.h"
#include "include/MeshDecimation/SurfaceMeshIO.h"
#include "include/MeshDecimation/SurfaceSimplification.h"

float fov = 80;										//Perspective projection parameters
float z_near = 0.1;
float z_far = 30;

MeshRenderer		renderer;
UnstructuredGrid3D* grid = 0;
int					drawing_style = 0;
int					modelToDraw = 0;//index of the model of the 

vector<distanceObject> resultQuery;

void viewing(int W, int H)								//Window resize function, sets up viewing parameters (GLUT callback function)
{
	glMatrixMode(GL_PROJECTION);						//1. Set the projection matrix
	glLoadIdentity();
	gluPerspective(fov, float(W) / H, z_near, z_far);

	glViewport(0, 0, W, H);								//2. Set the viewport to the entire size of the rendering window
}



void mouseclick(int button, int state, int x, int y)	//Callback for mouse click events. We use these to control the viewpoint interactively.
{
	int keys = glutGetModifiers();						//The mouse events are interpreted as follows:	
	if (keys & GLUT_ACTIVE_CTRL)							// 
		button = GLUT_MIDDLE_BUTTON;						// -left button + move:                         rotate viewpoint
	if (keys & GLUT_ACTIVE_SHIFT)							// -middle button (or left button+Control key): zoom in / out
		button = GLUT_RIGHT_BUTTON;							// -right buttom (or left button+Shift key):    translate viewpoint

	zprMouse(button, state, x, y);							//Use the ZPR library to manipulate the viewpoint. The library sets the modelview  
														  //OpenGL matrix based on the mouse events, thereby changing the current viewpoint.
}

void mousemotion(int x, int y)							//Callback for mouse move events. We use these to control the viewpoint interactively.
{
	zprMotion(x, y);										//Pass the current location of the mouse to the ZPR library, to change the viewpoint.

	glutPostRedisplay();									//After each mouse move event, ask GLUT to redraw our window, so we see the viewpoint change.
}

void setCamera()
{
	float eye_x = 0, eye_y = 0.2, eye_z = 1.5f;				//Modelview (camera extrinsic) parameters
	float c_x = 0, c_y = 0, c_z = 0;
	float up_x = 0, up_y = 0, up_z = -1;
	glMatrixMode(GL_MODELVIEW);							//1. Set the modelview matrix (including the camera position and view direction)
	glLoadIdentity();
	gluLookAt(eye_x, eye_y, eye_z, c_x, c_y, c_z, up_x, up_y, up_z);

	glMatrixMode(GL_PROJECTION);						//2. Set the projection matrix
	glLoadIdentity();
	gluPerspective(fov, 1, z_near, z_far);
}

void keyboard(unsigned char c, int, int)					//Callback for keyboard events:
{
	switch (c)
	{
	case ' ':											// space:   Toggle through the various drawing styles of the mesh renderer
	{
		drawing_style = (++drawing_style) % 4;
		renderer.setDrawingStyle((MeshRenderer::DRAW_STYLE)drawing_style);
		break;
	}
	case 'N':
	case 'n':
	{
		OffReader off;
		delete grid;
		cout << "number in query result: " << modelToDraw + 1 << "| ";
		resultQuery[modelToDraw].printDistance();
		grid = off.ReadOffFile(resultQuery[modelToDraw].pathName.c_str());
		grid->computeFaceNormals();
		grid->computeVertexNormals();
		glutSetWindowTitle(("Position: " + to_string(modelToDraw + 1) + " | name: " + 
										resultQuery[modelToDraw].name + " | label: " + 
										resultQuery[modelToDraw].label + " | distance: " +	
										to_string(resultQuery[modelToDraw].dValue)).c_str());
		cout <<resultQuery.size() << endl;
		modelToDraw = (modelToDraw+1) % (resultQuery.size());
		break;
	}
	case 'R':											// 'r','R': Reset the viewpoint
	case 'r':

		setCamera();
		//glViewport(0, 0, 500, 500);
		//glMatrixMode(GL_MODELVIEW);
		//glLoadIdentity();
		//zprInit(0, 0, 0);
		break;
	}

	glutPostRedisplay();
}

void draw()												//Render the 3D mesh (GLUT callback function)
{
	renderer.draw(*grid);
}

void generateDatabaseOverview(string outputDest, string dbLocation)
{
	string path = dbLocation;
	namespace fs = std::experimental::filesystem;
	UnstructuredGrid3D* mesh;
	meshDescription::writeColumnString(outputDest);
	OffReader off;

	for (const auto& entry : fs::directory_iterator(path)) {
		string directoryName = entry.path().filename().string(); //name of current directory
		for (const auto& entry2 : fs::directory_iterator(entry.path())) {
			string directoryName2 = entry2.path().filename().string();
			string pathName = entry2.path().string();
			string fileName = pathName + "\\" + directoryName2 + ".off";

			mesh = off.ReadOffFile(fileName.c_str());

			meshDescription newMesh = meshDescription(mesh, directoryName, directoryName2);
			newMesh.writeDescriptionsToFile(outputDest);
			delete mesh;
		}
	}
}

void generateFeatureTable(string outputDest, string dbLocation)
{
	string path = dbLocation;
	namespace fs = std::experimental::filesystem;
	UnstructuredGrid3D* mesh;

	for (const auto& entry : fs::directory_iterator(path)) {
		string directoryName = entry.path().filename().string(); //name of current directory
		for (const auto& entry2 : fs::directory_iterator(entry.path())) {
			string directoryName2 = entry2.path().filename().string();
			string pathName = entry2.path().string();
			string fileName = pathName + "\\" + directoryName2 + ".off";

			OffReader off;
			mesh = off.ReadOffFile(fileName.c_str());

			gridFeatures feat = gridFeatures(mesh, 100000, 12, 12, 12, 12, 12);
			string featString = feat.featuresToString();

			ofstream myfile;
			myfile.open(outputDest, ios::out | ios::app);
			myfile << directoryName2 << "," << featString << endl;
			myfile.close();
			cout << fileName << endl;
			delete mesh;
		}
	}
}

UnstructuredGrid3D* decimateMesh(string fileName, int targetVertCount)
{
	pmp::SurfaceMesh* mesh = new pmp::SurfaceMesh();

	pmp::SurfaceMeshIO surfMesh(fileName, pmp::IOFlags());
	surfMesh.read(*mesh);

	cout << mesh->n_faces() << " " << mesh->n_vertices() << endl;
	if (mesh->n_vertices() > targetVertCount) {
		pmp::SurfaceSimplification* simplifier = new pmp::SurfaceSimplification(*mesh);
		simplifier->simplify(targetVertCount);
	}

	int i = 0;
	UnstructuredGrid3D* g = new UnstructuredGrid3D(mesh->n_vertices(), mesh->n_faces());
	//convert to unstructuredGrid3D
	for (auto v : mesh->vertices())
	{
		float l[3];
		pmp::Point p = mesh->position(v);
		l[0] = p[0];
		l[1] = p[1];
		l[2] = p[2];
		g->setPoint(i, l);
		i++;
	}

	i = 0;
	for (pmp::SurfaceMesh::FaceIterator fit = mesh->faces_begin();
		fit != mesh->faces_end(); ++fit)
	{
		int V[3];
		int j = 0;

		pmp::SurfaceMesh::VertexAroundFaceCirculator fvit = mesh->vertices(*fit),
			fvend = fvit;
		do
		{
			V[j] = (*fvit).idx();
			j++;
		} while (++fvit != fvend);
		g->setCell(i, V);
		i++;
	}

	g->normalize();
	cout << mesh->n_faces() << " " << mesh->n_vertices() << endl;

	return g;
}

void createDatabase(string path, int targetVertexCount)
{
	namespace fs = std::experimental::filesystem;
	UnstructuredGrid3D* mesh;
	ofstream errlog("errlog.txt");

	OffReader ofr;//6.  Read a 3D mesh stored in a file in the PLY format
	//grid = rdr.read(filename);


	for (const auto& entry : fs::directory_iterator(path))
	{
		string directoryName = entry.path().filename().string();
		for (const auto& entry2 : fs::directory_iterator(entry.path()))
		{
			string directoryName2 = entry2.path().filename().string();
			string pathName = entry2.path().string();
			string fileName = pathName + "\\" + directoryName2 + ".off";
			cout << fileName << endl;

			mesh = ofr.ReadOffFile(fileName.c_str());

			if (mesh->numPoints() > targetVertexCount + 1000)
			{
				mesh = decimateMesh(fileName.c_str(), targetVertexCount);
			}

			if (mesh->numPoints() < targetVertexCount - 1000)
			{
				Supersampler ss;
				try
				{
					ss.supersample(*mesh, targetVertexCount);
				}
				catch (exception e)
				{
					cout << "File " << directoryName2 << " cannot be supersampled!\nError: " << e.what() << "\n";
					errlog << "File " << directoryName2 << " cannot be supersampled!\nError: " << e.what() << "\n";
					continue;
				}
			}

			FourStepNorm normalizer;
			normalizer.centerOnBary(mesh); //Step 1. center on the barycenter (average x,y,z)
			normalizer.PCA(mesh); //Step 2. do PCA and use eigenvectors to translate all vertices
			normalizer.flipTest(mesh); //Step 3. make sure most most mass (number triangles is on the let side)
			normalizer.normalizeInCube(mesh); //Step 4. normalize the model

			string databaseFile = "ShapeDB/benchmark/db/" + directoryName + "/" + directoryName2 + "/" + directoryName2 + ".off";

			OFFConverter* converter = new OFFConverter();
			converter->WriteFileOFF(*mesh, databaseFile);

			fs::remove_all(pathName);

			delete mesh;
			delete converter;
		}
	}
}

void replaceBackslash(char* path)
{
	int i = 0;
	while (path[i] != NULL)
	{
		if (path[i] == '\\')
		{
			path[i] = '/';
		}
		i++;
	}
}

UnstructuredGrid3D* prepareMesh(string path, int targetVertexCount)
{
	UnstructuredGrid3D* mesh;
	//read mesh then get it to targetVertexCount
	//mesh = decimateMesh(path, targetVertexCount);
	OffReader ofr;
	mesh = ofr.ReadOffFile(path.c_str());
	if (mesh->numPoints() < targetVertexCount - 1000)
	{
		Supersampler ss;
		ss.supersample(*mesh, targetVertexCount);
	}
	else if (mesh->numPoints() > targetVertexCount + 1000)
	{
		mesh = decimateMesh(path, targetVertexCount);
	}
	//normalize
	FourStepNorm normalizer;
	normalizer.centerOnBary(mesh); //Step 1. center on the barycenter (average x,y,z)
	normalizer.PCA(mesh); //Step 2. do PCA and use eigenvectors to translate all vertices
	normalizer.flipTest(mesh); //Step 3. make sure most mass in left side (larger number triangles is on the left side)
	normalizer.normalizeInCube(mesh); //Step 4. normalize the model

	//Workaround for error when computing normals: write result of up/downsampling to file and read file again
	OFFConverter converter;
	converter.WriteFileOFF(*mesh, "input.off");
	//OffReader ofr;
	mesh = ofr.ReadOffFile("input.off");

	mesh->computeFaceNormals();
	mesh->computeVertexNormals();

	return mesh;
}

int main(int argc, char* argv[])							//Main program
{
	PlyReader rdr;										//6.  Read a 3D mesh stored in a file in the PLY format
	PlyWriter writer;
	Supersampler ss;
	OffReader ofr;

	const int targetVertexCount = 20000;
	const char* featureTableFilePath = "output/featureTableFinal.csv";
	const char* databaseFilePath = "shapeDB/benchmark/db/";
	
	char path[500];
	cout << "3D Model Database Query System" << endl;
	cout << "***" << endl;
	cout << "By Florian-Claudiu Gheorghica and Diego Renders" << endl;
	cout << "***" << endl;
	cout << "Under the guidance of Prof. Dr. Alexandru Telea" << endl;
	cout << "***" << endl;
	cout << "Utrecht University" << endl;
	cout << "\nPlease insert the path to the query file in OFF format (\'\\\' will be replaced by \'/\' automatically)." << endl;
	cout << "\nInput path: ";
	cin >> path;
	replaceBackslash(path);
	cout << "\nProcessing input & matching...\n";
	grid = prepareMesh(path, targetVertexCount);
	//surfArea, BBV, Eccentricity, Circularity, Diameter, AngleH, DistToBaryH, Dist2PtH, AreaTriH, VolTetraH 
	//float weights[10] = { 2.5f, 0.5f, 2.5f, 2.5f, 0.5f, 2.5f, 2.5f, 2.5f, 1, 1 };
	float weights[10] = { 1,1,1,1,1,1,1,1,1,1 };

	gridMatcher matcher = gridMatcher(featureTableFilePath, databaseFilePath, weights);

	resultQuery = matcher.matchSingle(grid, "", 10);

	cout << "3D unstructured grid (mesh) visualization." << endl;
	cout << "Interaction options: " << endl << endl;
	cout << "  Mouse: changes the viewpoint:" << endl;
	cout << "      -rotate:     left-click and drag" << endl;
	cout << "      -translate:  right-click (or Shift-left-click) and drag" << endl;
	cout << "      -scale:      middle-click (or Control-left-click) and drag" << endl;
	cout << "  Keyboard: visualization options:" << endl;
	cout << "      -r,R:        reset the viewpoint" << endl;
	cout << "      -space:      cycle through mesh rendering styles" << endl;
	cout << "      -n,N:        cycle through result meshes" << endl;

	glutInit(&argc, argv);								//1.  Initialize the GLUT toolkit
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);//2.  Ask GLUT to create next windows with a RGB framebuffer and a Z-buffer too
	glutInitWindowSize(500, 500);							//3.  Tell GLUT how large are the windows we want to create next
	glutCreateWindow("6. 3D mesh (unstructured grid)");	//4.  Create our window
	zprInit(0, 0, 0);										//5.  Initialize the viewpoint interaction-tool to look at the point (0,0,0)
	setCamera();

	glutMouseFunc(mouseclick);							//9.  Bind the mouse click and mouse drag (click-and-move) events to callbacks. This allows us
	glutMotionFunc(mousemotion);							//    next to control the viewpoint interactively.
	glutKeyboardFunc(keyboard);
	glutDisplayFunc(draw);								//10. Add a drawing callback to the window	
	glutReshapeFunc(viewing);							//11. Add a resize callback to the window
	glutMainLoop();										//12. Start the event loop that displays the graph and handles window-resize events

	return 0;
}




