#include <GL/freeglut.h>									//GLUT library
#include <experimental/filesystem>							//for looping through the database directories 
#include <iostream>
#include "include/UnstructuredGrid3D.h"							//the 3D unstructured grid class
#include "include/MeshRenderer.h"								//a simple renderer for 3D unstructured grids, demonstrating flat/smooth shading
#include "include/PlyReader.h"									//a reader that initializes UnstructuredGrid3D objects with meshes stored in the PLY file format
#include "include/zpr.h"										//library for interactively manipulating the OpenGL camera (viewpoint) using the mouse
#include "include/meshDescription.h"
#include "OFFConvertor.h"
#include "PlyWriter.h"
#include "include/FourStepNorm.h"


float fov = 120;										//Perspective projection parameters
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
	float eye_x = -1, eye_y = -1, eye_z = -1;				//Modelview (camera extrinsic) parameters
	float c_x = 0, c_y = 0, c_z = 0;
	float up_x = 0, up_y = 0, up_z = 1;
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

	const char* filename = (argc < 2) ? "DATA/bunny.ply" : argv[1];  //Read the PLY file given as 1st argument. If no arguments given, use a default file.

	//OFFConverter* converter = new OFFConverter();
	//converter->ConvertOFFToPLY("DATA/m2.off");

	glutInit(&argc, argv);								//1.  Initialize the GLUT toolkit
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	//2.  Ask GLUT to create next windows with a RGB framebuffer and a Z-buffer too
	glutInitWindowSize(500, 500);							//3.  Tell GLUT how large are the windows we want to create next
	glutCreateWindow("6. 3D mesh (unstructured grid)");	//4.  Create our window
	zprInit(0, 0, 0);										//5.  Initialize the viewpoint interaction-tool to look at the point (0,0,0)
	setCamera();

	PlyReader rdr;										//6.  Read a 3D mesh stored in a file in the PLY format
	PlyWriter writer;
	grid = rdr.read(filename);

	//writer.WritePlyFile(filename, grid);

	//generates a summary of all the meshes to the outputfile. dbLocation is benchmark/db
	//generateDatabaseOverview("output/description.txt", "C:/Users/Diego/Documents/School/MultimediaRetrieval/Datasets/psb_v1/benchmark/db");

	//Perform 4 step normalization on the model
	FourStepNorm normalizer;
	normalizer.centerOnBary(grid); //Step 1. center on the barycenter (average x,y,z)
	normalizer.PCA(grid); //Step 2. do PCA and use eigenvectors to translate all vertices
	normalizer.flipTest(grid); //Step 3. make sure most most mass (number triangles is on the let side)
	normalizer.normalizeInCube(grid); //Step 4. normalize the model

	glutMouseFunc(mouseclick);							//9.  Bind the mouse click and mouse drag (click-and-move) events to callbacks. This allows us
	glutMotionFunc(mousemotion);							//    next to control the viewpoint interactively.
	glutKeyboardFunc(keyboard);
	glutDisplayFunc(draw);								//10. Add a drawing callback to the window	
	glutReshapeFunc(viewing);							//11. Add a resize callback to the window
	glutMainLoop();										//12. Start the event loop that displays the graph and handles window-resize events

	return 0;
}




