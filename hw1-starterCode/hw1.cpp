/*
	CSCI 420 Computer Graphics, USC
	Assignment 1: Height Fields
	C++ starter code

	Student username: cammaran
*/

#include <iostream>
#include <cstring>
#include <string>
#include <sstream>
#include <vector>
#include "openGLHeader.h"
#include "glutHeader.h"

#include "imageIO.h"
#include "openGLMatrix.h"
#include "basicPipelineProgram.h"

#ifdef WIN32
	#ifdef _DEBUG
		#pragma comment(lib, "glew32d.lib")
	#else
		#pragma comment(lib, "glew32.lib")
	#endif
#endif

#ifdef WIN32
	char shaderBasePath[1024] = SHADER_BASE_PATH;
#else
	char shaderBasePath[96] = "../openGLHelper";
#endif

using namespace std;

/************************************************************/
// GLOBAL VARIABLES
/************************************************************/
// Input
int mousePos[2];									// x,y coordinate of the mouse position
int leftMouseButton = 0;							// 1 if pressed, 0 if not 
int middleMouseButton = 0;							// 1 if pressed, 0 if not
int rightMouseButton = 0;							// 1 if pressed, 0 if not

// Control State
typedef enum { ROTATE, TRANSLATE, SCALE } CONTROL_STATE;
CONTROL_STATE controlState = ROTATE;

// Application state variables
float landRotate[3] = { 0.0f, 0.0f, 0.0f };
float landTranslate[3] = { 0.0f, 0.0f, 0.0f };
float landScale[3] = { 1.0f, 1.0f, 1.0f };
int windowWidth = 1280;
int windowHeight = 720;
char windowTitle[512] = "CSCI 420 homework I";

// Image IO
ImageIO* heightmapImage = NULL;

// Pipeline Program
BasicPipelineProgram g_pipeline;
GLint g_programID;

// Display handling
enum DisplayTypeEnum { DEFAULT, WIREFRAME, POINTS, MIXED };
DisplayTypeEnum g_displayType = DEFAULT;

// Handle user controls -- default to a spinning showcase view
enum UserControlModeEnum { DISPLAY, MODIFY };
UserControlModeEnum g_controlMode = DISPLAY;

// ModelView and Projection Matrices
OpenGLMatrix g_matrices;
float mv[16];
float proj[16];
const float FOV = 45;

// Terrain data
std::vector<GLfloat> g_triangleVertices;
std::vector<GLfloat> g_triangleColors;
std::vector<GLfloat> g_wireframeVertices;
std::vector<GLfloat> g_wireframeColors;
std::vector<GLfloat> g_pointVertices;
std::vector<GLfloat> g_pointColors;
float g_terrainScale = 0.025f;

GLuint g_terrainVAO;
GLuint g_terrainVBO;
GLuint g_colorVBO;

GLuint g_terrainWireframeVAO;
GLuint g_terrainWireframeVBO;
GLuint g_colorWireframeVBO;

GLuint g_terrainPointsVAO;
GLuint g_terrainPointsVBO;
GLuint g_colorPointsVBO;

// Screenshot counter
const int MAXIMUM_SCREENSHOTS = 301;
int g_screenshotCounter = 0;
bool g_takeScreenshots = false;

// Misc Helpful Constants
const float TRANSLATION_MODIFIER = 1.0f;
const float ROTATION_ANGLE_THETA = 0.05f;
const float SCALE_MODIFIER = 0.01f;

