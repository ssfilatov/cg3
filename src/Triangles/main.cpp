//standard libraries
#define _USE_MATH_DEFINES
#include <iostream>
#include <cmath>
using namespace std;
//opengl headers
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <SOIL/SOIL.h>
//opengl mathematics
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>

//camera
#include "camera.h"
//functions for shader compilation and linking
#include "shaderhelper.h"
//object for drawing
#include "MyObject.h"

Camera camera;

float yRotated = 0;

//model for drawing: a obj from two triangles
MyObject* obj;

//struct for loading shaders
ShaderProgram shaderProgram;

//window size
int windowWidth = 800;
int windowHeight = 600;

//last mouse coordinates
int mouseX, mouseY;

//matrices
glm::mat4x4 modelViewMatrix;
glm::mat4x4 projectionMatrix;
glm::mat4x4 modelViewProjectionMatrix;
glm::mat4x4 normalMatrix;

///defines drawing mode
bool useTexture = true;
float cur_x = 1, cur_z = -1;
//texture identificator

GLuint texId[2];
enum
{
	EARTH,
	SOL,
	STARS,
};

//names of shader files. program will search for them during execution
//don't forget place it near executable 
char VertexShaderName[] = "Vertex.vert";
char FragmentShaderName[] = "Fragment.frag";

////////////////////////////////////////////////////////
///
void initTexture(int planetName, string fileName)
{
	glBindTexture(GL_TEXTURE_2D, texId[planetName]);

	//don't use alignment
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// Set nearest filtering mode for texture minification
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	int w, h;
	unsigned char* texture = SOIL_load_image(fileName.c_str(), &w, &h, 0, SOIL_LOAD_RGB);
	//set Texture Data
	if (texture == NULL) {
		std::cout << "Cannot open texture file";
	}
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB,
		GL_UNSIGNED_BYTE, texture);

}
void initTexture()
{
	//enable texturing and zero slot
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_TEXTURE0);
	//generate as many textures as you need
	glGenTextures(2, texId);
	//make texId current 
	initTexture(EARTH, "../../earth.jpg");
	//initTexture(STARS, "stars.jpg");
	initTexture(SOL, "../../realsun.jpg");
}

/////////////////////////////////////////////////////////////////////
///is called when program starts
void init()
{
	//enable depth test
	glEnable(GL_DEPTH_TEST);
	//initialize shader program
	shaderProgram.init(VertexShaderName, FragmentShaderName);
	//use this shader program
	glUseProgram(shaderProgram.programObject);

	//create new object
	//obj = new MyObject[2];
	obj = (MyObject*) operator new (sizeof (MyObject[2])); //allocate memory for array of objects
	new (&obj[EARTH]) MyObject(1,0,0,5);
	new (&obj[SOL]) MyObject(2,0,0,0);

	//initialize opengl buffers and variables inside of object
	obj[EARTH].initGLBuffers(shaderProgram.programObject, "pos", "nor", "tex");
	obj[SOL].initGLBuffers(shaderProgram.programObject, "pos", "nor", "tex");

	//initializa texture
	initTexture();
}


/////////////////////////////////////////////////////////////////////
///called when window size is changed
void reshape(int width, int height)
{
	windowWidth = width;
	windowHeight = height;
	//set viewport to match window size
	glViewport(0, 0, width, height);

	float fieldOfView = 45.0f;
	float aspectRatio = float(width) / float(height);
	float zNear = 0.1f;
	float zFar = 100.0f;
	//set projection matrix
	projectionMatrix = glm::perspective(fieldOfView, aspectRatio, zNear, zFar);
}

////////////////////////////////////////////////////////////////////
///actions for single frame
void initObject(int planetName, glm::mat4x4 &viewMatrix, int r, glm::vec3 rotationVec)
{
	glm::mat4x4 modelMatrix = glm::translate(glm::mat4(), glm::vec3(0, 0, 0));
	//modelMatrix = glm::rotate(modelMatrix, yRotated, glm::vec3(0, 1, 0));

	obj[planetName].computePos(r);
	glm::vec3 off(obj[planetName].x, obj[planetName].y, obj[planetName].z);
	modelMatrix = glm::translate(glm::mat4(), off);

	if (rotationVec != glm::vec3(0,0,0)) {
		modelMatrix = glm::rotate(modelMatrix, yRotated, rotationVec);
	}
	//modelViewMatrix consists of viewMatrix and modelMatrix
	modelViewMatrix = viewMatrix*modelMatrix;
	//calculate normal matrix 
	normalMatrix = glm::inverseTranspose(modelViewMatrix);
	//finally calculate modelViewProjectionMatrix
	modelViewProjectionMatrix = projectionMatrix*modelViewMatrix;

	//bind texture
	glBindTexture(GL_TEXTURE_2D, texId[planetName]);
	int locMV = glGetUniformLocation(shaderProgram.programObject, "modelViewMatrix");
	if (locMV>-1)
		glUniformMatrix4fv(locMV, 1, 0, glm::value_ptr(modelViewMatrix));
	int locN = glGetUniformLocation(shaderProgram.programObject, "normalMatrix");
	if (locN>-1)
		glUniformMatrix4fv(locN, 1, 0, glm::value_ptr(normalMatrix));
	int locP = glGetUniformLocation(shaderProgram.programObject, "modelViewProjectionMatrix");
	if (locP>-1)
		glUniformMatrix4fv(locP, 1, 0, glm::value_ptr(modelViewProjectionMatrix));
	int texLoc = glGetUniformLocation(shaderProgram.programObject, "texture");
	if (texLoc>-1)
		glUniform1ui(texLoc, 0);
	int locFlag = glGetUniformLocation(shaderProgram.programObject, "useTexture");
	if (locFlag>-1)
		glUniform1i(locFlag, useTexture);
	obj[planetName].Draw();
}


