#ifndef __OBJLOADER__
#define __OBJLOADER__
#define GLM_FORCE_SIMD_AVX2
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <glm/glm.hpp>
#include <GL/freeglut.h>

#include "Exception.hpp"
#include "util.hpp"
#include "material.hpp"

#include "Drawables.hpp"

const char* obj_v = "v";
const char* obj_vt = "vt";
const char* obj_vn = "vn";
const char* obj_o = "o";
const char* obj_g = "g";
const char* obj_s = "s";
const char* obj_f = "f";
const char* obj_mtllib = "mtllib";
const char* obj_usemtl = "usemtl";

typedef std::vector<glm::vec<3,unsigned int>> objFace;

class objSmooth{
public:
    int id;
    std::vector<objFace> faces;
};

class objGroup{
public:
    std::string matName;
    std::string name;
    std::vector<objFace> faces;
};

class objObject{
public:
    std::string name;
    std::vector<objGroup> groups;
};

class objModel{
private:
    void addVertices(std::vector<std::string>& tokens){
        if(tokens.size()==4){
            this->vertices.push_back(glm::vec4(
                std::stof(tokens[1]),
                std::stof(tokens[2]),
                std::stof(tokens[3]),
                1.0f
            ));
        }else if(tokens.size()==5){
            this->vertices.push_back(glm::vec4(
                std::stof(tokens[1]),
                std::stof(tokens[2]),
                std::stof(tokens[3]),
                std::stof(tokens[4])
            ));
        }else{
            throw new ERRWRNGOBJ();
        }
    }
    void addTexCoords(std::vector<std::string>& tokens){
        if(tokens.size()==3){
            this->texCoords.push_back(glm::vec3(
                std::stof(tokens[1]),
                std::stof(tokens[2]),
                0.0f
            ));
        }else if(tokens.size()==4){
            this->texCoords.push_back(glm::vec3(
                std::stof(tokens[1]),
                std::stof(tokens[2]),
                std::stof(tokens[3])
            ));
        }else{
            throw new ERRWRNGOBJ();
        }
    }
    void addVerNormal(std::vector<std::string>& tokens){
        if(tokens.size()==4){
            this->verNomals.push_back(glm::vec3(
                std::stof(tokens[1]),
                std::stof(tokens[2]),
                std::stof(tokens[3])
            ));
        }else{
            throw new ERRWRNGOBJ();
        }
    }
    glm::vec<3,unsigned int> parseFaceVertex(std::string token,bool isVt, bool isVn){
        glm::vec<3,unsigned int> vertex;
        if((!isVt) && (!isVn)) {
            vertex[0] = std::stoi(token);
            vertex[1] = -1;
            vertex[2] = -1;
        }else if(isVt && (!isVn)){
            auto slashIdx = token.find_first_of('/');
            auto strV = token.substr(0,slashIdx);
            auto strVt = token.substr(slashIdx+1,token.size()-slashIdx-1);
            vertex[0] = std::stoi(strV)-idxMode;
            vertex[1] = std::stoi(strVt)-idxMode;
            vertex[2] = -1;
        }else if((!isVt) && isVn){
            auto firstIdx = token.find_first_of('/');
            auto lastIdx = firstIdx+1;
            auto strV = token.substr(0,firstIdx);
            auto strVn = token.substr(lastIdx+1,token.size()-lastIdx-1);
            vertex[0] = std::stoi(strV)-idxMode;
            vertex[1] = -1;
            vertex[2] = std::stoi(strVn)-idxMode;
        }else{
            auto firstIdx = token.find_first_of('/');
            auto lastIdx = token.find_last_of('/');
            auto strV = token.substr(0,firstIdx);
            auto strVt = token.substr(firstIdx+1,lastIdx-firstIdx-1);
            auto strVn = token.substr(lastIdx+1,token.size()-lastIdx-1);
            vertex[0] = std::stoi(strV)-idxMode;
            vertex[1] = std::stoi(strVt)-idxMode;
            vertex[2] = std::stoi(strVn)-idxMode;
        }
        return vertex;
    }
    void addFace(std::vector<std::string>& tokens){
        auto firstSlash = tokens[1].find_first_of('/');
        auto lastSlash = tokens[1].find_last_of('/');
        bool isVt, isVn;
        if(firstSlash==std::string::npos) isVt = isVn = false;
        else if(firstSlash==lastSlash) isVt = (!(isVn = false));
        else if((firstSlash+1)==lastSlash) isVn = (!(isVt = false));
        else  isVt = isVn = true;
        if(tokens.size()==5){
            // 0 1 2 3 => 0 1 2 + 2 3 0
            auto v0 = this->parseFaceVertex(tokens[1],isVt,isVn);
            auto v1 = this->parseFaceVertex(tokens[2],isVt,isVn);
            auto v2 = this->parseFaceVertex(tokens[3],isVt,isVn);
            auto v3 = this->parseFaceVertex(tokens[4],isVt,isVn);
            objFace f0,f1;
            f0.clear();
            f0.push_back(v0); f0.push_back(v1); f0.push_back(v2);
            f1.clear();
            f1.push_back(v2); f1.push_back(v3); f1.push_back(v0);
            this->objs.back().groups.back().faces.push_back(f0);
            this->objs.back().groups.back().faces.push_back(f1);
        }else if(tokens.size()==4){
            // 0 1 2 => 0 1 2
            auto v0 = this->parseFaceVertex(tokens[1],isVt,isVn);
            auto v1 = this->parseFaceVertex(tokens[2],isVt,isVn);
            auto v2 = this->parseFaceVertex(tokens[3],isVt,isVn);
            objFace f0;
            f0.clear();
            f0.push_back(v0); f0.push_back(v1); f0.push_back(v2);
            this->objs.back().groups.back().faces.push_back(f0);
        }else if(tokens.size() > 5){
            auto v0 = this->parseFaceVertex(tokens[1],isVt,isVn);
            auto prev = glm::uvec3();
            auto next = this->parseFaceVertex(tokens[2],isVt,isVn);
            auto N = tokens.size()-1;
            for(int i=3;i<=N;i++){
                prev = next;
                next = this->parseFaceVertex(tokens[i],isVt,isVn);
                objFace f;
                f.clear();
                f.push_back(v0); f.push_back(prev); f.push_back(next);
            }
        }else{
            throw new ERRWRNGOBJ();
        }
    }
public:
    std::vector<objObject> objs;
    std::vector<glm::vec4> vertices;
    std::vector<glm::vec3> texCoords;
    std::vector<glm::vec3> verNomals;
    MTL mtl;
    int idxMode;
    void mtlLoader(const char* path){
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
            //std::cout<<"PROCESSING : "<<trim(std::string(buf))<<std::endl;
            if(tokens[0].compare("newmtl")==0){
                if(isStarted){ 
                    std::cout<<"MAT NAME : "<<name<<std::endl;
                    m->print();
                    this->mtl[name] = m; name = tokens[1];
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
        if(isStarted) this->mtl[name] = m;
    }
    objModel(){

    }
    objModel(const char* path){
        char buf[2048];
        unsigned int sizeBuf = sizeof(buf);

        this->objs.clear();
        this->vertices.clear();
        this->texCoords.clear();
        this->verNomals.clear();
        mtl.clear();

        this->idxMode = 1;

        std::ifstream fin(path);
        std::vector<std::string> tokens;
        int faceCount = 0;
        int vertCount = 0;
        for(fin.getline(buf,sizeBuf);!fin.eof();fin.getline(buf,sizeBuf)){
            tokenizer(trim(std::string(buf)),tokens);
            if(tokens[0].compare("")==0 || tokens[0].at(0)=='#') continue;
            // LINE PARSING
            if(tokens[0].compare(obj_v)==0){
                this->addVertices(tokens);
                vertCount++;
            }else if(tokens[0].compare(obj_vt)==0){
                this->addTexCoords(tokens);
            }else if(tokens[0].compare(obj_vn)==0){
                this->addVerNormal(tokens);
            }else if(tokens[0].compare(obj_o)==0){
                objObject obj;
                obj.name = tokens[1];
                this->objs.push_back(obj);
            }else if(tokens[0].compare(obj_g)==0){
                objGroup group;
                group.name = tokens[1];
                this->objs.back().groups.push_back(group);                
            }else if(tokens[0].compare(obj_s)==0){
                // not implemented
            }else if(tokens[0].compare(obj_f)==0){
                if(this->objs.size()==0){
                    objObject obj;
                    obj.name = std::string("default");
                    this->objs.push_back(obj);
                }
                if(this->objs.back().groups.size()==0){
                    objGroup group;
                    group.name = std::string("default");
                    this->objs.back().groups.push_back(group);
                }
                this->addFace(tokens);
                faceCount++;
            }else if(tokens[0].compare(obj_mtllib)==0){
                this->mtlLoader(tokens[1].c_str());
            }else if(tokens[0].compare(obj_usemtl)==0){
                if(this->objs.size()==0){
                    objObject obj;
                    obj.name = std::string("default");
                    this->objs.push_back(obj);
                }
                if(this->objs.back().groups.size()==0){
                    objGroup group;
                    group.name = std::string("default");
                    this->objs.back().groups.push_back(group);
                }
                objGroup* g = &this->objs.back().groups.back();
                std::cout<<"Load material : "<<trim(tokens[1])<<std::endl;
                g->matName =trim(tokens[1]);
            }
        }
        std::cout<<"Vertices : "<<vertCount<<std::endl;
        std::cout<<"Face : "<<faceCount<<std::endl;
    }
    void translate(glm::mat4x4 tmat, glm::mat4x4 rmat, glm::mat4x4 smat){
        auto mat = tmat * rmat * smat;
        auto verIterEnd = this->vertices.end();
        for(auto iter = this->vertices.begin(); iter != verIterEnd; iter++){
            auto vertex = (*iter);
            vertex = mat * vertex;
            (*iter)[0] = vertex[0];(*iter)[1] = vertex[1];(*iter)[2] = vertex[2];(*iter)[3] = vertex[3];
        }
        auto normIterEnd = this->verNomals.end();
        for(auto iter = this->verNomals.begin(); iter != normIterEnd; iter++){
            auto vertex = glm::vec4((*iter)[0],(*iter)[1],(*iter)[2],0.0f);
            vertex = mat * vertex;
            vertex = glm::normalize(vertex);
            (*iter)[0] = vertex[0];(*iter)[1] = vertex[1];(*iter)[2] = vertex[2];
        }
    }

    void draw(){
        auto objIterEnd = this->objs.end();
        for(auto objIter = this->objs.begin(); objIter != objIterEnd; objIter++){
            auto groupIterEnd = (*objIter).groups.end();
            for(auto groupIter = (*objIter).groups.begin(); groupIter != groupIterEnd; groupIter++){
                //std::cout<<"Load mat : "<<(*groupIter).matName<<std::endl;
                this->mtl[(*groupIter).matName]->LoadMateiral();
                //this->mtl[(*groupIter).matName]->print();
                auto faceIterEnd = (*groupIter).faces.end();
                for(auto faceIter = (*groupIter).faces.begin(); faceIter != faceIterEnd; faceIter++){
                    glBegin(GL_POLYGON);
                    auto pos = this->vertices[(*faceIter)[0][0]];
                    auto norm = this->verNomals[(*faceIter)[0][2]];
                    glNormal3f(norm[0],norm[1],norm[2]);
                    glVertex4f(pos[0],pos[1],pos[2],pos[3]);
                    pos = this->vertices[(*faceIter)[1][0]];
                    norm = this->verNomals[(*faceIter)[1][2]];
                    glNormal3f(norm[0],norm[1],norm[2]);
                    glVertex4f(pos[0],pos[1],pos[2],pos[3]);
                    pos = this->vertices[(*faceIter)[2][0]];
                    norm = this->verNomals[(*faceIter)[2][2]];
                    glNormal3f(norm[0],norm[1],norm[2]);
                    glVertex4f(pos[0],pos[1],pos[2],pos[3]);
                    glEnd();
                }
            }
        }
    }
    void convertTriangles(std::vector<Hittable*>* hittables){
        auto objIterEnd = this->objs.end();
        for(auto objIter = this->objs.begin(); objIter != objIterEnd; objIter++){
            auto groupIterEnd = (*objIter).groups.end();
            for(auto groupIter = (*objIter).groups.begin(); groupIter != groupIterEnd; groupIter++){
                //std::cout<<"Load mat : "<<(*groupIter).matName<<std::endl;
                //this->mtl[(*groupIter).matName]->print();
                auto faceIterEnd = (*groupIter).faces.end();
                for(auto faceIter = (*groupIter).faces.begin(); faceIter != faceIterEnd; faceIter++){
                    auto pos1 = glm::vec3(this->vertices[(*faceIter)[0][0]]);
                    auto texcrd1 = this->texCoords[(*faceIter)[0][1]];
                    auto norm1 = this->verNomals[(*faceIter)[0][2]];
                    auto pos2 = glm::vec3(this->vertices[(*faceIter)[1][0]]);
                    auto texcrd2 = this->texCoords[(*faceIter)[1][1]];
                    auto norm2 = this->verNomals[(*faceIter)[1][2]];
                    auto pos3 = glm::vec3(this->vertices[(*faceIter)[2][0]]);
                    auto texcrd3 = this->texCoords[(*faceIter)[2][1]];
                    auto norm3 = this->verNomals[(*faceIter)[2][2]];
                    
                    auto mat = this->mtl[(*groupIter).matName];
                    Triangle* t = new Triangle(
                        pos1, pos2, pos3,
                        norm1, norm2, norm3,
                        texcrd1,texcrd2,texcrd3,
                        // 0 for bidirectional for now.
                        //ks, kt, refIdx, fuzzySpec,
                        mat->Kr,mat->Kt,
                        this->mtl[(*groupIter).matName]
                    );
                    t->refractIdx = 1.4;
                    t->spec = glm::vec3(mat->spec[0],mat->spec[1],mat->spec[2]);
                    t->diff = glm::vec3(mat->diff[0],mat->diff[1],mat->diff[2]);
                    hittables->push_back(t);
                }
            }
        }
    }
};

#endif