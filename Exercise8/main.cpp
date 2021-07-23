#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <GL/glew.h>
#include<GL/freeglut.h>
#include <iostream>
#include <stdio.h>
#include <cstdlib>

// Use Very Simple Libs
#include "VSMathlib.h"
#include "VSShaderlib.h"

using namespace std;

int g_mouseX, g_mouseY;
int rx1, ry1;
float lightDir[4] = { 0.0f, 800.0f, 800.0f, 0.0f };

int vertexNum;//顶点个数
int faceNum;//面个数
GLfloat* vertices;//各个顶点坐标数组，大小为3*vertexNum
GLuint* indices;//组成各个面的顶点的索引数组

VSMathLib* vsml;
VSShaderLib shader;
unsigned int VAO;


// 加载 PLY 文件；此处为了方便硬编码有关格式；格式参考：http://paulbourke.net/dataformats/ply/
bool load_ply() {
	char fname[] = "Dragon-ply.ply";
	FILE* f = fopen(fname, "rb");

	if (f == NULL) {
		cout << "file " << fname << " does not exist" << endl;
		return false;
	}

	int counter = 0; 
	while (counter < 11) {
		char c;
		fread((void*)(&c), sizeof(c), 1, f);
		if (c == '\n') {
			counter++;
		}
	}

	vertexNum = 2500086;
	faceNum = 5000000;

	vertices = new GLfloat[vertexNum * 3];
	indices = new GLuint[faceNum * 3];

	int j = 0;
	for (int i = 0; i < vertexNum; i++) {
		float x, y, z;
		fread((void*)(&x), 4, 1, f);
		fread((void*)(&y), 4, 1, f);
		fread((void*)(&z), 4, 1, f);

		if (j < vertexNum * 3) {
			vertices[j] = x;
			vertices[j + 1] = y;
			vertices[j + 2] = z;
			j += 3;
		}
	}
	int k = 0;
	for (int i = 0; i < faceNum; i++) {
		unsigned char n;
		int v1, v2, v3;

		fread((unsigned char*)(&n), 1, 1, f);
		fread((void*)(&v1), 4, 1, f);
		fread((void*)(&v2), 4, 1, f);
		fread((void*)(&v3), 4, 1, f);

		if (k < faceNum * 3) {
			indices[k] = v1;
			indices[k + 1] = v2;
			indices[k + 2] = v3;
			k += 3;
		}
	}

	fclose(f);
	return true;
}

void reshape(int w,int h) {
	float ratio = 0;
	if (h == 0) h = 1;
	glViewport(0, 0, w, h);

	ratio = (1.0f * w) / h;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(53.13f, ratio, 0.1f, 10000.0f);
	glMatrixMode(GL_MODELVIEW);
}

void display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	gluLookAt(0, 0, 600, 0, 0, 0, 0, 1, 0);
	glEnable(GL_DEPTH_TEST);

	glLightfv(GL_LIGHT0, GL_POSITION, lightDir);

	glPushMatrix();
	glTranslatef(0.0f, -1.0f, 0.0f);
	glRotatef(rx1, 0, 1, 0);
	glRotatef(ry1, 1, 0, 0);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 3 * sizeof(GLfloat), vertices);

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	

	glDrawElements(GL_TRIANGLES, faceNum * 3, GL_UNSIGNED_INT, indices);

	glPopMatrix();
	
	glutSwapBuffers();
}

void mouse(int button, int state, int x, int y){
	g_mouseX = x;
	g_mouseY = y;
}

void motion(int x, int y){
	rx1 -= x - g_mouseX;
	ry1 -= y - g_mouseY;
	g_mouseX = x;
	g_mouseY = y;
}

void idle()
{
	glutPostRedisplay();
}

void createLight()
{
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	GLfloat LightAmbient[] = { 0.7f, 0.7f, 0.7f, 1.0f }; //环境光参数
	GLfloat LightDiffuse[] = { 0.5f, 0.5f, 0.5f, 1.0f }; //漫射光参数
	GLfloat LightSpecular[] = { 0.3f, 0.3f, 0.3f, 1.0f }; //镜面光参数

	glLightfv(GL_LIGHT0, GL_AMBIENT, LightAmbient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, LightDiffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, LightSpecular);
}

int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(600, 600);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Exercise 8");

	if (!load_ply())
	{
		cerr << "Load ply file failed!" << endl;
		return -1;
	}

	cout << vertices[0] << " " << vertices[1] << " " << vertices[2] << " " << endl;
	cout << indices[0] << " " << indices[1] << " " << indices[2] << " " << endl;

	glClearColor(0.5, 0.5, 0.5, 1.0);
	createLight();
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutIdleFunc(idle);
	glutMainLoop();
	return 1;
}