#ifndef __PARSER__
#define __PARSER__
#define GLM_FORCE_SIMD_AVX2
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "spline.hpp"
#include "Exception.hpp"
#include "util.hpp"
#include "plane.hpp"

enum STATE{
    DONE,
    CURVE,
    CROSS,
    CONTROL,
    READDATA
};

class Parser{
public:
    int state;
    SPLINETYPE curveType;
    int numCRS;
    int numCP;
    std::vector<glm::vec2> points;
    std::vector<float> scalings;
    std::vector<glm::vec4> rots;
    std::vector<glm::vec3> pos;

    int currLineNum;
    Parser(){
        this->state = STATE::DONE;
    }
    void parse(const char* path,std::vector<plane>& planes){
        _parse(path);
        planes.clear();
        for(int i=0; i<this->numCRS; i++){
            std::vector<glm::vec2> _planePoints = {this->points.begin()+i*(this->numCP),this->points.begin()+(i+1)*(this->numCP)};
            std::vector<glm::vec3> planePoints;
            for(int j=0; j<this->numCP; j++){
                planePoints.push_back(glm::vec3(_planePoints[j][0],0.0f,_planePoints[j][1]));
            }
            glm::fquat q = glm::angleAxis<float,glm::packed_highp>(glm::degrees(rots[i][0]),glm::vec<3,float,glm::packed_highp>(rots[i][1],rots[i][2],rots[i][3]));
            planes.push_back(plane(this->curveType,planePoints,this->pos[i],glm::vec1(this->scalings[i]),q));
        }
    }
    int _parse(const char* path){
        this->currLineNum = -1;
        this->points.clear();
        this->scalings.clear();
        this->rots.clear();
        this->pos.clear();
        try{
            this->read(path);
        }catch(ERRNOFILE e){
            std::cout<<"ERROR["<<this->currLineNum<<"] : "<<e.what()<<std::endl;
            return -1;
        }catch(ERRPARSE e){
            std::cout<<"ERROR["<<this->currLineNum<<"] : "<<e.what()<<std::endl;
            return -1;
        }
        return 1;
    }
    void read(const char* path){
        std::ifstream fin(path);
        char _buf[64];
        if(fin.fail()){
            throw ERRNOFILE();
        }
        this->state = STATE::CURVE;
        int curCP = 0;
        int curCRS = 0;
        while(this->state != STATE::DONE){
            this->currLineNum++;
            fin.getline(_buf,sizeof(_buf));
            std::string buf(_buf);
            buf = removeComment(trim(buf));
            if(buf.length()==0) continue;
            if(this->state==STATE::CURVE){
                std::string& curveName = buf;
                if(curveName.compare("BSPLINE")==0) this->curveType = SPLINETYPE::BSPLINE;
                else if(curveName.compare("CATMULL_ROM")==0) this->curveType = SPLINETYPE::CATMULL_ROM;
                else throw ERRNOCURVE();
                this->state = STATE::CROSS;
            }else if(this->state==STATE::CROSS){
                this->numCRS = parseInt(buf);
                this->state = STATE::CONTROL;
            }else if(this->state==STATE::CONTROL){
                this->numCP = parseInt(buf);
                this->state = STATE::READDATA;
            }else if(this->state==STATE::READDATA){
                if(curCP < this->numCP){
                    this->points.push_back(parseVec<2>(buf));
                    curCP++;
                }else if(curCP==this->numCP){
                    this->scalings.push_back(parseFloat(buf));
                    curCP++;
                }else if(curCP==(this->numCP+1)){
                    this->rots.push_back(parseVec<4>(buf));
                    curCP++;
                }else if(curCP==(this->numCP+2)){
                    this->pos.push_back(parseVec<3>(buf));
                    curCP = 0;
                    curCRS++;
                    if(curCRS==this->numCRS) this->state=STATE::DONE;
                }
            }
        }
        fin.close();
    }
};
#endif