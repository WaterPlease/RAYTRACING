#ifndef __MATERIAL__
#define __MATERIAL__
#define GLM_FORCE_SIMD_AVX2
#include <cstring>
#include <iostream>
#include <vector>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtx/component_wise.hpp>
#include <map>
#include <cmath>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define MATDIM (3)

class Texture{
public:
    int w,h,c;
    unsigned char* data;
    GLuint glTextureID;
    Texture(const char* path){
        this->data = stbi_load(path,&this->w,&this->h,&this->c,0);
        glGenTextures( 1, &this->glTextureID );
        glBindTexture( GL_TEXTURE_2D, this->glTextureID );
        glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE,GL_MODULATE);
        glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST );
        glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR );
        glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,GL_REPEAT );
        glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,GL_REPEAT );
        gluBuild2DMipmaps( GL_TEXTURE_2D, 3, this->w, this->h,GL_RGB, GL_UNSIGNED_BYTE, this->data );
    }
    inline glm::vec3 at(int i, int j){
        auto idx = (i + j*this->w)*this->c;
        auto r = (float)this->data[idx]/255.0f;
        auto g = (float)this->data[idx+1]/255.0f;
        auto b = (float)this->data[idx+2]/255.0f;
        return glm::vec3(r,g,b);
    }
    glm::vec3 at(float u,float v){
        float tx, ty;
        v = -v;
        if(u<0.0f) u+=1.0f;
        else if(u>1.0f) u-=1.0;
        if(v<0.0f) v+=1.0f;
        else if(v>1.0f) v-=1.0;
        tx = (u*this->w);
        ty = (v*this->h);
        auto point = glm::vec2(tx,ty);
        auto tfx = glm::max<float>(glm::min<float>(floorf(tx),this->w),0);
        auto tcx = glm::max<float>(glm::min<float>(ceilf(tx),this->w),0);
        auto tfy = glm::max<float>(glm::min<float>(floorf(ty),this->h),0);
        auto tcy = glm::max<float>(glm::min<float>(ceilf(ty),this->h),0);
        auto xyArea = glm::abs(glm::compMul(glm::vec2(tcx,tcy) - point));
        auto XyArea = glm::abs(glm::compMul(glm::vec2(tfx,tcy) - point));
        auto xYArea = glm::abs(glm::compMul(glm::vec2(tcx,tfy) - point));
        auto XYArea = glm::abs(glm::compMul(glm::vec2(tfx,tfy) - point));
        auto idx = (tx + ty*this->w)*this->c;
        auto xyIdx = ((int)tfx + (int)tfy*this->w)*this->c;
        auto XyIdx = ((int)tcx + (int)tfy*this->w)*this->c;
        auto xYIdx = ((int)tfx + (int)tcy*this->w)*this->c;
        auto XYIdx = ((int)tcx + (int)tcy*this->w)*this->c;
        auto r = xyArea*(float)(this->data[xyIdx])
                +XyArea*(float)(this->data[XyIdx])
                +xYArea*(float)(this->data[xYIdx])
                +XYArea*(float)(this->data[XYIdx]);
        auto g = xyArea*(float)(this->data[xyIdx+1])
                +XyArea*(float)(this->data[XyIdx+1])
                +xYArea*(float)(this->data[xYIdx+1])
                +XYArea*(float)(this->data[XYIdx+1]);
        auto b = xyArea*(float)(this->data[xyIdx+2])
                +XyArea*(float)(this->data[XyIdx+2])
                +xYArea*(float)(this->data[xYIdx+2])
                +XYArea*(float)(this->data[XYIdx+2]);
        return (glm::vec3(r,g,b)/255.0f);
    }
};
std::vector<Texture> textures;

class Material{
public:
    float ambient[MATDIM+1];
    float diff[MATDIM+1];
    float spec[MATDIM+1];
    float shininess[1];
    float Kr;
    float Kt;
    float u_off;
    float v_off;
    float rIdx;
    float fuzzy;
    int tid;
    Material(){
        memset(this->ambient,0.2f,sizeof(float)*MATDIM);
        memset(this->diff,1.0f,sizeof(float)*MATDIM);
        memset(this->spec,1.0f,sizeof(float)*MATDIM);
        this->ambient[3] = this->diff[3] = this->spec[3] = 1.0f;
        this->shininess[0] = 16;
        this->Kr = this->Kt = 0;
        this->tid = -1;
        this->u_off = this->v_off = 0.0f;
        this->rIdx = 1.4f;
        this->fuzzy = 0.0f;
    }
    void LoadMateiral(){
        glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT,this->ambient);
        glMaterialfv(GL_FRONT_AND_BACK,GL_DIFFUSE,this->diff);
        glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,this->spec);
        glMaterialfv(GL_FRONT_AND_BACK,GL_SHININESS,this->shininess);
    }
    void setAmbient(float* _a){
        memcpy(this->ambient,_a,sizeof(float)*MATDIM);
    }
    void setDiffuse(float* _d){
        memcpy(this->diff,_d,sizeof(float)*MATDIM);
    }
    void setSpecular(float* _s){
        memcpy(this->spec,_s,sizeof(float)*MATDIM);
    }
    void setShininess(float _sh){ this->shininess[0] = _sh; }
    void setRefraction(float kt) {
        //this is for next assignment
        this->Kt = kt;
    }
    void setReflection(float kr) {
        //this is for next assignment
        this->Kr = kr;
    }
    void setTexture(int tid){
        this->tid = tid;
    }
    void setUVoffset(float _u, float _v){
        this->u_off = _u;
        this->v_off = _v;
    }
    void setRefIdx(float _rIdx){
        this->rIdx = _rIdx;
    }
    void setFuzzy(float _fuzzy){
        this->fuzzy = _fuzzy;
    }
    void print(){
        std::cout<<"====Material info===="<<std::endl;
        std::cout<<"AMBI : "<<this->ambient[0]<<", "<<this->ambient[1]<<", "<<this->ambient[2]<<", "<<this->ambient[3]<<", "<<std::endl;
        std::cout<<"DIFF : "<<this->diff[0]<<", "<<this->diff[1]<<", "<<this->diff[2]<<", "<<this->diff[3]<<", "<<std::endl;
        std::cout<<"SPEC : "<<this->spec[0]<<", "<<this->spec[1]<<", "<<this->spec[2]<<", "<<this->spec[3]<<", "<<std::endl;
        std::cout<<"SHININESS : "<<this->shininess[0]<<std::endl;
    }
};

typedef std::map<std::string, Material*> MTL;

#endif