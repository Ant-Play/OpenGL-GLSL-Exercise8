#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <GL/glew.h>
#include<GL/freeglut.h>
#include <iostream>
#include <stdio.h>
#include <cstdlib>

#pragma comment(lib,"glew32.lib")

using namespace std;

int g_mouseX, g_mouseY;
int rx1, ry1;

int vertexNum;//顶点个数
int faceNum;//面个数
GLfloat* vertices;//各个顶点坐标数组，大小为3*vertexNum
GLuint* indices;//组成各个面的顶点的索引数组

GLfloat l_dir[3] = { 1.0f, 1.0f, 1.0f };
GLfloat l_ambient[4] = { 0.2f , 0.15f , 0.1f , 1.0f };
GLfloat l_diffuse[4] = { 0.8f,0.6f,0.4f,1.0f };
GLfloat l_specular[4] = { 1.0f,1.0f,1.0f,1.0f };
GLfloat l_shininess = 30;
GLuint programHandle;
GLuint vShader, fShader;

// Camera Position
float camX = 0, camY = 700, camZ = 400;

unsigned int VBO;
unsigned int VAO;
unsigned int EBO;


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

//读入字符流  
char* textFileRead(const char* fn)
{
	FILE* fp;
	char* content = NULL;
	int count = 0;
	if (fn != NULL)
	{
		fp = fopen(fn, "rt");
		if (fp != NULL)
		{
			fseek(fp, 0, SEEK_END);
			count = ftell(fp);
			rewind(fp);
			if (count > 0)
			{
				content = (char*)malloc(sizeof(char) * (count + 1));
				count = fread(content, sizeof(char), count, fp);
				content[count] = '\0';
			}
			fclose(fp);
		}
	}
	return content;
}

