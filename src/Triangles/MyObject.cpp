#define _USE_MATH_DEFINES
#include <cmath>
#include "MyObject.h"
#include <GL/glew.h>
#include <GL/freeglut.h>

#include <iostream>



MyObject::MyObject():
	pData(0),
	dataCount(0),
	radius(1),
	pIndexes(0),
	indexesCount(0)
{
	generateData();
}

MyObject::MyObject(int r, int x_,int y_, int z_) :
pData(0),
dataCount(0),
radius(r),
pIndexes(0),
indexesCount(0),
x(x_),
y(y_),
z(z_)
{
    dt = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    velocity = glm::vec3(0.1, 0, 0);
	generateData();
}

MyObject::~MyObject(void)
{
	glDeleteBuffers(2,vbo);
	glDeleteVertexArrays(1,&vao);
	if (pData)
	{
		delete[] pData;
		delete[] pIndexes;
	}
}

void MyObject::computePos(float r)                                                                                                      
{
    if (r == 0) return;

    double period = 10;
    double mean_motion = 2 * M_PI / period;
    double eccentricity = 0.5;
   // convert to seconds
   double fElapsedTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
   double semi_major_axis = 10;
   double semi_minor_axis = 5;

   // fmodf - compute remainder
   double fCurrTimeThroughLoop = fmod(fElapsedTime, period);
   double mean_anomaly = mean_motion * fCurrTimeThroughLoop;
   double E = mean_anomaly + eccentricity * sin(mean_anomaly) * (1.0 + eccentricity * cos(mean_anomaly));

   x = semi_major_axis * (cos(E) - eccentricity);
   z = semi_minor_axis * sin(E);
}

void MyObject::initGLBuffers(GLuint programId, const char* posName,const char* norName,const char* texName)
{
	glGenVertexArrays( 1, &vao );
	glBindVertexArray ( vao );

	glGenBuffers ( 2, &vbo[0]);
	
	glBindBuffer ( GL_ARRAY_BUFFER, vbo[0] );
	glBufferData ( GL_ARRAY_BUFFER, dataCount*sizeof(VertexData), pData, GL_STATIC_DRAW );
		
	glEnable(GL_ELEMENT_ARRAY_BUFFER);
	glBindBuffer ( GL_ELEMENT_ARRAY_BUFFER, vbo[1] );
	glBufferData ( GL_ELEMENT_ARRAY_BUFFER, indexesCount*sizeof(unsigned int), pIndexes, GL_STATIC_DRAW );
	
	int	loc = glGetAttribLocation(programId, posName);
	if (loc>-1)
	{
		glVertexAttribPointer(loc,3,GL_FLOAT,GL_FALSE,sizeof(VertexData),(GLuint *)0);
		glEnableVertexAttribArray (loc);
	}
	int loc2 = glGetAttribLocation(programId, norName);
	if (loc2>-1)
	{
		glVertexAttribPointer(loc2,3,GL_FLOAT,GL_FALSE,sizeof(VertexData),(GLuint *)(0+sizeof(float)*3));
		glEnableVertexAttribArray (loc2);
	}
	int loc3 = glGetAttribLocation(programId, texName);
	if (loc3>-1)
	{
		glVertexAttribPointer(loc3,2,GL_FLOAT,GL_FALSE,sizeof(VertexData),(GLuint *)(0+sizeof(float)*6));
		glEnableVertexAttribArray (loc3);
	}
	
	
}
void MyObject::Draw()
{
	glBindVertexArray(vao);
	glDrawElements(GL_QUADS,indexesCount,GL_UNSIGNED_INT,0);
	glBindVertexArray(0);
}

void MyObject::generateData()
{
	
	if (pData)
	{
		delete[] pData;
		delete[] pIndexes;
	}
	int rings = 20, sectors = 20;
	dataCount = rings * sectors;
	indexesCount = 4 * sectors * rings;
	pData = new VertexData[dataCount];
	pIndexes = new unsigned int[indexesCount];
	float const R = 1.0 / (float)(rings - 1);
	float const S = 1.0 / (float)(sectors - 1);
	int count = 0;
		for (int r = 0; r < rings; r++) for (int s = 0; s < sectors; s++) {
			float const y = sin(-M_PI_2 + M_PI * r * R);
			float const x = cos(2 * M_PI * s * S) * sin(M_PI * r * R);
			float const z = sin(2 * M_PI * s * S) * sin(M_PI * r * R);
			pData[count].pos = glm::vec3(x*radius,y*radius,z*radius);
			pData[count].nor = glm::vec3(x, y, z);
			pData[count].tex = glm::vec2(s*S,r*R);
			count++;
		}
		count = 0;
		for (int r = 0; r < rings; r++) for (int s = 0; s < sectors; s++) {
			pIndexes[count++] = r * sectors + s;
			pIndexes[count++] = r * sectors + (s + 1);
			pIndexes[count++] = (r + 1) * sectors + (s + 1);
			pIndexes[count++] = (r + 1) * sectors + s;
		}
	
}
