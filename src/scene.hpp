#ifndef __SCENE__
#define __SCENE__
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
#include "objLoader.hpp"
#include "sweepModel.hpp"
#include "light.hpp"
#include "implicit.hpp"
#include "Drawables.hpp"
//#include "KDTREE.hpp"
#include "BVH.hpp"

const char* scene_obj = "obj";
const char* scene_pos = "pos";
const char* scene_rot = "rot";
const char* scene_scale = "scale";
const char* scene_light = "light";
const char* scene_la = "La";
const char* scene_ld = "Ld";
const char* scene_ls = "Ls";
const char* scene_latt = "Latt";
const char* scene_lpos = "Lpos";
const char* scene_swept = "swept";
const char* scene_implicit = "implicit";
const char* scene_sphere = "sphere";

class Scene{
public:
    std::vector<objModel*> objs;
    std::vector<Light*> lights;
    std::vector<sweepModel*> swepts;
    std::vector<implicit*> implicits;
    // ray traced geometrics...
    std::vector<Hittable*> hittables;
    //kTree* tree;
    BVHTree* tree;
    Scene(const char* path){
        char buf[2048];
        unsigned int sizeBuf = sizeof(buf);

        glm::mat4x4 sMat = glm::scale(glm::identity<glm::mat4x4>(),glm::vec3(1.0f));
        glm::mat4x4 rMat = glm::toMat4(glm::angleAxis(0.0f,glm::vec3(0,0,1)));
        glm::mat4x4 tMat = glm::translate(glm::identity<glm::mat4x4>(),glm::vec3(0,0,0));

        std::ifstream fin(path);
        std::vector<std::string> tokens;

        int curLight = GL_LIGHT0;

        this->tree = new BVHTree();

        for(fin.getline(buf,sizeBuf);!fin.eof();fin.getline(buf,sizeBuf)){
            tokenizer(trim(std::string(buf)),tokens);
            std::cout<<"PROCESSING : "<<trim(std::string(buf))<<std::endl;
            if(tokens[0].compare("")==0 || tokens[0].at(0)=='#') continue;
            // LINE PARSING
            if(tokens[0].compare(scene_obj)==0){
                objModel* ptr_obj = new objModel(tokens[1].c_str());
                this->objs.push_back(ptr_obj);
                ptr_obj->translate(tMat, rMat, sMat);
            }else if(tokens[0].compare(scene_light)==0){
                Light* ptr_light = new Light(curLight);
                this->lights.push_back(ptr_light);
                curLight++;
                this->lights.back()->LoadLight();
            }else if(tokens[0].compare(scene_la)==0){
                float fv[4];
                fv[0] = std::stof(tokens[1]);
                fv[1] = std::stof(tokens[2]);
                fv[2] = std::stof(tokens[3]);
                fv[3] = 1.0f;
                this->lights.back()->setAmbient(fv);
                this->lights.back()->LoadLight();
            }else if(tokens[0].compare(scene_ld)==0){
                float fv[4];
                fv[0] = std::stof(tokens[1]);
                fv[1] = std::stof(tokens[2]);
                fv[2] = std::stof(tokens[3]);
                fv[3] = 1.0f;
                this->lights.back()->setDiffuse(fv);
                this->lights.back()->LoadLight();
            }else if(tokens[0].compare(scene_ls)==0){
                float fv[4];
                fv[0] = std::stof(tokens[1]);
                fv[1] = std::stof(tokens[2]);
                fv[2] = std::stof(tokens[3]);
                fv[3] = 1.0f;
                this->lights.back()->setSpecular(fv);
                this->lights.back()->LoadLight();
            }else if(tokens[0].compare(scene_lpos)==0){
                float fv[4];
                fv[0] = std::stof(tokens[1]);
                fv[1] = std::stof(tokens[2]);
                fv[2] = std::stof(tokens[3]);
                fv[3] = std::stof(tokens[4]);
                this->lights.back()->setPos(fv);
                this->lights.back()->LoadLight();
            }else if(tokens[0].compare(scene_latt)==0){
                float fv[3];
                fv[0] = std::stof(tokens[1]);
                fv[1] = std::stof(tokens[2]);
                fv[2] = std::stof(tokens[3]);
                this->lights.back()->setAttenuation(fv);
                this->lights.back()->LoadLight();
            }else if(tokens[0].compare(scene_pos)==0){
                tMat = glm::translate(glm::identity<glm::mat4x4>(),glm::vec3(
                    std::stof(tokens[1]),
                    std::stof(tokens[2]),
                    std::stof(tokens[3])
                ));
            }else if(tokens[0].compare(scene_rot)==0){
                rMat = glm::toMat4(glm::angleAxis(
                    std::stof(tokens[1]),glm::normalize(glm::vec3(
                        std::stof(tokens[2]),
                        std::stof(tokens[3]),
                        std::stof(tokens[4]))
                    )
                ));
            }else if(tokens[0].compare(scene_scale)==0){
                sMat = glm::scale(glm::identity<glm::mat4x4>(),glm::vec3(std::stof(tokens[1])));
            }else if(tokens[0].compare(scene_swept)==0){
                sweepModel* ptr_swept = new sweepModel(tokens[1].c_str(),tokens[2].c_str());
                ptr_swept->translate(tMat, rMat, sMat);
                this->swepts.push_back(ptr_swept);
            }else if(tokens[0].compare(scene_implicit)==0){
                std::cout<<"PROCESSING : "<<trim(std::string(buf))<<std::endl;
                implicit* implicitModel;
                if(tokens[1].compare("torus")==0) implicitModel = getTorusModel(tokens[2].c_str(), stof(tokens[3]));
                else if(tokens[1].compare("genus2")==0) implicitModel = getGenus2Model(tokens[2].c_str(), stof(tokens[3]));
                else if(tokens[1].compare("merging")==0) implicitModel = getMergingModel(tokens[2].c_str(), stof(tokens[3]));
                else{
                    throw new ERRWRNGIMPLICIT(tokens[1]);
                }
                implicitModel->translate(tMat, rMat, sMat);
                this->implicits.push_back(implicitModel);
            }else if(tokens[0].compare(scene_sphere)==0){
                MTL* mtl = new MTL();
                SphereLoadMTL(tokens[5].c_str(),mtl);
                Sphere* s = new Sphere(stof(tokens[1]),glm::vec3(
                    stof(tokens[2]),
                    stof(tokens[3]),
                    stof(tokens[4])),
                    mtl);
                s->hit_type = OBJ_SPHERE;
                this->hittables.push_back(s);
            }else if(tokens[0].compare("transball")==0){
            }else if(tokens[0].compare("sample")==0){
                sample_factor = stoi(tokens[1]);
            }else if(tokens[0].compare("soft_shadow")==0){
                soft_shadow = stof(tokens[1]);
            }else if(tokens[0].compare("sample_shadow")==0){
                sample_shadow = stoi(tokens[1]);
            }else if(tokens[0].compare("sample_spec")==0){
                sample_spec = stoi(tokens[1]);
            }else if(tokens[0].compare("thread")==0){
                max_thread = stoi(tokens[1]);
            }else if(tokens[0].compare("depth")==0){
                max_depth = stoi(tokens[1]);
            }else if(tokens[0].compare("sky")==0){
                color_sky = COLOR(
                    stof(tokens[1]),
                    stof(tokens[2]),
                    stof(tokens[3]));
            }else if(tokens[0].compare("KDdepth")==0){
                KD_MAX_Depth = stoi(tokens[1]);
            }else if(tokens[0].compare("KDtriangle")==0){
                KD_MAX_TRI = stoi(tokens[1]);
            }else if(tokens[0].compare("bKDtree")==0){
                bKDtree = (stoi(tokens[1])==1)? true:false;
            }
        }
        
        for(auto iter = this->objs.begin(); iter != this->objs.end();iter++)
            (*iter)->convertTriangles(&(this->hittables));
        for(auto iter = this->implicits.begin(); iter != this->implicits.end();iter++)
            (*iter)->convertTriangles(&(this->hittables));
        for(auto iter = this->swepts.begin(); iter != this->swepts.end();iter++)
            (*iter)->convertTriangles(&(this->hittables));
        printf("Tree generate start\n");
        printf("Depth : %d\nTri Limit : %d\n",KD_MAX_Depth,KD_MAX_TRI);
        //this->tree = new kTree(this->hittables,KD_MAX_Depth,KD_MAX_TRI);
        if(bKDtree){
            this->tree->initQueue(this->hittables);
            printf("Tree build start\n");
            //this->tree->buildTree();
            this->tree->buildTree();
            printf("Tree pruning\n");
            this->tree->pruneTree();
            printf("Tree build done\n");
        }
    }
    void draw(){
        glEnable(GL_LIGHTING);
        for(auto iter=this->lights.begin();iter!=this->lights.end();iter++){
            (*iter)->LoadLight();
        }
        if(bKDtree){
            this->tree->draw();
        }else{
            for(auto iter=this->hittables.begin(); iter != this->hittables.end(); iter++){
                auto ptr_drawable = (Hittable*)(*iter);
                if(ptr_drawable->hit_type == OBJ_SPHERE){
                    Sphere* ptr_sphere = (Sphere*)ptr_drawable;
                    ptr_sphere->draw();
                }else if(ptr_drawable->hit_type == OBJ_TRIANGLE){
                    Triangle* ptr_tri = (Triangle*)ptr_drawable;
                    ptr_tri->draw();
                }
            }
        }
        /*
        if(bWireMode){
            //this->tree->draw();
            this->tree->draw();
        }*/
    }
};

#endif