/************************************************************/
// HELPER FUNCTIONS
/************************************************************/
void generateHeightfield (ImageIO* image) {

	// Calculate our height based on the image height
	g_terrainScale *= image->getHeight() / 100;

	// Generate our vertices--we should later use a triangle strip, that will make this so much more efficient. We center the heightmap at the origin.
	int width = image->getWidth();
	int height = image->getHeight();
	for (int i = -height / 2; i < height / 2 - 1; i++) {
		for (int j = -width / 2; j < width / 2 - 1; j++) {

			// Generate corners -- Bottom left
			GLfloat y = g_terrainScale * image->getPixel(i + height / 2, j + width / 2, 0);
			GLfloat bl[3] = { i, y, -j };

			// Generate corners -- Top Left
			y = g_terrainScale * image->getPixel(i + height / 2, j + width / 2 + 1, 0);
			GLfloat tl[3] = { i, y, -(j + 1) };

			// Generate corners -- Top Right
			y = g_terrainScale * image->getPixel(i + height / 2 + 1, j + width / 2 + 1, 0);
			GLfloat tr[3] = { i + 1, y, -(j + 1) };

			// Generate corners -- Bottom Right
			y = g_terrainScale * image->getPixel(i + height / 2 + 1, j + width / 2, 0);
			GLfloat br[3] = { i + 1, y, -j };

			// Push our coordinates into the vertex buffer as two triangles (clockwise)
			g_triangleVertices.insert (g_triangleVertices.end(), tl, tl + 3);
			g_triangleVertices.insert (g_triangleVertices.end(), tr, tr + 3);
			g_triangleVertices.insert (g_triangleVertices.end(), bl, bl + 3);
			g_triangleVertices.insert (g_triangleVertices.end(), bl, bl + 3);
			g_triangleVertices.insert (g_triangleVertices.end(), tr, tr + 3);
			g_triangleVertices.insert (g_triangleVertices.end(), br, br + 3);

			// Calculate colors
			GLfloat color = (float)image->getPixel(i + height / 2, j + width / 2, 0) / (float)255.0f;
			GLfloat colorBL[4] = { color, color, color, 1.0f };

			color = (float)image->getPixel(i + height / 2, j + width / 2 + 1, 0) / (float)255.0f;
			GLfloat colorTL[4] = { color, color, color, 1.0f };

			color = (float)image->getPixel(i + height / 2 + 1, j + width / 2 + 1, 0) / (float)255.0f;
			GLfloat colorTR[4] = { color, color, color, 1.0f };

			color = (float)image->getPixel(i + height / 2 + 1, j + width / 2, 0) / (float)255.0f;
			GLfloat colorBR[4] = { color, color, color, 1.0f };
			
			// Push our color data to the buffer
			g_triangleColors.insert (g_triangleColors.end(), colorTL, colorTL + 4);
			g_triangleColors.insert (g_triangleColors.end(), colorTR, colorTR + 4);
			g_triangleColors.insert (g_triangleColors.end(), colorBL, colorBL + 4);
			g_triangleColors.insert (g_triangleColors.end(), colorBL, colorBL + 4);
			g_triangleColors.insert (g_triangleColors.end(), colorTR, colorTR + 4);
			g_triangleColors.insert (g_triangleColors.end(), colorBR, colorBR + 4);

			// Now, do the same for our wireframe
			g_wireframeVertices.insert (g_wireframeVertices.end(), tl, tl + 3);
			g_wireframeVertices.insert (g_wireframeVertices.end(), tr, tr + 3);
			g_wireframeVertices.insert (g_wireframeVertices.end(), tr, tr + 3);
			g_wireframeVertices.insert (g_wireframeVertices.end(), bl, bl + 3);
			g_wireframeVertices.insert (g_wireframeVertices.end(), bl, bl + 3);
			g_wireframeVertices.insert (g_wireframeVertices.end(), tl, tl + 3);
			g_wireframeVertices.insert (g_wireframeVertices.end(), tr, tr + 3);
			g_wireframeVertices.insert (g_wireframeVertices.end(), br, br + 3);
			g_wireframeVertices.insert (g_wireframeVertices.end(), br, br + 3);
			g_wireframeVertices.insert (g_wireframeVertices.end(), bl, bl + 3);

			GLfloat colorWF[4] = { 1.0f, 1.0f, 0.0f, 1.0f };
			g_wireframeColors.insert (g_wireframeColors.end(), colorWF, colorWF + 4);
			g_wireframeColors.insert (g_wireframeColors.end(), colorWF, colorWF + 4);
			g_wireframeColors.insert (g_wireframeColors.end(), colorWF, colorWF + 4);
			g_wireframeColors.insert (g_wireframeColors.end(), colorWF, colorWF + 4);
			g_wireframeColors.insert (g_wireframeColors.end(), colorWF, colorWF + 4);
			g_wireframeColors.insert (g_wireframeColors.end(), colorWF, colorWF + 4);
			g_wireframeColors.insert (g_wireframeColors.end(), colorWF, colorWF + 4);
			g_wireframeColors.insert (g_wireframeColors.end(), colorWF, colorWF + 4);
			g_wireframeColors.insert (g_wireframeColors.end(), colorWF, colorWF + 4);
			g_wireframeColors.insert (g_wireframeColors.end(), colorWF, colorWF + 4);

			// Now, do the same for our point cloud
			g_pointVertices.insert (g_pointVertices.end(), tl, tl + 3);
			g_pointVertices.insert (g_pointVertices.end(), tr, tr + 3);
			g_pointVertices.insert (g_pointVertices.end(), bl, bl + 3);
			g_pointVertices.insert (g_pointVertices.end(), br, br + 3);

			GLfloat colorP[4] = { 1.0f, 0.0f, 1.0f, 1.0f };
			g_pointColors.insert (g_pointColors.end(), colorP, colorP + 4);
			g_pointColors.insert (g_pointColors.end(), colorP, colorP + 4);
			g_pointColors.insert (g_pointColors.end(), colorP, colorP + 4);
			g_pointColors.insert (g_pointColors.end(), colorP, colorP + 4);
		}
	}
}

