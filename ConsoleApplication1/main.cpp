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
	float eye_x = 1.2, eye_y = 1.2, eye_z = -1.0f;				//Modelview (camera extrinsic) parameters
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

	for (const auto& entry : fs::directory_iterator(path)) {
		string directoryName = entry.path().filename().string(); //name of current directory
		for (const auto& entry2 : fs::directory_iterator(entry.path())) {
			string directoryName2 = entry2.path().filename().string();
			string pathName = entry2.path().string();
			string fileName = pathName + "\\" + directoryName2 + ".off";

			OFFConverter* converter = new OFFConverter();
			converter->ConvertOFFToPLY(fileName);

			PlyReader rdr;										
			mesh = rdr.read((pathName + "\\" + directoryName2 + ".ply").c_str());

			mesh->normalize();

			meshDescription newMesh = meshDescription(mesh, directoryName, directoryName2);
			newMesh.writeDescriptionsToFile(outputDest);
			delete mesh;
			delete converter;
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
	g->computeFaceNormals();							//8.  Compute face and vertex normals for the mesh. This allows us to shade the mesh next.
	g->computeVertexNormals();
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

			if (mesh->numPoints() < targetVertexCount)
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

			string databaseFile = "ShapeDB/benchmark/db/" +directoryName + "/" + directoryName2 + "/" + directoryName2 + ".off";
			
			OFFConverter* converter = new OFFConverter();
			converter->WriteFileOFF(*mesh, databaseFile);

			fs::remove_all(pathName);

			delete mesh;
			delete converter;
		}
	}
}

int main(int argc, char* argv[])							//Main program
{
	cout << "3D unstructured grid (mesh) visualization." << endl;
	cout << "Interaction options: " << endl << endl;
	cout << "  Mouse: changes the viewpoint:" << endl;
	cout << "      -rotate:     left-click and drag" << endl;
	cout << "      -translate:  right-click (or Shift-left-click) and drag" << endl;
	cout << "      -scale:      middle-click (or Control-left-click) and drag" << endl;
	cout << "  Keyboard: visualization options:" << endl;
	cout << "      -r,R:        reset the viewpoint" << endl;
	cout << "      -space:      cycle through mesh rendering styles" << endl;
	/*
	const char* filename = (argc < 2) ? "DATA/m94.ply" : argv[1];  //Read the PLY file given as 1st argument. If no arguments given, use a default file.

	//OFFConverter* converter = new OFFConverter();
	//converter->ConvertOFFToPLY(filename);
	
	glutInit(&argc, argv);								//1.  Initialize the GLUT toolkit
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	//2.  Ask GLUT to create next windows with a RGB framebuffer and a Z-buffer too
	glutInitWindowSize(500, 500);							//3.  Tell GLUT how large are the windows we want to create next
	glutCreateWindow("6. 3D mesh (unstructured grid)");	//4.  Create our window
	zprInit(0, 0, 0);										//5.  Initialize the viewpoint interaction-tool to look at the point (0,0,0)
	setCamera();

	//createDatabase("InputShapes/benchmark/db", 20000);

	PlyReader rdr;										//6.  Read a 3D mesh stored in a file in the PLY format
	PlyWriter writer;
	Supersampler ss;

	//OFFConverter* converter = new OFFConverter();
	//converter->ConvertOFFToPLY(filename);
	//return 1;
	//const char* newfile = "DATA/m43.ply";
	*/
	char classFilePath[50] = "../classification/v1/coarse1/coarse1.cla";
	char outputFile[50] = "output/classes.txt";
	
	ofstream classFile(outputFile);

	PSBCategoryList* categories = parseFile(classFilePath);

	for (int i = 0; i < categories->_numCategories; i++)
	{
		cout << "Category " << i << ": " << categories->_categories[i]->_fullName << " and name " << categories->_categories[i]->_name << "\n";
		if (categories->_categories[i]->_numModels > 0)
		{
			cout << categories->_categories[i]->_models[0] << "\n";
			for (int j = 0; j < categories->_categories[i]->_numModels; j++)
			{
				//possible crash trying to access models +1
				classFile << categories->_categories[i]->_models[j] << "," << categories->_categories[i]->_name << "\n";
			}
		}
	}
	classFile.close();
	/*
	grid = rdr.read(filename);
	cout << grid->getVolume() << "\n";
	//ss.addTriangle(*grid, 0);

	//ss.supersample(*grid, 20000);
	//ss.supersample(*grid, 10000);

	//writer.WritePlyFile(filename, grid);
	//writer.WritePlyFile(filename, grid);

	//generates a summary of all the meshes to the outputfile. dbLocation is benchmark/db
	//generateDatabaseOverview("output/description.txt", "C:/Users/Diego/Documents/School/MultimediaRetrieval/Datasets/psb_v1/benchmark/db");
	
	//Perform 4 step normalization on the model
	FourStepNorm normalizer;
	normalizer.centerOnBary(grid); //Step 1. center on the barycenter (average x,y,z)
	normalizer.PCA(grid); //Step 2. do PCA and use eigenvectors to translate all vertices
	normalizer.flipTest(grid); //Step 3. make sure most most mass (number triangles is on the let side)
	normalizer.normalizeInCube(grid); //Step 4. normalize the model
	
	grid->computeVertexNormals();
	grid->computeFaceNormals();

	cout << grid->getVolume() << "\n";
	cout << grid->getDistance(0, 1) << " " << grid->getDistance(1, 2) << " " << grid->getDistance(0, 2) << "\n";
	cout << grid->computeCircularity() << "\n";

	glutMouseFunc(mouseclick);							//9.  Bind the mouse click and mouse drag (click-and-move) events to callbacks. This allows us
	glutMotionFunc(mousemotion);							//    next to control the viewpoint interactively.
	glutKeyboardFunc(keyboard);
	glutDisplayFunc(draw);								//10. Add a drawing callback to the window	
	glutReshapeFunc(viewing);							//11. Add a resize callback to the window
	glutMainLoop();										//12. Start the event loop that displays the graph and handles window-resize events
	*/


	return 0;
}




