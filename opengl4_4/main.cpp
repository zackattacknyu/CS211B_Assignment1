#include <stdio.h>
#include <stdlib.h>
#include "glew\include\GL\glew.h"
#include "glut\glut.h"
#include <memory.h>
#include <math.h>
#include "mymath.h"
#include <stack>

using namespace std;
 
#define PI       3.14159265358979323846

void setTransMatrix(float *mat, float x, float y, float z);
void multiplyMatrix(float *a, float *b);
void xProduct( float *a, float *b, float *res);
void normalize(float *a);
void setIdentMatrix( float *mat, int size);
void placeCam(float posX, float posY, float posZ, float lookX, float lookY, float lookZ);
void setTransMatrix(float *mat, float x, float y, float z);
float * rotationMatrix(float x, float y, float z, float angle);

struct vec3
{
	float x;
	float y;
	float z;
};

//transparency values
float triangle1Alpha = 0.5f;
float triangle2Alpha = 0.5f;

// vertices for triangle 1
float vertices1[] = {   -1.0f, -1.0f, -5.0f,
            1.0f, -1.0f, -5.0f,
            0.0f, 1.0f, -4.0f};
float normals1[] = {   0.0f, -2.0f, 4.0f,
            0.0f, -2.0f, 4.0f,
            0.0f, 0.0f, 4.0f,};
float colors1[] = { 1.0f, 0.0f, 0.0f, triangle1Alpha,
            1.0f, 0.0f, 0.0f, triangle1Alpha,
            1.0f, 0.0f, 0.0f, triangle1Alpha};

//vertices for other triangle
float vertices2[] = {   -1.0f, -1.0f, -3.0f,
            1.0f, -1.0f, -3.0f,
            0.0f, 1.0f, -4.0f};
float normals2[] = {   0.0f, 2.0f, 4.0f,
            0.0f, 2.0f, 4.0f,
            0.0f, 0.0f, 4.0f,};
float colors2[] = { 0.0f, 1.0f, 0.0f, triangle2Alpha,
            0.0f, 1.0f, 0.0f, triangle2Alpha,
            0.0f, 1.0f, 0.0f, triangle2Alpha};

//vertices for 3rd triangle 
float vertices3[] = {   -1.0f, -1.0f, -3.0f,
            -1.0f, -1.0f, -5.0f,
            0.0f, 1.0f, -4.0f};
float normals3[] = {   0.0f, 2.0f, 4.0f,
            0.0f, 2.0f, 4.0f,
            0.0f, 0.0f, 4.0f,};
float colors3[] = { 0.0f, 0.0f, 1.0f, triangle2Alpha,
            0.0f, 0.0f, 1.0f, triangle2Alpha,
            0.0f, 0.0f, 1.0f, triangle2Alpha};

//vertices for 4th triangle 
float vertices4[] = {   1.0f, -1.0f, -3.0f,
            1.0f, -1.0f, -5.0f,
            0.0f, 1.0f, -4.0f};
float normals4[] = {   0.0f, 2.0f, 4.0f,
            0.0f, 2.0f, 4.0f,
            0.0f, 0.0f, 4.0f,};
float colors4[] = { 1.0f, 0.0f, 1.0f, triangle2Alpha,
            1.0f, 0.0f, 1.0f, triangle2Alpha,
            1.0f, 0.0f, 1.0f, triangle2Alpha};
 
 
// shader names
char *vertexFileName = "vertex.txt";
char *fragmentFileName = "frag.txt";
 
// program and shader Id
GLuint p,v,f;
 
// vert attrib locations
GLuint vertexLoc, colorLoc,normalLoc;
GLuint angleLoc;
GLuint axisLocation, fovLocation, ratioLocation, nearPlaneLocation, farPlaneLocation;
 
// uniform var locations
GLuint projMatrixLoc, viewMatrixLoc;

//matrix stack
static bool currentMatrix_modelView = true;
static bool currentMatrix_projection = false;
static stack<float*> modelViewMatrix_stack;
static stack<float*> projectionMatrix_stack;
 
// vert array obj Id
GLuint vert[10];
 
// storage for matrices
float* projMatrix = new float[16];
float* viewMatrix = new float[16];

float myAngle = 0.0f;

int frame=0,time,timebase=0;
char s[50];
 
