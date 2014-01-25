#include <stdio.h>
#include <stdlib.h>
#include "glew\include\GL\glew.h"
#include "glut\glut.h"
#include <memory.h>
#include <stack>
#include <math.h>
#include "mymath.h"
using namespace std;
#define PI       3.14159265358979323846
static bool gl_modeview=true;
static bool gl_projection=false;
static stack<float*> modeview_stack;
static stack<float*> project_stack;
void my_gl_translatef(float x, float y, float z);
void my_gl_scalaf(float x, float y, float z);
void my_gl_rotatef(float ang,float x, float y, float z);
void my_gl_pushMatrix();
void my_gl_popMatrix();
void my_gl_loadidentity();
void my_gl_matrixmode(GLenum num);
void my_gl_perspective(float fovy, float aspect, float zNear, float zFar);
void gl_init();
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
 
// vertices for triangle
float vertices1[] = {   -1.0f, -1.0f, -5.0f,
            1.0f, -1.0f, -5.0f,
            0.0f, 1.0f, -5.0f};
 
float colors1[] = { 1.0f, 0.0f, 0.0f, 1.0f,
            0.0f, 1.0f, 0.0f, 1.0f,
            0.0f,0.0f, 1.0f, 1.0f};
 
 
// shader names
char *vertexFileName = "vertex.txt";
char *fragmentFileName = "frag.txt";
float ang=0;
// program and shader Id
GLuint p,v,f;
 
// vert attrib locations
GLuint vertexLoc, colorLoc,axisLoc,angLoc;
GLuint fovLoc,ratioLoc,nearPLoc,farPLoc;
// uniform var locations
GLuint projMatrixLoc, viewMatrixLoc;
 
// vert array obj Id
GLuint vert[3];
 
// storage for matrices
float* projMatrix=new float[16];
float* viewMatrix=new float[16];

