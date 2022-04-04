#ifndef __DRAWABLE__
#define __DRAWABLE__
#define GLM_FORCE_SIMD_AVX2
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/common.hpp>
#include <GL/freeglut.h>
#include <math.h>
#include <fstream>

#include "Ray.hpp"
#include "material.hpp"
//#include "objLoader.hpp"
#include "parameters.hpp"
#include "util.hpp"
#include "AABB.hpp"

/*enum OBJ_TYPE{
    SPHERE,
    TRIANGLE
};*/

const int OBJ_SPHERE = 1;
const int OBJ_TRIANGLE = 0;

enum MAT_TYPE{
    RGBCOLOR,
    TEXTURE
};

class Drawable{
public:
    MAT_TYPE mat_type;
    bool isBumped;
    float glossy = 51.2f;
    float ks = 1.0f;
    float kt = 1.0f;
    float fuzzySpec = 0.0f;
    float refractIdx;
    virtual void draw() = 0;
    virtual glm::vec3 getAmbientColor(const void* rec) = 0;
    virtual glm::vec3 getDiffuseColor(const void* rec) = 0;
    virtual glm::vec3 getSpecularColor(glm::vec3 p) = 0;
};

class HitRecord{
public:
    glm::vec3 p;
    glm::vec3 N;
    glm::vec2 texCoord;
    Drawable* ptr_hitObj;
    glm::vec3 dir;
    float u;
    float v;
    float t;
    bool front_face;

    inline void set_face_normal(const Ray& r, const glm::vec3& outward_normal) {
        front_face = glm::dot(r.dir, outward_normal) < 0;
        this->N = front_face ? outward_normal :-outward_normal;
    }
};

class Hittable{
public:
    int hit_type;
    virtual bool hit(const Ray& r, float t_min, float t_max, HitRecord& rec) = 0;
    virtual AABB bounding_box() = 0;
};

void SphereLoadMTL(const char* path,MTL* mtl){
    std::cout<<"Only last mat left"<<std::endl;
    char buf[2048];
    unsigned int sizeBuf = sizeof(buf);
    std::ifstream fin(path);
    std::vector<std::string> tokens;
    std::string name;
    bool isStarted = false;
    Material* m;
    for(fin.getline(buf,sizeBuf);!fin.eof();fin.getline(buf,sizeBuf)){
        tokenizer(trim(std::string(buf)),tokens);
        if(tokens[0].compare("")==0 || tokens[0].at(0)=='#') continue;
        // LINE PARSING
        std::cout<<"PROCESSING : "<<trim(std::string(buf))<<std::endl;
        if(tokens[0].compare("newmtl")==0){
            if(isStarted){ 
                std::cout<<"MAT NAME : "<<name<<std::endl;
                m->print();
                mtl->insert({name,m});; name = tokens[1];
                m = new Material();
            }
            else { isStarted = true; name = tokens[1]; m = new Material();}
        }else if(tokens[0].compare("Ns")==0){
            m->setShininess(std::stof(tokens[1]));
        }else if(tokens[0].compare("Ka")==0){
            float fv[3];
            fv[0] = std::stof(tokens[1]);
            fv[1] = std::stof(tokens[2]);
            fv[2] = std::stof(tokens[3]);
            m->setAmbient(fv);
            std::cout<<"Ka : "<<fv[0]<<", "<<fv[1]<<", "<<fv[2]<<std::endl;
        }else if(tokens[0].compare("Kd")==0){
            float fv[3];
            fv[0] = std::stof(tokens[1]);
            fv[1] = std::stof(tokens[2]);
            fv[2] = std::stof(tokens[3]);
            m->setDiffuse(fv);          
            std::cout<<"Kd : "<<fv[0]<<", "<<fv[1]<<", "<<fv[2]<<std::endl;
        }else if(tokens[0].compare("Ks")==0){
            float fv[3];
            fv[0] = std::stof(tokens[1]);
            fv[1] = std::stof(tokens[2]);
            fv[2] = std::stof(tokens[3]);
            m->setSpecular(fv);          
            std::cout<<"Ks : "<<fv[0]<<", "<<fv[1]<<", "<<fv[2]<<std::endl;      
        }else if(tokens[0].compare("Kr")==0){
            auto t = std::stof(tokens[1]);
            m->setReflection(t);
        }else if(tokens[0].compare("Tr")==0){
            auto t = std::stof(tokens[1]);
            m->setRefraction(t);
        }else if(tokens[0].compare("map_Kd")==0){
            auto idx = textures.size();
            textures.push_back(Texture(
                tokens[1].c_str()
            ));
            m->setTexture(idx);
            std::cout<<"new Texture"<<std::endl;
        }else if(tokens[0].compare("uv")==0){
            auto u = std::stof(tokens[1]);
            auto v = std::stof(tokens[2]);
            m->setUVoffset(u,v);
        }else if(tokens[0].compare("rIdx")==0){
            m->setRefIdx(std::stof(tokens[1]));
        }else if(tokens[0].compare("fuzzy")==0){
            m->setFuzzy(std::stof(tokens[1]));
        }
    }
        if(isStarted) mtl->insert({name,m});
}