// vector opt
// res = a cross b;
void xProduct( float *a, float *b, float *res) 
{
 
    res[0] = a[1] * b[2]  -  b[1] * a[2];
    res[1] = a[2] * b[0]  -  b[2] * a[0];
    res[2] = a[0] * b[1]  -  b[0] * a[1];
}

void zrd_glInit(){

	float* init = new float[16];
	setIdentMatrix(init,4);

	if(currentMatrix_projection){
		projectionMatrix_stack.push(init);
	}else{
		modelViewMatrix_stack.push(init);
	}

}

void zrd_glRotatef(float angle, float x, float y, float z){
	float* rotMatrix = new float[16];
	float* resultMatrix = new float[16];

	if(currentMatrix_projection){
		if(projectionMatrix_stack.size() == 0){
			zrd_glInit();
		}
		resultMatrix = projectionMatrix_stack.top();
	}else{
		if(modelViewMatrix_stack.size() == 0){
			zrd_glInit();
		}
		resultMatrix = modelViewMatrix_stack.top();
	}

	rotMatrix = rotationMatrix(x,y,z,angle);
	multiplyMatrix(resultMatrix,rotMatrix);	

	if(currentMatrix_projection){
		projectionMatrix_stack.pop();
		projectionMatrix_stack.push(resultMatrix);
	}else{
		modelViewMatrix_stack.pop();
		modelViewMatrix_stack.push(resultMatrix);
	}



}
 
// normalize a vec3
void normalize(float *a) 
{
 
    float mag = sqrt(a[0] * a[0]  +  a[1] * a[1]  +  a[2] * a[2]);
    a[0] /= mag;
    a[1] /= mag;
    a[2] /= mag;
}

// Matrix Opt. - In Opengl 3 we need to handle our own matrix.
 
// In this form : a = a * b; 
void multiplyMatrix(float *a, float *b) 
{
    float res[16];
 
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            res[j*4 + i] = 0.0f;
            for (int k = 0; k < 4; ++k) {
                res[j*4 + i] += a[k*4 + i] * b[j*4 + k];
            }
        }
    }
    memcpy(a, res, 16 * sizeof(float));
}

// sets the square matrix mat to the ID matrix,
void setIdentMatrix( float *mat, int size) 
{
 
    // 0s
    for (int i = 0; i < size * size; ++i)
            mat[i] = 0.0f;
 
    // diagonal 1s
    for (int i = 0; i < size; ++i)
        mat[i + i * size] = 1.0f;
}
 
// View Matrix
// just like glulookat
void placeCam(float posX, float posY, float posZ, float lookX, float lookY, float lookZ) 
{
 
    float dir[3], right[3], up[3];
 
    up[0] = 0.0f;   up[1] = 1.0f;   up[2] = 0.0f;
 
    dir[0] =  (lookX - posX);
    dir[1] =  (lookY - posY);
    dir[2] =  (lookZ - posZ);
    normalize(dir);
 
	// this order matters
    xProduct(dir,up,right);
    normalize(right);
 
    xProduct(right,dir,up);
    normalize(up);
 
    float aux[16];
	
	float* tempViewMatrix = new float[16];
    tempViewMatrix[0]  = right[0];
	tempViewMatrix[1]  = up[0];
	tempViewMatrix[2]  = -dir[0];
	tempViewMatrix[3]  = 0.0f;
    tempViewMatrix[4]  = right[1];
	tempViewMatrix[5]  = up[1];
	tempViewMatrix[6]  = -dir[1];
	tempViewMatrix[7]  = 0.0f;
    tempViewMatrix[8]  = right[2];
	tempViewMatrix[9]  = up[2];
	tempViewMatrix[10] = -dir[2];
	tempViewMatrix[11] = 0.0f;
    tempViewMatrix[12] = 0.0f;
    tempViewMatrix[13] = 0.0f;
    tempViewMatrix[14] =  0.0f;
    tempViewMatrix[15] = 1.0f;
    setTransMatrix(aux, -posX, -posY, -posZ);
    multiplyMatrix(tempViewMatrix, aux);

	if(modelViewMatrix_stack.size() == 0){
		zrd_glInit();
	}

	modelViewMatrix_stack.pop();
	modelViewMatrix_stack.push(tempViewMatrix);
	
	// you should do this instead. If you want to apply rotation to your viewMatrix.
	//multiplyMatrix(viewMatrix, rotationMatrix(0.0,0.0,1.0, 0.785)); 
}

