#ifndef __MY_CAMERA__
#define __MY_CAMERA__
#define GLM_FORCE_SIMD_AVX2
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/geometric.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include "GL/glut.h"
#include "math.h"

#include "util.hpp"

float err = 1e-6;

glm::fquat getQuatFor(glm::vec3 v1, glm::vec3 v2){ // v1,v2 must be normalized
    glm::vec3 axis = glm::normalize(glm::cross(v1,v2));
    auto cosVal = glm::dot(v1,v2);
    auto sinVal = sqrtf32(1-cosVal*cosVal);
    float rotDeg = atan2(sinVal,cosVal);
    return glm::angleAxis<float,glm::packed_highp>(rotDeg,axis);;
}

class Cam{
    glm::vec3 tmp_camOri;
    glm::vec3 tmp_camUp;
    float A; // tan^2(fovx/2)+tan^2(fovy/2)
public:
    glm::vec3 camOffset;
    glm::vec3 camOri;
    glm::vec3 camUp;
    glm::vec3 camRight;
    float r;
    float fovy;
    Cam(){
        this->camOffset = glm::vec3(0.0f,0.0f,0.0f);
        this->camOri = this->tmp_camOri = glm::normalize(glm::vec3(0.0,0.0,1.0));
        this->camUp = this->tmp_camUp = glm::normalize(glm::vec3(0.0,1.0,0.0));
        this->camRight = glm::normalize(glm::cross(this->camUp,this->camOri));
        this->r = 50.0f;
        this->fovy = 45.0f;
    }
    void viewMatrix(){
        auto camPos = (this->r * this->tmp_camOri) + this->camOffset;
        gluLookAt(camPos[0],camPos[1],camPos[2],
        this->camOffset[0],this->camOffset[1],this->camOffset[2],
        //0.0,1.0,0.0);
        this->tmp_camUp[0],this->tmp_camUp[1],this->tmp_camUp[2]);
    }
    void rot(glm::vec3 v1, glm::vec3 v2){
        auto _v1 = -v1[0]*glm::cross(this->camOri,this->camUp) + v1[1]*this->camUp + v1[2]*this->camOri;
        auto _v2 = -v2[0]*glm::cross(this->camOri,this->camUp) + v2[1]*this->camUp + v2[2]*this->camOri;
        auto q = getQuatFor(_v1,_v2);
        //q = glm::inverse(q);
        auto rotMat = glm::toMat4(q);
        this->tmp_camOri = glm::normalize(glm::vec3(glm::vec4(this->camOri,1.0f)*rotMat));
        this->tmp_camUp  = glm::normalize(glm::vec3(glm::vec4(this->camUp,1.0f)*rotMat));
    }
    void cameraLock(){
        this->camOri = this->tmp_camOri;
        this->camUp = this->tmp_camUp;
        this->camRight = glm::normalize(glm::cross(this->camUp,this->camOri));
    }
    float getTrackBallSize(int w,int h){
        auto _fovy = this->fovy*M_PI/180.0;
        auto _fovx = getFovX(this->fovy,w,h);
        _fovy = _fovy*M_PI/180.0;
        _fovx = _fovx*M_PI/180.0;
        auto X = tanf(_fovx/2.0);
        auto Y = tanf(_fovy/2.0);
        this->A = X*X+Y*Y;
        return (this->A-sqrtf(this->A))*this->r/(this->A-1);
    }
    void getOptimalR(float ballSize){
        this->r = (this->A-1.0f)*ballSize/(this->A-sqrtf(this->A));
    }
};
#endif