class Sphere : public Hittable, public Drawable{
private:
    GLUquadric *quad;
public :
    float r;
    glm::vec3 center;
    Material* mat;
    COLOR ambi;
    COLOR diff;
    COLOR spec;
    MTL* mtl;
    Sphere(float _r, glm::vec3 _c,MTL* _mtl):r(_r),center(_c)
    {
        this->mat_type = MAT_TYPE::RGBCOLOR;
        this->mtl = _mtl;
        this->mat = _mtl->at(std::string("sphere"));
        if(this->mat->tid == -1){
            this->mat_type = MAT_TYPE::RGBCOLOR;
            printf("SPHERE NO TEXTURE\n");
        }
        else
            this->mat_type = MAT_TYPE::TEXTURE;
        this->ks = this->mat->Kr;
        this->kt = this->mat->Kt;
        this->ambi = COLOR(this->mat->ambient[0],this->mat->ambient[1],this->mat->ambient[2]);
        this->diff = COLOR(this->mat->diff[0],this->mat->diff[1],this->mat->diff[2]);
        this->spec = COLOR(this->mat->spec[0],this->mat->spec[1],this->mat->spec[2]);
        this->hit_type = OBJ_SPHERE;
        this->refractIdx = this->mat->rIdx;
        this->fuzzySpec = this->mat->fuzzy;
        this->isBumped = false;
        this->quad = gluNewQuadric();
    }
    bool hit(const Ray& r, float t_min, float t_max, HitRecord& rec){
        auto a = glm::dot(r.dir,r.dir);
        auto b = glm::dot(r.dir,r.ori - this->center);// b/2
        auto c = glm::dot(r.ori - this->center,r.ori - this->center) - this->r*this->r;
        auto D = b*b-a*c;
        if(D < 0){
            return false;
        }

        auto sqrtD = glm::sqrt(D);
        auto t = (-b-sqrtD)/a;
        if(t < t_min || t > t_max){
            t = (-b+sqrtD)/a;
            if(t < t_min || t > t_max){
                return false;
            }
        }

        rec.t = t;
        rec.p = r.at(t);
        auto outward = glm::normalize(rec.p - this->center);
        rec.set_face_normal(r,outward);
        rec.ptr_hitObj = this;
        rec.dir = r.dir;
        
        return true;
    }
    glm::vec3 getAmbientColor(const void* rec){
        return this->ambi;
    }
    glm::vec3 getDiffuseColor(const void* rec){
        if(this->mat_type==MAT_TYPE::RGBCOLOR)
            return this->diff;
        else{
            auto p = ((HitRecord*)rec)->p;
            auto pi = glm::pi<float>();
            auto d = (this->center - p);
            d = glm::normalize(d);
            float u,v;
            u = 0.5f + atan2f(d[0],d[2])/(2.0f * pi);
            v = 0.5f + asinf(d[1])/pi;
            return textures[this->mat->tid].at(u + this->mat->u_off,-(v + this->mat->v_off));
        }
    }
    glm::vec3 getSpecularColor(glm::vec3 p){
        if(this->mat_type==MAT_TYPE::RGBCOLOR)
            return this->spec;
        else
            return this->spec;
    }
    void draw(){
        float d[4] = {this->mat->spec[0],this->mat->spec[1],this->mat->spec[2],1.0f};
        float s[4] = {this->mat->spec[0],this->mat->spec[1],this->mat->spec[2],1.0f};
        glPushMatrix();
        glLoadIdentity();
        if(this->mat_type==MAT_TYPE::TEXTURE){
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, textures[this->mat->tid].glTextureID);
        }
            this->mat->LoadMateiral();
            glTranslatef(this->center[0],this->center[1],this->center[2]);
            glScalef(-1.0,1.0,1.0);
            glRotatef(90.0,1,0,0);
            gluQuadricTexture(this->quad,1);
            gluSphere(this->quad,this->r,20,20);
        if(this->mat_type==MAT_TYPE::TEXTURE){
            glDisable(GL_TEXTURE_2D);
        }
        glPopMatrix();
    }
    AABB bounding_box(){
        return AABB(
            this->center - glm::vec3(this->r),
            this->center + glm::vec3(this->r)
        );
    }
};