// Generate all VBOs here.
void generateBuffers () {

	// Handle triangle VAO and VBOs
	glGenVertexArrays(1, &g_terrainVAO);
	glBindVertexArray(g_terrainVAO);

	glGenBuffers(1, &g_terrainVBO);
	glBindBuffer(GL_ARRAY_BUFFER, g_terrainVBO);
	glBufferData(GL_ARRAY_BUFFER, g_triangleVertices.size() * sizeof(GLfloat), &g_triangleVertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &g_colorVBO);
	glBindBuffer(GL_ARRAY_BUFFER, g_colorVBO);
	glBufferData(GL_ARRAY_BUFFER, g_triangleColors.size() * sizeof(GLfloat), &g_triangleColors[0], GL_STATIC_DRAW);

	// Handle wireframe VAO and VBOs
	glGenVertexArrays(1, &g_terrainWireframeVAO);
	glBindVertexArray(g_terrainWireframeVAO);

	glGenBuffers(1, &g_terrainWireframeVBO);
	glBindBuffer(GL_ARRAY_BUFFER, g_terrainWireframeVBO);
	glBufferData(GL_ARRAY_BUFFER, g_wireframeVertices.size() * sizeof(GLfloat), &g_wireframeVertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &g_colorWireframeVBO);
	glBindBuffer(GL_ARRAY_BUFFER, g_colorWireframeVBO);
	glBufferData(GL_ARRAY_BUFFER, g_wireframeColors.size() * sizeof(GLfloat), &g_wireframeColors[0], GL_STATIC_DRAW);

	// Handle point VAO and VBOs
	glGenVertexArrays(1, &g_terrainPointsVAO);
	glBindVertexArray(g_terrainPointsVAO);

	glGenBuffers(1, &g_terrainPointsVBO);
	glBindBuffer(GL_ARRAY_BUFFER, g_terrainPointsVBO);
	glBufferData(GL_ARRAY_BUFFER, g_pointVertices.size() * sizeof(GLfloat), &g_pointVertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &g_colorPointsVBO);
	glBindBuffer(GL_ARRAY_BUFFER, g_colorPointsVBO);
	glBufferData(GL_ARRAY_BUFFER, g_pointColors.size() * sizeof(GLfloat), &g_pointColors[0], GL_STATIC_DRAW);
}

/************************************************************/
// SCREENSHOTS
/************************************************************/
void saveScreenshot(const char* filename) {
	unsigned char* screenshotData = new unsigned char[windowWidth * windowHeight * 3];
	glReadPixels(0, 0, windowWidth, windowHeight, GL_RGB, GL_UNSIGNED_BYTE, screenshotData);

	ImageIO screenshotImg(windowWidth, windowHeight, 3, screenshotData);

	if (screenshotImg.save(filename, ImageIO::FORMAT_JPEG) == ImageIO::OK) {
		cout << "File " << filename << " saved successfully." << endl;
	}

	else {
		cout << "Failed to save file " << filename << '.' << endl;
	}

	delete [] screenshotData;
}