void initShader(const char* VShaderFile, const char* FShaderFile)
{
	//1、查看显卡、GLSL和OpenGL的信息    
	const GLubyte* vendor = glGetString(GL_VENDOR);
	const GLubyte* renderer = glGetString(GL_RENDERER);
	const GLubyte* version = glGetString(GL_VERSION);
	const GLubyte* glslVersion = glGetString(GL_SHADING_LANGUAGE_VERSION);
	cout << "显卡供应商   : " << vendor << endl;
	cout << "显卡型号     : " << renderer << endl;
	cout << "OpenGL版本   : " << version << endl;
	cout << "GLSL版本     : " << glslVersion << endl;
	//2、编译着色器    
	//创建着色器对象：顶点着色器    
	vShader = glCreateShader(GL_VERTEX_SHADER);
	//错误检测    
	if (0 == vShader)
	{
		cerr << "ERROR : Create vertex shader failed" << endl;
		exit(1);
	}
	//把着色器源代码和着色器对象相关联    
	const GLchar* vShaderCode = textFileRead(VShaderFile);
	const GLchar* vCodeArray[1] = { vShaderCode };

	//将字符数组绑定到对应的着色器对象上  
	glShaderSource(vShader, 1, vCodeArray, NULL);

	//编译着色器对象    
	glCompileShader(vShader);

	//检查编译是否成功    
	GLint compileResult;
	glGetShaderiv(vShader, GL_COMPILE_STATUS, &compileResult);
	if (GL_FALSE == compileResult)
	{
		GLint logLen;
		//得到编译日志长度    
		glGetShaderiv(vShader, GL_INFO_LOG_LENGTH, &logLen);
		if (logLen > 0)
		{
			char* log = (char*)malloc(logLen);
			GLsizei written;
			//得到日志信息并输出    
			glGetShaderInfoLog(vShader, logLen, &written, log);
			cerr << "vertex shader compile log : " << endl;
			cerr << log << endl;
			free(log);//释放空间    
		}
	}

	//创建着色器对象：片断着色器    
	fShader = glCreateShader(GL_FRAGMENT_SHADER);
	//错误检测    
	if (0 == fShader)
	{
		cerr << "ERROR : Create fragment shader failed" << endl;
		exit(1);
	}

	//把着色器源代码和着色器对象相关联    
	const GLchar* fShaderCode = textFileRead(FShaderFile);
	const GLchar* fCodeArray[1] = { fShaderCode };
	glShaderSource(fShader, 1, fCodeArray, NULL);

	//编译着色器对象    
	glCompileShader(fShader);

	//检查编译是否成功    
	glGetShaderiv(fShader, GL_COMPILE_STATUS, &compileResult);
	if (GL_FALSE == compileResult)
	{
		GLint logLen;
		//得到编译日志长度    
		glGetShaderiv(fShader, GL_INFO_LOG_LENGTH, &logLen);
		if (logLen > 0)
		{
			char* log = (char*)malloc(logLen);
			GLsizei written;
			//得到日志信息并输出    
			glGetShaderInfoLog(fShader, logLen, &written, log);
			cerr << "fragment shader compile log : " << endl;
			cerr << log << endl;
			free(log);//释放空间    
		}
	}
	//3、链接着色器对象    
	//创建着色器程序    
	programHandle = glCreateProgram();
	if (!programHandle)
	{
		cerr << "ERROR : create program failed" << endl;
		exit(1);
	}
	//将着色器程序链接到所创建的程序中    
	glAttachShader(programHandle, vShader);
	glAttachShader(programHandle, fShader);
	//将这些对象链接成一个可执行程序    
	glLinkProgram(programHandle);
	//查询链接的结果    
	GLint linkStatus;
	glGetProgramiv(programHandle, GL_LINK_STATUS, &linkStatus);
	if (GL_FALSE == linkStatus)
	{
		cerr << "ERROR : link shader program failed" << endl;
		GLint logLen;
		glGetProgramiv(programHandle, GL_INFO_LOG_LENGTH,
			&logLen);
		if (logLen > 0)
		{
			char* log = (char*)malloc(logLen);
			GLsizei written;
			glGetProgramInfoLog(programHandle, logLen,
				&written, log);
			cerr << "Program log : " << endl;
			cerr << log << endl;
		}
	}
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
	gluLookAt(camX, camY, camZ, 0, 0, 0, 0, 1, 0);


	glRotatef(rx1, 0, 1, 0);
	glRotatef(ry1, 1, 0, 0);


	glColor3f(1.0, 1.0, 1.0);
	glUseProgram(programHandle);
	glUniform3f(glGetUniformLocation(programHandle, "l_dir"), l_dir[0], l_dir[1], l_dir[2]);
	glUniform4f(glGetUniformLocation(programHandle, "l_ambient"), l_ambient[0], l_ambient[1], l_ambient[2], l_ambient[3]);
	glUniform4f(glGetUniformLocation(programHandle, "l_diffuse"), l_diffuse[0], l_diffuse[1], l_diffuse[2], l_diffuse[3]);
	glUniform4f(glGetUniformLocation(programHandle, "l_specular"), l_specular[0], l_specular[1], l_specular[2], l_specular[3]);
	glUniform1f(glGetUniformLocation(programHandle, "l_shininess"), l_shininess);

	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, faceNum * 3, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	
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

void mouseWheel(int wheel, int direction, int x, int y) {
	if (direction > 0) {
		camZ += 20;
	}
	else if (direction < 0) {
		camZ -= 20;
	}
}

void idle()
{
	glutPostRedisplay();
}

void initVBO() {
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vertexNum * 3, vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * faceNum * 3, indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void init() {
	GLenum err = glewInit();
	if (err != GLEW_OK) {
		cout << "Error initializing GLEW: " << glewGetErrorString(err) << endl;
	}

	glEnable(GL_DEPTH_TEST);

	initShader("shaders/Vert.vert", "shaders/Frag.frag");
	glClearColor(0.5, 0.5, 0.5, 1.0);
	initVBO();
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

	init();

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutMouseWheelFunc(mouseWheel);
	glutIdleFunc(idle);

	glutMainLoop();
	return 1;
}