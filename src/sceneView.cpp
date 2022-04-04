

#include <iostream>
#include "math.h"
#include <GL/glut.h>
#define GLM_FORCE_SIMD_AVX2

#include "util.hpp"
#include "cam.hpp"
#include "Parser.hpp"
#include "plane.hpp"
#include "scene.hpp"
#include "light.hpp"
#include "Render.hpp"
#include "parameters.hpp"

static unsigned int width = 320;
static unsigned int height = 240;

static bool mouseRotatePressed = false;
static bool mouseMovePressed   = false;
static bool mouseZoomPressed   = false;

static int lastX = 0;
static int lastY = 0;
static float lastFov = 0.0f;
static float lastR = 0.0f;
static glm::vec3 lastOffset;

static bool fullScreen = false;
static float far = 100000.0;
static float near = 0.1;

static Cam camera;
static float sensitivity = 1.0f;
static float senseZoom = 0.1;
static float senseR = 0.01;
static float senseTL = 0.01;

static float TSIZE;

static bool bDrawTarget = false;
static bool bDrawTrackball = false;


Renderer* renderer = nullptr;

Scene* scene;
int displayMode = 0;
void drawSomething()
{
// Draw something Here
    if(bDrawTarget){
        glPushMatrix();
            glColor3f(1.0,1.0,0.0);
            glTranslatef(camera.camOffset[0],camera.camOffset[1],camera.camOffset[2]);
            glutSolidSphere(0.1,15,10);
        glPopMatrix();
    }
    if(bDrawTrackball){
        glPushMatrix();
            glColor3f(1.0,1.0,0.0);
            glTranslatef(camera.camOffset[0],camera.camOffset[1],camera.camOffset[2]);
            glutWireSphere(TSIZE,20,10);
        glPopMatrix();
    }
    //model->draw(displayMode);
    scene->draw();
}

void reshape(int w, int h)
{
    width = glutGet(GLUT_WINDOW_WIDTH);
    height = glutGet(GLUT_WINDOW_HEIGHT);

    glViewport(0, 0, w, h );

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	float aspectRatio = (float)w/(float)h;
	gluPerspective( camera.fovy /*field of view angle*/,
					aspectRatio,
					near /*near clipping plane*/,
					far /* far clipping plane */ );
    TSIZE = camera.getTrackBallSize(w,h);
    camera.viewMatrix();
	glMatrixMode( GL_MODELVIEW );
}
void display()
{
    glLoadIdentity();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    drawSomething();
    glFlush();
    glutSwapBuffers();
}
void keyboardCB(unsigned char keyPressed, int x, int y)
{
    switch (keyPressed) {
    case 'f':
        if (fullScreen == true) {
            glutReshapeWindow(width,height);
            fullScreen = false;
        } else {
            glutFullScreen();
            fullScreen = true;
        }
        break;
    case 'q':
        exit(0);
        break;
    case 't':
        bDrawTarget = !bDrawTarget;
        break;
    case 'b':
        bDrawTrackball = !bDrawTrackball;
        break;
    case 'a':
        stbi_write_png("test.png",textures[2].w,textures[2].h,textures[2].c,textures[2].data,textures[2].w*textures[2].c);
        break;
    case 'w':
        if(bWireMode){
            glPolygonMode(GL_FRONT,GL_FILL);
            //glEnable(GL_LIGHTING);
        }else{
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            //glDisable(GL_LIGHTING);
        }
        bWireMode = !bWireMode;
        break;
    case 'r':
        int _w,_h;
        std::cout<<" Width : ";
        std::cin>>_w;
        std::cout<<" Height : ";
        std::cin>>_h;
        glutReshapeWindow(_w,_h);
        break;
    case 'd':
        int _d;
        std::cout<<" Depth : ";
        std::cin>>_d;
        render_kd_depth = _d;
        break;
    case 'p':
        renderer = new Renderer(&camera,scene, width,height,1000.0f);
        renderer->tree = scene->tree;
        std::cout<<"Ray tracing start"<<std::endl;
        renderer->Render();
        std::cout<<"Ray tracing done"<<std::endl;
        delete renderer;
        break;
    }
    glutPostRedisplay();
}

