#include "include/OffReader.h"
#include "include/UnstructuredGrid3D.h"
#include <ctype.h>
#include <string>
#include <iostream>

typedef struct Face
{
	int nverts;    /* number of vertex indices in list */
	int* verts;              /* vertex index list */
	~Face() {
		delete[] verts;
	};
} Face;

UnstructuredGrid3D* OffReader::ReadOffFile(const char* filename)
{
	int i;

	// Open file
	FILE* fp;
	if (!(fp = fopen(filename, "r"))) {
		fprintf(stderr, "Unable to open file %s\n", filename);
		return 0;
	}

	// Read file
	UnstructuredGrid3D* mesh = NULL;
	int nverts = 0; int nProcessedVerts = 0;
	int nfaces = 0; int nProcessedFaces = 0;
	int nedges = 0;
	int line_count = 0;
	char buffer[1024];
	while (fgets(buffer, 1023, fp)) {
		// Increment line counter
		line_count++;

		// Skip white space
		char* bufferp = buffer;
		while (isspace(*bufferp)) bufferp++;

		// Skip blank lines and comments
		if (*bufferp == '#') continue;
		if (*bufferp == '\0') continue;

		// Check section
		if (nverts == 0) {
			// Read header 
			if (!strstr(bufferp, "OFF")) {
				// Read mesh counts
				if ((sscanf(bufferp, "%d%d%d", &nverts, &nfaces, &nedges) != 3) || (nverts == 0)) {
					fprintf(stderr, "Syntax error reading header on line %d in file %s\n", line_count, filename);
					fclose(fp);
					return NULL;
				}

				// Allocate mesh structure
				mesh = new UnstructuredGrid3D(nverts,nfaces);
				if (!mesh) {
					fprintf(stderr, "Unable to allocate memory for file %s\n", filename);
					fclose(fp);
					return 0;
				}

			}
		}
		else if (nProcessedVerts < nverts) {
			// Read vertex coordinates
			float V[3];
			if (sscanf(bufferp, "%f%f%f", &(V[0]), &(V[1]), &(V[2])) != 3) {
				fprintf(stderr, "Syntax error with vertex coordinates on line %d in file %s\n", line_count, filename);
				fclose(fp);
				return NULL;
			}
			mesh->setPoint(nProcessedVerts, V);
			nProcessedVerts++;
		}
		else if (nProcessedFaces < nfaces) {
			// Get next face
			//Face& face = mesh->faces[mesh->nfaces++];
			Face* face = new Face();
			// Read number of vertices in face 
			bufferp = strtok(bufferp, " \t");
			if (bufferp) face->nverts = atoi(bufferp);
			else {
				fprintf(stderr, "Syntax error with face on line %d in file %s\n", line_count, filename);
				fclose(fp);
				return NULL;
			}

			// Allocate memory for face vertices
			
			face->verts = new int [face->nverts];
			//assert(face.verts);

			// Read vertex indices for face
			for (i = 0; i < face->nverts; i++) {
				bufferp = strtok(NULL, " \t");
				if (bufferp) face->verts[i] = atoi(bufferp);
				else {
					fprintf(stderr, "Syntax error with face on line %d in file %s\n", line_count, filename);
					fclose(fp);
					return NULL;
				}
			}
			mesh->setCell(nProcessedFaces, face->verts);
			nProcessedFaces++;
			delete face;
		}
		else {
			// Should never get here
			fprintf(stderr, "Found extra text starting at line %d in file %s\n", line_count, filename);
			break;
		}
	}

	// Check whether read all faces
	if (nfaces != mesh->numCells()) {
		fprintf(stderr, "Expected %d faces, but read only %d faces in file %s\n", nfaces, mesh->numCells(), filename);
	}

	// Close file
	fclose(fp);
	// Return mesh 
	return mesh;
}