// Generates a rotation matrix.  Angle is in radian.
float * rotationMatrix(float x, float y, float z, float angle)
{
	float tempVec[3];
	tempVec[0] = x;
	tempVec[1] = y;
	tempVec[2] = z;
    normalize(tempVec);

	vec3 axis;
	axis.x = tempVec[0];
	axis.y = tempVec[1];
	axis.z = tempVec[2];

    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;

	float mat[16] = {oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
				0.0,                                0.0,                                0.0,                                1.0};

    return mat;

}
 
// Projection Matrix
void buildProjMatrix(float fov, float ratio, float nearP, float farP) {
 
    float f = 1.0f / tan (fov * (PI / 360.0));
	float* projectionMatrix = new float[16];
    setIdentMatrix(projectionMatrix,4);
    projectionMatrix[0] = f / ratio;
    projectionMatrix[1 * 4 + 1] = f;
    projectionMatrix[2 * 4 + 2] = (farP + nearP) / (nearP - farP);
    projectionMatrix[3 * 4 + 2] = (2.0f * farP * nearP) / (nearP - farP);
    projectionMatrix[2 * 4 + 3] = -1.0f;
    projectionMatrix[3 * 4 + 3] = 0.0f;

	if(projectionMatrix_stack.size() == 0){
		zrd_glInit();
	}
	projectionMatrix_stack.pop();
	projectionMatrix_stack.push(projectionMatrix);
}

// Transformation matrix mat with a translation
void setTransMatrix(float *mat, float x, float y, float z) {
 
    setIdentMatrix(mat,4);
    mat[12] = x;
    mat[13] = y;
    mat[14] = z;
}

void changeSize(int w, int h) {
 
    float ratio;

    // place viewport to be the entire window
    glViewport(0, 0, w, h);
    ratio = (1.0f * w) / h;
    buildProjMatrix(53.13f, ratio, 1.0f, 30.0f);
}
 