class Triangle : public Hittable, public Drawable{
public :
    glm::vec3 v0,v1,v2;
    glm::vec3 vt0, vt1, vt2;
    glm::vec3 vn0, vn1, vn2;
    glm::vec3 N;
    float doubleDrea;
    COLOR ambi;
    COLOR diff;
    COLOR spec;
    Material* mat;
    Triangle(glm::vec3 _v0,glm::vec3 _v1,glm::vec3 _v2,
        glm::vec3 _vn0, glm::vec3 _vn1, glm::vec3 _vn2,
        glm::vec3 _vt0, glm::vec3 _vt1, glm::vec3 _vt2,
        float _ks, float _kt,Material* _mat)
        :v0(_v0),v1(_v1),v2(_v2),
         vn0(_vn0),vn1(_vn1),vn2(_vn2),
         vt0(_vt0),vt1(_vt1),vt2(_vt2)
    {
        this->N = glm::cross(v1-v0,v2-v0);
        this->doubleDrea = glm::length(this->N);
        this->N = glm::normalize(this->N);
        this->ks = _ks;
        this->kt = _kt;
        this->mat = _mat;
        if(this->mat->tid == -1)
            this->mat_type = MAT_TYPE::RGBCOLOR;
        else
            this->mat_type = MAT_TYPE::TEXTURE;
        this->hit_type = OBJ_TRIANGLE;
        this->ambi = COLOR(this->mat->ambient[0],this->mat->ambient[1],this->mat->ambient[2]);
        this->diff = COLOR(this->mat->diff[0],this->mat->diff[1],this->mat->diff[2]);
        this->spec = COLOR(this->mat->spec[0],this->mat->spec[1],this->mat->spec[2]);
        this->refractIdx = this->mat->rIdx;
        this->fuzzySpec = this->mat->fuzzy;
    }
    bool pointOnPlane(const Ray& r, float& t){
        auto dirN = glm::dot(r.dir,this->N);
        if(glm::abs(dirN) < eps) return false;
        t = glm::dot(this->v0-r.ori,this->N)/dirN;
        return true;
    }
    bool hit(const Ray& r, float t_min, float t_max, HitRecord& rec){
        float t;
        auto isIntersect = this->pointOnPlane(r,t);
        if(!isIntersect || t < t_min || t > t_max) return false;
        if(this->doubleDrea < 1e-6) return false;
        auto P = rec.p = r.at(t);
        
        auto e0 =  this->v1 - this->v0;
        auto vp0 = P - this->v0;
        auto C = glm::cross(e0,vp0);
        if(glm::dot(this->N,C) < 0) return false;

        auto e1 =  this->v2 - this->v1;
        auto vp1 = P - this->v1;
        C = glm::cross(e1,vp1);
        if(glm::dot(this->N,C) < 0) return false;
        auto u = glm::length(C)/this->doubleDrea;

        auto e2 =  this->v0 - this->v2;
        auto vp2 = P - this->v2;
        C = glm::cross(e2,vp2);
        if(glm::dot(this->N,C) < 0) return false;
        auto v = glm::length(C)/this->doubleDrea;
        
        auto w = (1-u-v);
        auto outward = glm::normalize(u*this->vn0 + v*this->vn1 + w*this->vn2);
        rec.set_face_normal(r,outward);
        rec.t = t;
        rec.ptr_hitObj = this;
        rec.dir = r.dir;
        rec.u = u;
        rec.v = v;
        return true;
    }
    glm::vec3 getAmbientColor(const void* rec){
        return this->ambi;
    }
    glm::vec3 getDiffuseColor(const void* rec){
        if(this->mat_type==MAT_TYPE::RGBCOLOR)
            return this->diff;
        else{
            //textures[this->mat->tid].
            float u = ((HitRecord*)rec)->u;
            float v = ((HitRecord*)rec)->v;
            auto w = (1-u-v);
            auto center = u*this->vt0 + v*this->vt1 + w*this->vt2;
            return textures[this->mat->tid].at(center[0] + this->mat->u_off,center[1] + this->mat->v_off);
        }
    }
    glm::vec3 getSpecularColor(glm::vec3 p){
        return this->spec;
    }
    void draw(){
        float d[4] = {this->diff[0],this->diff[1],this->diff[2],1.0f};
        float s[4] = {this->spec[0],this->spec[1],this->spec[2],1.0f};
        glPushMatrix();
        glLoadIdentity();
        this->mat->LoadMateiral();
        if(this->mat_type==MAT_TYPE::TEXTURE){
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, textures[this->mat->tid].glTextureID);
        }
        glBegin(GL_TRIANGLES);
            glTexCoord2f((this->vt0[0]+this->mat->u_off), -(this->vt0[1]+this->mat->v_off));
            glNormal3f(this->vn0[0],this->vn0[1],this->vn0[2]);
            glVertex3f(this->v0[0],this->v0[1],this->v0[2]);glTexCoord2f((this->vt1[0]+this->mat->u_off), -(this->vt1[1]+this->mat->v_off));
            glNormal3f(this->vn1[0],this->vn1[1],this->vn1[2]);
            glVertex3f(this->v1[0],this->v1[1],this->v1[2]);glTexCoord2f((this->vt2[0]+this->mat->u_off), -(this->vt2[1]+this->mat->v_off));
            glNormal3f(this->vn2[0],this->vn2[1],this->vn2[2]);
            glVertex3f(this->v2[0],this->v2[1],this->v2[2]);
        glEnd();
        if(this->mat_type==MAT_TYPE::TEXTURE){
            glDisable(GL_TEXTURE_2D);
        }
        glPopMatrix();
    }
    AABB bounding_box() {
        glm::vec3 min,max;
        min = glm::min(glm::min(this->v0,this->v1),this->v2);
        max = glm::max(glm::max(this->v0,this->v1),this->v2);
        return AABB(
            min,
            max
        );
    }
};

#endif