void display()
{
	camera.update();
    glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glUseProgram(shaderProgram.programObject);
	
	glm::mat4x4 viewMatrix = camera.getViewMatrix();

	initObject(EARTH, viewMatrix, 5, glm::vec3(0, 1, 0));
	initObject(SOL, viewMatrix, 0,  glm::vec3(0, 0, 0));
	//initObject(STARS, viewMatrix, 0, 0, glm::vec3(0, 0, 0));

	glutSwapBuffers();
}

//////////////////////////////////////////////////////////////////////////
///IdleFunction
void update()
{
	yRotated += 0.2f;
	glutPostRedisplay();
}


/////////////////////////////////////////////////////////////////////////
///is called when key on keyboard is pressed
///use SPACE to switch mode
///TODO: place camera transitions in this function
void keyboard(unsigned char key, int mx, int my)
{
	switch (key) {
	case GLUT_KEY_UP: camera.setDeltaMove(UP_ACTION); break;
	case GLUT_KEY_DOWN: camera.setDeltaMove(DOWN_ACTION); break;
	case ' ': useTexture = !useTexture;
	}
}

void pressKey(int key, int xx, int yy) {

	switch (key) {
	case GLUT_KEY_UP: camera.setDeltaMove(UP_ACTION); break;
	case GLUT_KEY_DOWN: camera.setDeltaMove(DOWN_ACTION); break;
	}
}

void releaseKey(int key, int x, int y) {

	switch (key) {
	case GLUT_KEY_UP:
	case GLUT_KEY_DOWN: camera.setDeltaMove(STOP_ACTION); break;
	}
}


void mouseMove(int x, int y) {
	camera.moveMouseUpdate(x, y);
}

void mouseButton(int button, int state, int x, int y) {
	// only start motion if the left button is pressed
	if (button == GLUT_LEFT_BUTTON) {

		// when the button is released
		if (state == GLUT_UP) {
			camera.releaseMouseButton();
		}
		else  {// state = GLUT_DOWN
			camera.setOrigins(&x, &y);
		}
	}
}


/////////////////////////////////////////////////////////////////////////
///is called when mouse button is pressed
///TODO: place camera rotations in this function
void mouse(int button, int mode, int posx, int posy)
{
	if (button == GLUT_LEFT_BUTTON)
	{
		if (mode == GLUT_DOWN)
		{
			mouseX = posx; mouseY = posy;
		}
		else
		{
			mouseX = -1; mouseY = -1;
		}
	}

}

////////////////////////////////////////////////////////////////////////
///this function is used in case of InitializationError
void emptydisplay()
{
}

//////////////////////////////////////////////////////////////////////// 
///entry point
int main(int argc, char* argv[])
{
	glutInit(&argc, argv);
#ifdef __APPLE__
	glutInitDisplayMode(GLUT_2_1_CORE_PROFILE | GLUT_RGBA | GLUT_DOUBLE);
#else
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitContextVersion(2, 1);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glewExperimental = GL_TRUE;
#endif
	glutCreateWindow("Test OpenGL application");
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutReshapeWindow(windowWidth, windowHeight);
	glutIdleFunc(update);
	glutMouseFunc(mouse);
	glutIgnoreKeyRepeat(1);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(pressKey);
	glutSpecialUpFunc(releaseKey);

	// here are the two new functions
	glutMouseFunc(mouseButton);
	glutMotionFunc(mouseMove);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glewInit();
	const char * slVer = (const char *)glGetString(GL_SHADING_LANGUAGE_VERSION);
	cout << "GLSL Version: " << slVer << endl;

	try
	{
		init();
	}
	catch (const char *str)
	{
		cout << "Error During Initialiation: " << str << endl;
		delete obj;
		//glDeleteTextures(2, texId);
		//start main loop with empty screen
		glutDisplayFunc(emptydisplay);
		glutMainLoop();
		return -1;
	}


	try
	{
		glutMainLoop();
	}
	catch (const char *str)
	{
		cout << "Error During Main Loop: " << str << endl;
	}
	//release memory
	delete obj;
	//glDeleteTextures(1, &texId);
	return 0;
}