/************************************************************/
// GLOBAL CALLBACKS
/************************************************************/
// Render function -- draw in objects, then swap buffers
void displayFunc() {

	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	switch (g_displayType) {
		case DEFAULT:
			// Enable our vertex buffer data
			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, g_terrainVBO);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

			// Enable our color data
			glEnableVertexAttribArray(1);
			glBindBuffer(GL_ARRAY_BUFFER, g_colorVBO);
			glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

			glDrawArrays(GL_TRIANGLES, 0, g_triangleVertices.size() / 3);

			glDisableVertexAttribArray(0);
			glDisableVertexAttribArray(1);
			break;

		case WIREFRAME:
			// Enable our vertex buffer data
			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, g_terrainWireframeVBO);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

			// Enable our color data
			glEnableVertexAttribArray(1);
			glBindBuffer(GL_ARRAY_BUFFER, g_colorWireframeVBO);
			glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

			glDrawArrays(GL_LINES, 0, g_wireframeVertices.size() / 3);

			glDisableVertexAttribArray(0);
			glDisableVertexAttribArray(1);
			break;

		case POINTS:
			// Enable our vertex buffer data
			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, g_terrainPointsVBO);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

			// Enable our color data
			glEnableVertexAttribArray(1);
			glBindBuffer(GL_ARRAY_BUFFER, g_colorPointsVBO);
			glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

			glDrawArrays(GL_POINTS, 0, g_pointVertices.size() / 3);

			glDisableVertexAttribArray(0);
			glDisableVertexAttribArray(1);
			break;

		case MIXED:
			// Enable our vertex buffer data
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);   
			glEnable(GL_POLYGON_OFFSET_LINE);   // enable polygon offset
			glPolygonOffset(-1.0, -1.0);

			// Enable our vertex buffer data
			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, g_terrainWireframeVBO);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

			// Enable our color data
			glEnableVertexAttribArray(1);
			glBindBuffer(GL_ARRAY_BUFFER, g_colorWireframeVBO);
			glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

			glDrawArrays(GL_LINES, 0, g_wireframeVertices.size() / 3);

			glDisableVertexAttribArray(0);
			glDisableVertexAttribArray(1);

			glDisable(GL_POLYGON_OFFSET_LINE);   
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, g_terrainVBO);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

			// Enable our color data
			glEnableVertexAttribArray(1);
			glBindBuffer(GL_ARRAY_BUFFER, g_colorVBO);
			glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

			glDrawArrays(GL_TRIANGLES, 0, g_triangleVertices.size() / 3);

			glDisableVertexAttribArray(0);
			glDisableVertexAttribArray(1);
			break;
	}
	// Lastly, swap buffers
	glutSwapBuffers();
}

// Idle function -- used to take screenshots and rotate the object in display mode
void idleFunc() {
	// Animation requirement -- use stringstreams to build our filename
	if (g_screenshotCounter < MAXIMUM_SCREENSHOTS && g_takeScreenshots) {
		std::stringstream ss;
		ss << "../anim/" << g_screenshotCounter << ".jpg";
		std::string name = ss.str();
		saveScreenshot (name.c_str());
		g_screenshotCounter++;
	}

	// If we are in display mode, rotate the terrain
	if (g_controlMode == DISPLAY && heightmapImage != NULL) {
		g_matrices.SetMatrixMode (OpenGLMatrix::ModelView);
		g_matrices.Rotate(ROTATION_ANGLE_THETA / 8, 0.0f, 1.0f, 0.0f);
		g_matrices.GetMatrix (mv);

		// Send the matrices to the pipeline
		g_pipeline.SetModelViewMatrix (mv);
	}

	// make the screen update 
	glutPostRedisplay();
}

// Reshape function -- used to set up the perspective view matrix
void reshapeFunc(int w, int h) {
	glViewport(0, 0, w, h);

	// Set up our perspective matrix -- this will not change
	g_matrices.SetMatrixMode (OpenGLMatrix::Projection);
	g_matrices.LoadIdentity ();
	g_matrices.Perspective (FOV, (float)windowWidth / (float)windowHeight, 0.01, 1000.0);
	g_matrices.GetMatrix (proj);

	// Send matrix to the pipeline
	g_pipeline.SetProjectionMatrix (proj);
}