int frame=0,time,timebase=0;
char s[50];
void gl_init()
{
        float* tmp=new float[16];
        setIdentMatrix(tmp,4);
        if(gl_projection)
                project_stack.push(tmp);
        else
                modeview_stack.push(tmp);
}
void my_gl_translatef(float x,float y, float z)
{
        float *tmp=new float[16];
        float *result=new float[16];
        if(gl_projection)
        {
                if(project_stack.size()==0)
                        gl_init();
                result=project_stack.top();
        }
        else
        {
                if(modeview_stack.size()==0)
                        gl_init();
                result=modeview_stack.top();
        }
        setTransMatrix(tmp,x,y,z);
        multiplyMatrix(result,tmp);
        if(gl_projection)
        {
                project_stack.pop();
                project_stack.push(result);
        }
        else
        {
                modeview_stack.pop();
                modeview_stack.push(result);
        }
}
void my_gl_scalaf(float x,float y,float z)
{
        float *tmp=new float[16];
        float *result=new float[16];
        if(modeview_stack.size()==0)
                gl_init();
        result=modeview_stack.top();
        for (int i = 0; i < 4 * 4; ++i)
            tmp[i] = 0.0f;
        tmp[0]=x;tmp[5]=y;tmp[10]=z;
        multiplyMatrix(result,tmp);
        modeview_stack.pop();
        modeview_stack.push(result);
}
void my_gl_rotatef(float ang,float x, float y, float z)
{
        float *tmp=new float[16];
        float *result=new float[16];
        if(gl_projection)
        {
                if(project_stack.size()==0)
                        gl_init();
                result=project_stack.top();
        }
        else
        {
                if(modeview_stack.size()==0)
                        gl_init();
                result=modeview_stack.top();
        }
        tmp=rotationMatrix(x,y,z,ang);
        multiplyMatrix(result,tmp);
        if(gl_projection)
        {
                project_stack.pop();
                project_stack.push(result);
        }
        else
        {
                modeview_stack.pop();
                modeview_stack.push(result);
        }
}
void my_gl_pushMatrix()
{
        float *tmp=new float[16];
        if(gl_projection)
        {
                tmp=project_stack.top();
                project_stack.push(tmp);
        }
        else
        {
                tmp=modeview_stack.top();
                modeview_stack.push(tmp);
        }
}
void my_gl_popmatrix()
{
        float *tmp=new float[16];
        if(gl_projection)
        {
                project_stack.pop();
        }
        else
        {
                modeview_stack.pop();
        }
}
// vector opt
// res = a cross b;
void xProduct( float *a, float *b, float *res) 
{
 
    res[0] = a[1] * b[2]  -  b[1] * a[2];
    res[1] = a[2] * b[0]  -  b[2] * a[0];
    res[2] = a[0] * b[1]  -  b[0] * a[1];
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
        float* tmp=new float[16];
    tmp[0]  = right[0];
        tmp[1]  = up[0];
        tmp[2]  = -dir[0];
        tmp[3]  = 0.0f;
    tmp[4]  = right[1];
        tmp[5]  = up[1];
        tmp[6]  = -dir[1];
        tmp[7]  = 0.0f;
    tmp[8]  = right[2];
        tmp[9]  = up[2];
        tmp[10] = -dir[2];
        tmp[11] = 0.0f;
    tmp[12] = 0.0f;
    tmp[13] = 0.0f;
    tmp[14] =  0.0f;
    tmp[15] = 1.0f;
    setTransMatrix(aux, -posX, -posY, -posZ);
    multiplyMatrix(tmp, aux);
        if(modeview_stack.size()==0)
                gl_init();
        modeview_stack.pop();
        modeview_stack.push(tmp);
        // you should do this instead. If you want to apply rotation to your viewMatrix.
        
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
 
        float *tmp=new float[16];
    float f = 1.0f / tan (fov * (PI / 360.0));
    setIdentMatrix(tmp,4);
    tmp[0] = f / ratio;
    tmp[1 * 4 + 1] = f;
    tmp[2 * 4 + 2] = (farP + nearP) / (nearP - farP);
    tmp[3 * 4 + 2] = (2.0f * farP * nearP) / (nearP - farP);
    tmp[2 * 4 + 3] = -1.0f;
    tmp[3 * 4 + 3] = 0.0f;
        if(project_stack.size()==0)
                gl_init();
        project_stack.pop();
        project_stack.push(tmp);
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
 
    GLuint buffers[2];
 
    glGenVertexArrays(1, vert);

    // first triangle
    glBindVertexArray(vert[0]);
    // generate 2 buffers for vert and color
    glGenBuffers(2, buffers);
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
}
 
void setUniforms() {
 
    // must be called after glUseProgram
	// set the variables for the shader
        projMatrix=project_stack.top();
        viewMatrix=modeview_stack.top();
        float vec[]={0,1,0,0};
    glUniformMatrix4fv(projMatrixLoc,  1, false, projMatrix);
    glUniformMatrix4fv(viewMatrixLoc,  1, false, viewMatrix);
        glUniform4fv(axisLoc,1,vec);
        glUniform1f(angLoc,ang);

        float ratio;

    // place viewport to be the entire window
    ratio = (1.0f * 640) / 480;
    buildProjMatrix(53.13f, ratio, 1.0f, 30.0f);


        glUniform1f(fovLoc,53.13);
        glUniform1f(ratioLoc,ratio);
        glUniform1f(nearPLoc,1);
        glUniform1f(farPLoc,30);
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
 
    //placeCam(10,2,10,0,2,-5);
	placeCam(0,0,0,0,0,-5);
        //my_gl_rotatef(ang,0.0,1.0,0.0); 
    glUseProgram(p);
    setUniforms();
 
    glBindVertexArray(vert[0]);
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
    projMatrixLoc = glGetUniformLocation(p, "projMatrix");
    viewMatrixLoc = glGetUniformLocation(p, "viewMatrix");
        axisLoc=glGetUniformLocation(p, "axis_i");
    angLoc=glGetUniformLocation(p, "ang_i");
        fovLoc=glGetUniformLocation(p, "fov");
    ratioLoc=glGetUniformLocation(p, "ratio");
        nearPLoc=glGetUniformLocation(p, "nearP");
    farPLoc=glGetUniformLocation(p, "farP");
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
void idle()
{
        ang+=0.001;
        glutPostRedisplay( );
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
    glutIdleFunc(idle);
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