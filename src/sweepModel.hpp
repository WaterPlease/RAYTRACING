#ifndef __SWEEPMODEL__
#define __SWEEPMODEL__
#define GLM_FORCE_SIMD_AVX2
#include <vector>
#include <GL/glut.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <iostream>

#include "parameters.hpp"
#include "plane.hpp"
#include "spline.hpp"
#include "qSpline.hpp"
#include "Parser.hpp"
#include "objLoader.hpp"

class sweepModel:public objModel{
    std::vector<plane> planes;
    std::vector<plane> interpoledPlanes;
    std::string mtlPath;
public:
    sweepModel(const char* path, const char* _mtlPath): objModel(), mtlPath(_mtlPath){
        this->idxMode = 0;
        Parser parser;
        parser.parse(path,this->planes);
        for(auto iter = this->planes.begin(); iter != this->planes.end(); iter++){
            (*iter).toggleInterpol();
        }
        planeSpline(this->planes,numOnmodelSegment,this->interpoledPlanes);
        for(auto iter = this->interpoledPlanes.begin(); iter != this->interpoledPlanes.end(); iter++){
            (*iter).toggleInterpol();
        }
        this->bakePolygons();
    }

    void bakePolygons(){
        objObject obj;
        obj.name = std::string("default");
        this->objs.push_back(obj);
        objGroup group;
        group.name = std::string("default");
        group.matName = std::string("swept");
        this->objs.back().groups.push_back(group);
        this->mtlLoader(this->mtlPath.c_str());

        this->vertices.clear();
        this->texCoords.clear();
        this->verNomals.clear();
        for(auto iter = this->interpoledPlanes.begin(); iter != this->interpoledPlanes.end(); iter++){
            auto plane = *iter;
            for(auto cpIter = plane.bufPoints.begin(); cpIter != plane.bufPoints.end(); cpIter++){
                this->vertices.push_back(glm::vec4((*cpIter),1.0f));
                this->texCoords.push_back(glm::vec3(0.0,0.0,0.0));
                this->verNomals.push_back(glm::vec3(0.0,0.0,0.0));
            }
            // face generation
        }
        auto& faces = this->objs.back().groups.back().faces;
        //int numCont = this->numCP;
        int numCont = this->interpoledPlanes.back().bufPoints.size();
        for(int pNum = 0; pNum < (this->interpoledPlanes.size()-1); pNum++){
            for(int i=0; i<=(numCont-1); i++){
                objFace face;
                int v1,v2,v3,v4,v5;
                auto vIdx = (pNum+1)*numCont+i;
                v1 = vIdx;
                glm::vec<3,unsigned int> vIdxes1(vIdx,vIdx,vIdx);
                face.push_back(vIdxes1);
                vIdx = (pNum)*numCont+i;
                v2 = vIdx;
                glm::vec<3,unsigned int> vIdxes2(vIdx,vIdx,vIdx);
                face.push_back(vIdxes2);
                vIdx = (pNum)*numCont+(i+1)%numCont;
                v3 = vIdx;
                glm::vec<3,unsigned int> vIdxes3(vIdx,vIdx,vIdx);
                face.push_back(vIdxes3);
                faces.push_back(face);
                face = objFace();
                face.push_back(vIdxes3);
                vIdx = (pNum+1)*numCont+(i+1)%numCont;
                v4 = vIdx;
                glm::vec<3,unsigned int> vIdxes4(vIdx,vIdx,vIdx);
                face.push_back(vIdxes4);
                vIdx = (pNum+1)*numCont+i;
                v5 = vIdx;
                glm::vec<3,unsigned int> vIdxes5(vIdx,vIdx,vIdx);
                face.push_back(vIdxes5);
                faces.push_back(face);
                auto n1 = this->findNormalVec(v1,v2,v3);
                auto n2 = this->findNormalVec(v3,v4,v5);
                this->verNomals[v1] += n1;
                this->verNomals[v2] += n1;
                this->verNomals[v3] += n1;
                this->verNomals[v3] += n2;
                this->verNomals[v4] += n2;
                this->verNomals[v5] += n2;
                //std::cout<<"face : "<<v1<<", "<<v2<<", "<<v3<<std::endl;
                //std::cout<<"face : "<<v3<<", "<<v4<<", "<<v5<<std::endl;
            }
        }
        for(int i = 0; i < this->verNomals.size(); i++){
            this->verNomals[i] = glm::normalize(this->verNomals[i]);
        }
    }
    /*
    void draw(){
        _draw(this->interpoledPlanes);
    }
    */
    void _draw(std::vector<plane>& visiblePlane){
        auto curPlane = visiblePlane.begin();
        auto numSize = (*curPlane).bufPoints.size();
        auto endPointIter = (visiblePlane.end()-1);
        auto count = 0;
        while(curPlane != endPointIter){
            auto curPoint = (*curPlane).bufPoints.begin();
            auto nextPoint = (*(curPlane+1)).bufPoints.begin();
            auto endPointsIter = (*curPlane).bufPoints.end();
            //glBegin(GL_QUAD_STRIP);
            glBegin(GL_TRIANGLE_STRIP);
                auto cp = (*curPoint);
                auto np = (*nextPoint);
                for(;curPoint!=endPointsIter;curPoint++,nextPoint++){
                    cp = (*curPoint);
                    np = (*nextPoint);
                    glVertex3f(cp[0],cp[1],cp[2]);
                    glVertex3f(np[0],np[1],np[2]);
                }
                curPoint = (*curPlane).bufPoints.begin();
                nextPoint = (*(curPlane+1)).bufPoints.begin();
                cp = (*curPoint);
                np = (*nextPoint);
                glVertex3f(cp[0],cp[1],cp[2]);
                glVertex3f(np[0],np[1],np[2]);
            glEnd();
            count += 2*(*curPlane).bufPoints.size();
            curPlane++;
        }
        std::cout<<"drawn poly_tri count : "<<count<<std::endl;
    }
    glm::vec3 findNormalVec(int idx1, int idx2, int idx3){
        glm::vec3 v1 = this->vertices[idx1];
        glm::vec3 v2 = this->vertices[idx2];
        glm::vec3 v3 = this->vertices[idx3];
        auto a = v1 - v2;
        auto b = v3 - v2;
        return glm::normalize(glm::cross(b,a));
    }
};
#endif