// Track mouse movement while one of the buttons is pressed.
void mouseMotionDragFunc(int x, int y) {

	// Only allow the following manipulations if the system is not on display mode.
	if (g_controlMode == MODIFY) {

		// The change in mouse position since the last invocation of this function.
		int mousePosDelta[2] = { x - mousePos[0], y - mousePos[1] };
		switch (controlState) {
			// translate the landscape
			case TRANSLATE:
				if (leftMouseButton) {
					// control x,y translation via the left mouse button
					landTranslate[0] += mousePosDelta[0] * 0.01f;
					landTranslate[1] -= mousePosDelta[1] * 0.01f;
				}

				if (middleMouseButton) {
					// control z translation via the middle mouse button
					landTranslate[2] += mousePosDelta[1] * 0.01f;
				}

				g_matrices.SetMatrixMode (OpenGLMatrix::ModelView);
				g_matrices.Translate(landTranslate[0] * TRANSLATION_MODIFIER, landTranslate[1] * TRANSLATION_MODIFIER, landTranslate[2] * TRANSLATION_MODIFIER);
				g_matrices.GetMatrix (mv);

				// Send the matrices to the pipeline
				g_pipeline.SetModelViewMatrix (mv);
				break;

			// rotate the landscape
			case ROTATE:
				if (leftMouseButton) {
					// control x,y rotation via the left mouse button
					landRotate[0] += mousePosDelta[1];
					landRotate[1] += mousePosDelta[0];
				}

				if (middleMouseButton) {
					// control z rotation via the middle mouse button
					landRotate[2] += mousePosDelta[1];
				}

				g_matrices.SetMatrixMode (OpenGLMatrix::ModelView);
				g_matrices.Rotate(ROTATION_ANGLE_THETA, landRotate[0], landRotate[1], landRotate[2]);
				g_matrices.GetMatrix (mv);

				// Send the matrices to the pipeline
				g_pipeline.SetModelViewMatrix (mv);
				break;

			// scale the landscape
			case SCALE:
				if (leftMouseButton) {
					// control x,y scaling via the left mouse button
					landScale[0] *= 1.0f + mousePosDelta[0] * 0.01f * SCALE_MODIFIER;
					landScale[2] *= 1.0f - mousePosDelta[1] * 0.01f * SCALE_MODIFIER;
				}

				if (middleMouseButton) {
					// control z scaling via the middle mouse button
					landScale[1] *= 1.0f - mousePosDelta[1] * 0.01f * SCALE_MODIFIER;
				}

				g_matrices.SetMatrixMode (OpenGLMatrix::ModelView);
				g_matrices.Scale(landScale[0], landScale[1], landScale[2]);
				g_matrices.GetMatrix (mv);

				// Send the matrices to the pipeline
				g_pipeline.SetModelViewMatrix (mv);

				break;
		}

		// store the new mouse position
		mousePos[0] = x;
		mousePos[1] = y;
	}
}

void mouseMotionFunc(int x, int y) {
	// mouse has moved
	// store the new mouse position
	mousePos[0] = x;
	mousePos[1] = y;
}

void mouseButtonFunc(int button, int state, int x, int y) {
	// a mouse button has has been pressed or depressed

	// keep track of the mouse button state, in leftMouseButton, middleMouseButton, rightMouseButton variables
	switch (button) {
		case GLUT_LEFT_BUTTON:
			leftMouseButton = (state == GLUT_DOWN);
		break;

		case GLUT_MIDDLE_BUTTON:
			middleMouseButton = (state == GLUT_DOWN);
		break;

		case GLUT_RIGHT_BUTTON:
			rightMouseButton = (state == GLUT_DOWN);
		break;
	}

	// keep track of whether CTRL and SHIFT keys are pressed
	switch (glutGetModifiers()) {
		case GLUT_ACTIVE_CTRL:
			controlState = TRANSLATE;
		break;

		case GLUT_ACTIVE_SHIFT:
			controlState = SCALE;
		break;

		// if CTRL and SHIFT are not pressed, we are in rotate mode
		default:
			controlState = ROTATE;
		break;
	}

	// store the new mouse position
	mousePos[0] = x;
	mousePos[1] = y;
}

