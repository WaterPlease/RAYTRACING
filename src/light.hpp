#ifndef __LIGHT__
#define __LIGHT__
#define GLM_FORCE_SIMD_AVX2
#include <GL/freeglut.h>
#include <cstring>
#include <iostream>

#define LIGHTDIM (4)

class Light{
public:
    int lightID;
    float ambient[LIGHTDIM];
    float diff[LIGHTDIM];
    float spec[LIGHTDIM];
    float att[3]; // const linear quadratic
public:
    float pos[4];
    Light(int _lightID){
        this->lightID = _lightID;
        this->ambient[0] = this->ambient[1] = this->ambient[2] = 0.2f;
        this->diff[0] = this->diff[1] = this->diff[2] = 1.0f;
        this->spec[0] = this->spec[1] = this->spec[2] = 1.0f;
        this->ambient[3] = this->diff[3] = this->spec[3] = 1.0f;
        //memset(this->pos,0.0f,sizeof(float)*4);
        this->pos[0]=this->pos[1]=this->pos[2]=this->pos[3] = 0.0f;
        this->pos[1] = 28.5f;
        this->pos[3] = 1.0f;
        this->att[2] = 0.001f;
        this->att[1] = 0.015f;
        this->att[0] = 1.0f;
        glEnable(_lightID);
        this->print();
    }
    void LoadLight(){
        glLightfv(this->lightID,GL_AMBIENT,this->ambient);
        glLightfv(this->lightID,GL_DIFFUSE,this->diff);
        glLightfv(this->lightID,GL_SPECULAR,this->spec);
        glLightfv(this->lightID,GL_POSITION,this->pos);
        glLightfv(this->lightID,GL_CONSTANT_ATTENUATION,&this->att[0]);
        glLightfv(this->lightID,GL_LINEAR_ATTENUATION,&this->att[1]);
        glLightfv(this->lightID,GL_QUADRATIC_ATTENUATION,&this->att[2]);
        glEnable(this->lightID);
    }
    void setAmbient(float* _a){
        memcpy(this->ambient,_a,sizeof(float)*LIGHTDIM);
    }
    void setDiffuse(float* _d){
        memcpy(this->diff,_d,sizeof(float)*LIGHTDIM);
    }
    void setSpecular(float* _s){
        memcpy(this->spec,_s,sizeof(float)*LIGHTDIM);
    }
    void setPos(float* _pos){
        memcpy(this->pos,_pos,sizeof(float)*4);
    }
    void setAttenuation(float* _att){
        memcpy(this->att,_att,sizeof(float)*3);
        this->print();
    }
    void draw(){
        glPushMatrix();
        glTranslatef(pos[0],pos[1],pos[2]);
            glutWireSphere(1.0,20,10);
        glPopMatrix();
    }
    void print(){
        std::cout<<"====Light info===="<<std::endl;
        std::cout<<"AMBI : "<<this->ambient[0]<<", "<<this->ambient[1]<<", "<<this->ambient[2]<<", "<<this->ambient[3]<<", "<<std::endl;
        std::cout<<"DIFF : "<<this->diff[0]<<", "<<this->diff[1]<<", "<<this->diff[2]<<", "<<this->diff[3]<<", "<<std::endl;
        std::cout<<"SPEC : "<<this->spec[0]<<", "<<this->spec[1]<<", "<<this->spec[2]<<", "<<this->spec[3]<<", "<<std::endl;
        std::cout<<"ATTE : "<<this->att[0]<<", "<<this->att[1]<<", "<<this->att[2]<<", "<<std::endl;
    }
};

#endif