void mouseCB(int button, int state, int x, int y) {
    if (state == GLUT_UP) {
        mouseMovePressed   = false;
        mouseRotatePressed = false;
        mouseZoomPressed   = false;
        camera.cameraLock();
    } else {
		lastX = x;
		lastY = y;
        if(button==GLUT_LEFT_BUTTON && (GLUT_ACTIVE_SHIFT | GLUT_ACTIVE_CTRL)==glutGetModifiers()){

            lastR = camera.r;

            mouseMovePressed   = true;
            mouseRotatePressed = false;
            mouseZoomPressed   = true;
        }
        else if (button==GLUT_LEFT_BUTTON && GLUT_ACTIVE_SHIFT==glutGetModifiers())
        {
			// do something here

            lastOffset = camera.camOffset;

            mouseMovePressed   = true;
            mouseRotatePressed = false;
            mouseZoomPressed   = false;
        }
        else if (button==GLUT_LEFT_BUTTON && GLUT_ACTIVE_CTRL==glutGetModifiers())
        {
			// do something here

            lastFov = camera.fovy;

            mouseMovePressed   = false;
            mouseRotatePressed = false;
            mouseZoomPressed   = true;
        }
        else if (button==GLUT_LEFT_BUTTON)
        {
			// do something here

            mouseMovePressed   = false;
            mouseRotatePressed = true;
            mouseZoomPressed   = false;
        }
    }
    glutPostRedisplay();
}

void motionCB(int x, int y) {
    if(x > width || x < 0 || y > height || y < 0){

    }
    else if (mouseRotatePressed == true)
	{
        // do something here
        auto lastS3 = getTrackballPos(width,height,lastX,lastY);
        auto S3 = getTrackballPos(width,height,x,y);
        camera.rot(lastS3,S3);
	}
    else if (mouseMovePressed && mouseZoomPressed){
        // dolly in & out
        camera.r = lastR + sensitivity*(y-lastY)*senseR;

    }
    else if (mouseMovePressed == true)
    {
	    // do something here
        camera.camOffset = lastOffset -
            sensitivity*(x-lastX)*senseTL*glm::cross(camera.camUp,camera.camOri) +
            sensitivity*(y-lastY)*senseTL*camera.camUp;

    }
    else if (mouseZoomPressed == true)
    {
		// do something here
        camera.fovy = lastFov + sensitivity*(x-lastX)*senseZoom;
    }
	reshape(width, height);
    glutPostRedisplay();
}

void idle() { }//glutPostRedisplay(); }
// no animation

int main(int argc, char** argv) {
    Parser parser;
    if(argc!=2){
        std::cout<<"[usage] $ "<<argv[0]<<" [scene filename]"<<std::endl;
        exit(1);
    }

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
    glutInitWindowSize(width, height);
    glutCreateWindow("3D viewer");

    reshape(width, height);

    glClearDepth(1.0f);
    glFrontFace(GL_CCW);
    //glFrontFace(GL_CW);
    glDepthFunc(GL_LESS);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glPolygonMode(GL_FRONT, GL_FILL);
    std::cout<<"Read Scene file : "<<argv[1]<<std::endl;
    scene = new Scene(argv[1]);
    glClearColor(color_sky[0], color_sky[1], color_sky[2], 1.0);

    //float globalAmbient[] = {0.2f,0.2f,0.2f,1.0f};
    //glLightModelfv(GL_LIGHT_MODEL_AMBIENT,globalAmbient);

    glutIdleFunc(idle);
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboardCB);
    glutReshapeFunc(reshape);
    glutMotionFunc(motionCB);
    glutMouseFunc(mouseCB);

    glutMainLoop();
    return 0;
}