void keyboardFunc(unsigned char key, int x, int y) {
	switch (key) {
		case 27: // ESC key
			exit(0); // exit the program
			break;

		case ' ':
			cout << "You pressed the spacebar." << endl;
			break;

		case 'x':
			// take a screenshot
			saveScreenshot("screenshot.jpg");
			break;

		// Handle changing drawing modes
		case '1':
			// Switch to default mesh
			g_displayType = DEFAULT;
			break;

		case '2':
			// Switch to wireframe mesh
			g_displayType = WIREFRAME;
			break;

		case '3':
			// Switch to point mesh
			g_displayType = POINTS;
			break;

		case '4':
			// Switch to overlay
			g_displayType = MIXED;
			break;

		// Changing from display to manipulation -- Tab key
		case 9:
			if (g_controlMode == DISPLAY) {
				g_controlMode = MODIFY;
				std::cout << "Modify mode." << std::endl;
			}

			else {
				g_controlMode = DISPLAY;
				std::cout << "Display mode." << std::endl;
			}
			break;

		case 'q':
			g_takeScreenshots = true;
			std::cout << "Starting animation!." << std::endl;
			break;
	}
}

// Initialize the scene and all associated values.
void initScene(int argc, char *argv[]) {

	// Load the image from a jpeg disk file to main memory
	heightmapImage = new ImageIO();
	if (heightmapImage->loadJPEG(argv[1]) != ImageIO::OK) {
		cout << "Error reading image " << argv[1] << "." << endl;
		exit(EXIT_FAILURE);
	}

	// Set the color to clear -- (black with no alpha)
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	// Load, compile, and initialize the shaders
	if (g_pipeline.Init("../openGLHelper-starterCode") != 0) {
		exit(EXIT_FAILURE);
	}
	 // Bind the shaders
	g_pipeline.Bind();

	g_programID = g_pipeline.GetProgramHandle();
	glUseProgram(g_programID);

	// Set up our matrices -- start with the ModelView matrix default value
	g_matrices.SetMatrixMode (OpenGLMatrix::ModelView);
	g_matrices.LoadIdentity ();									// Set up the model matrix							
	g_matrices.LookAt (
		128, 64, 128,
		0, 0, 0,
		0, 1, 0
	);															// Set up the view matrix
	g_matrices.GetMatrix (mv);

	// Send the matrices to the pipeline
	g_pipeline.SetModelViewMatrix (mv);

	// Now, load our heightmap
	generateHeightfield (heightmapImage);
	generateBuffers ();

	// Enable depth testing, then prioritize fragments closest to the camera
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
}

/************************************************************/
// ENTRY POINT FUNCTION
/************************************************************/
int main(int argc, char *argv[]) {
	if (argc != 2) {
		cout << "The arguments are incorrect." << endl;
		cout << "usage: ./hw1 <heightmap file>" << endl;
		exit(EXIT_FAILURE);
	}

	cout << "Initializing GLUT..." << endl;
	glutInit(&argc,argv);

	cout << "Initializing OpenGL..." << endl;

	#ifdef __APPLE__
		glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
	#else
		glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
	#endif

	glutInitWindowSize(windowWidth, windowHeight);
	glutInitWindowPosition(0, 0);	
	glutCreateWindow(windowTitle);

	cout << "OpenGL Version: " << glGetString(GL_VERSION) << endl;
	cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << endl;
	cout << "Shading Language Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

	// tells glut to use a particular display function to redraw 
	glutDisplayFunc(displayFunc);
	// perform animation inside idleFunc
	glutIdleFunc(idleFunc);
	// callback for mouse drags
	glutMotionFunc(mouseMotionDragFunc);
	// callback for idle mouse movement
	glutPassiveMotionFunc(mouseMotionFunc);
	// callback for mouse button changes
	glutMouseFunc(mouseButtonFunc);
	// callback for resizing the window
	glutReshapeFunc(reshapeFunc);
	// callback for pressing the keys on the keyboard
	glutKeyboardFunc(keyboardFunc);

	// init glew
	#ifdef __APPLE__
		// nothing is needed on Apple
	#else
		// Windows, Linux
		GLint result = glewInit();
		if (result != GLEW_OK) {
			cout << "error: " << glewGetErrorString(result) << endl;
			exit(EXIT_FAILURE);
		}
	#endif

	// do initialization
	initScene(argc, argv);

	// sink forever into the glut loop
	glutMainLoop();
}