void setupBuffers() {
 
    GLuint buffers[3];
	GLuint nextBuffers[3];
	GLuint thirdBuffers[3];
	GLuint fourthBuffers[3];
 
    glGenVertexArrays(4, vert);

	//-----------------------------------------------------
    // first triangle
    glBindVertexArray(vert[0]);
    // generate 2 buffers for vert and color
    glGenBuffers(3, buffers);

    // bind buffer for vertices and copy data into buffer
    glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices1), vertices1, GL_STATIC_DRAW);
    glEnableVertexAttribArray(vertexLoc);
    glVertexAttribPointer(vertexLoc, 3, GL_FLOAT, 0, 0, 0);
 
    // bind buffer for colors and copy data into buffer
    glBindBuffer(GL_ARRAY_BUFFER, buffers[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(colors1), colors1, GL_STATIC_DRAW);
    glEnableVertexAttribArray(colorLoc);
    glVertexAttribPointer(colorLoc, 4, GL_FLOAT, 0, 0, 0);

	//bind buffer for normals and copy data into buffer
	glBindBuffer(GL_ARRAY_BUFFER,buffers[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(normals1), normals1, GL_STATIC_DRAW);
	glEnableVertexAttribArray(normalLoc);
    glVertexAttribPointer(normalLoc, 3, GL_FLOAT, 0, 0, 0);

	//--------------------------------------------------------
	// second triangle
    glBindVertexArray(vert[1]);
    // generate 2 buffers for vert and color
	glGenBuffers(3, nextBuffers);
    // bind buffer for vertices and copy data into buffer
	glBindBuffer(GL_ARRAY_BUFFER, nextBuffers[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices2), vertices2, GL_STATIC_DRAW);
    glEnableVertexAttribArray(vertexLoc);
    glVertexAttribPointer(vertexLoc, 3, GL_FLOAT, 0, 0, 0);
 
    // bind buffer for colors and copy data into buffer
    glBindBuffer(GL_ARRAY_BUFFER, nextBuffers[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(colors2), colors2, GL_STATIC_DRAW);
    glEnableVertexAttribArray(colorLoc);
    glVertexAttribPointer(colorLoc, 4, GL_FLOAT, 0, 0, 0);

	//bind buffer for normals and copy data into buffer
	glBindBuffer(GL_ARRAY_BUFFER,nextBuffers[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(normals2), normals2, GL_STATIC_DRAW);
	glEnableVertexAttribArray(normalLoc);
    glVertexAttribPointer(normalLoc, 3, GL_FLOAT, 0, 0, 0);

	//--------------------------------------------------------
	// third triangle
    glBindVertexArray(vert[2]);
    // generate 2 buffers for vert and color
	glGenBuffers(3, thirdBuffers);
    // bind buffer for vertices and copy data into buffer
	glBindBuffer(GL_ARRAY_BUFFER, thirdBuffers[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices3), vertices3, GL_STATIC_DRAW);
    glEnableVertexAttribArray(vertexLoc);
    glVertexAttribPointer(vertexLoc, 3, GL_FLOAT, 0, 0, 0);
 
    // bind buffer for colors and copy data into buffer
    glBindBuffer(GL_ARRAY_BUFFER, thirdBuffers[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(colors3), colors3, GL_STATIC_DRAW);
    glEnableVertexAttribArray(colorLoc);
    glVertexAttribPointer(colorLoc, 4, GL_FLOAT, 0, 0, 0);

	//bind buffer for normals and copy data into buffer
	glBindBuffer(GL_ARRAY_BUFFER,thirdBuffers[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(normals3), normals3, GL_STATIC_DRAW);
	glEnableVertexAttribArray(normalLoc);
    glVertexAttribPointer(normalLoc, 3, GL_FLOAT, 0, 0, 0);

	//--------------------------------------------------------
	// fourth triangle
    glBindVertexArray(vert[3]);
    // generate 2 buffers for vert and color
	glGenBuffers(3, fourthBuffers);
    // bind buffer for vertices and copy data into buffer
	glBindBuffer(GL_ARRAY_BUFFER, fourthBuffers[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices4), vertices4, GL_STATIC_DRAW);
    glEnableVertexAttribArray(vertexLoc);
    glVertexAttribPointer(vertexLoc, 3, GL_FLOAT, 0, 0, 0);
 
    // bind buffer for colors and copy data into buffer
    glBindBuffer(GL_ARRAY_BUFFER, fourthBuffers[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(colors4), colors4, GL_STATIC_DRAW);
    glEnableVertexAttribArray(colorLoc);
    glVertexAttribPointer(colorLoc, 4, GL_FLOAT, 0, 0, 0);

	//bind buffer for normals and copy data into buffer
	glBindBuffer(GL_ARRAY_BUFFER,fourthBuffers[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(normals4), normals4, GL_STATIC_DRAW);
	glEnableVertexAttribArray(normalLoc);
    glVertexAttribPointer(normalLoc, 3, GL_FLOAT, 0, 0, 0);
}
 
void setUniforms() {
 
    // must be called after glUseProgram
	// set the variables for the shader
	projMatrix = projectionMatrix_stack.top();
	viewMatrix = modelViewMatrix_stack.top();
    glUniformMatrix4fv(projMatrixLoc,  1, false, projMatrix);
    glUniformMatrix4fv(viewMatrixLoc,  1, false, viewMatrix);

	float vector[] = {0,1,0,0};
	glUniform4fv(axisLocation,1,vector);
	glUniform1f(angleLoc,myAngle);

	float theRatio = 640.0f/480.0f;
	buildProjMatrix(53.13f,theRatio,1.0f,30.0f);

	glUniform1f(fovLocation,53.13);
	glUniform1f(ratioLocation,theRatio);
	glUniform1f(nearPlaneLocation,1);
	glUniform1f(farPlaneLocation,30);
}
 
void renderScene(void) {

	frame++;
	time=glutGet(GLUT_ELAPSED_TIME);
	if (time - timebase > 1000) {
		sprintf(s,"FPS:%4.2f",
			frame*1000.0/(time-timebase));
		timebase = time;
		frame = 0;
	}
    glutSetWindowTitle(s);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_SRC_ALPHA);

    //placeCam(10,2,10,0,2,-5);
	placeCam(0,0,-10,0,0,-5);
	zrd_glRotatef(myAngle,0.0,1.0,0.0);
    glUseProgram(p);
    setUniforms();
 
    glBindVertexArray(vert[0]);
    glDrawArrays(GL_TRIANGLES, 0, 3);

	glBindVertexArray(vert[1]);
	glDrawArrays(GL_TRIANGLES, 0, 3);

	glBindVertexArray(vert[2]);
	glDrawArrays(GL_TRIANGLES, 0, 3);

	glBindVertexArray(vert[3]);
	glDrawArrays(GL_TRIANGLES, 0, 3);
  
   glutSwapBuffers();
}
 

void printShaderInfoLog(GLuint obj)
{
    int infologLength = 0;
    int charsWritten  = 0;
    char *infoLog;
 
    glGetShaderiv(obj, GL_INFO_LOG_LENGTH,&infologLength);
 
    if (infologLength > 0)
    {
        infoLog = (char *)malloc(infologLength);
        glGetShaderInfoLog(obj, infologLength, &charsWritten, infoLog);
        printf("%s\n",infoLog);
        free(infoLog);
    }
}
 
void printProgramInfoLog(GLuint obj)
{
    int infologLength = 0;
    int charsWritten  = 0;
    char *infoLog;
 
    glGetProgramiv(obj, GL_INFO_LOG_LENGTH,&infologLength);
 
    if (infologLength > 0)
    {
        infoLog = (char *)malloc(infologLength);
        glGetProgramInfoLog(obj, infologLength, &charsWritten, infoLog);
        printf("%s\n",infoLog);
        free(infoLog);
    }
}
 
GLuint initShaders() {
 
    char *vertShader = NULL,*fragShader = NULL;
 
    GLuint p,v,f;
 
    v = glCreateShader(GL_VERTEX_SHADER);
    f = glCreateShader(GL_FRAGMENT_SHADER);
    vertShader = getTxtFile(vertexFileName);
    fragShader = getTxtFile(fragmentFileName);
    const char * vv = vertShader;
    const char * ff = fragShader;
    glShaderSource(v, 1, &vv,NULL);
    glShaderSource(f, 1, &ff,NULL);
    free(vertShader);
	free(fragShader);
    glCompileShader(v);
    glCompileShader(f);
    printShaderInfoLog(v);
    printShaderInfoLog(f);
    p = glCreateProgram();
    glAttachShader(p,v);
    glAttachShader(p,f);
    glBindFragDataLocation(p, 0, "outputF");
    glLinkProgram(p);
    printProgramInfoLog(p);
    vertexLoc = glGetAttribLocation(p,"position");
    colorLoc = glGetAttribLocation(p, "color"); 
	normalLoc = glGetAttribLocation(p, "normal");
    projMatrixLoc = glGetUniformLocation(p, "projMatrix");
    viewMatrixLoc = glGetUniformLocation(p, "viewMatrix");
	angleLoc = glGetUniformLocation(p,"ang_i");
	nearPlaneLocation = glGetUniformLocation(p,"nearP");
	farPlaneLocation = glGetUniformLocation(p,"farP");
	axisLocation = glGetUniformLocation(p,"axis_i");
	fovLocation = glGetUniformLocation(p,"fov");
	ratioLocation = glGetUniformLocation(p,"ratio");
 
    return(p);
}

float deltaAngle = 0.0f;
int xOrigin = -1;

float angle = 0.0f;

//This event will trigger when you have a mouse button pressed down.
void mouseMove(int x, int y) 
{
	// x and y is the mouse position.
}

//This event occur when you press a mouse button.
void mouseButton(int button, int state, int x, int y) 
{
	// only start motion if the left button is pressed
	if (button == GLUT_LEFT_BUTTON) 
	{
		// when the button is released
		if (state == GLUT_UP) 
		{
		}
		else  // state = GLUT_DOWN	
		{
		}
	}
}

int iterationNumber = 0;
void rotateIdle(){
	myAngle += 0.0005;
	glutPostRedisplay();
}
 
int main(int argc, char **argv) 
{
	// sets up glut
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(100,100);
    glutInitWindowSize(640,480);
    glutCreateWindow("ICS Graphics");
    glutSetWindowTitle(s);
	// call back functions
    glutDisplayFunc(renderScene);
    glutIdleFunc(rotateIdle);
    glutReshapeFunc(changeSize);

	glutMouseFunc(mouseButton);
	glutMotionFunc(mouseMove);
 
	// check if a particular extension is available on your platform
    glewInit();
    if (glewIsSupported("GL_VERSION_3_3"))
        printf("OpenGL 3.3 is supported\n");
    else 
	{
        printf("OpenGL 3.3 not supported\n");
        exit(1);
    }
    glEnable(GL_DEPTH_TEST);
    glClearColor(1.0,1.0,1.0,1.0);
    p = initShaders(); 
    setupBuffers(); 
    glutMainLoop();
 
    return(0); 
}