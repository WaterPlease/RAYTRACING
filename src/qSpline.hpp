#ifndef __QSPLINE__
#define __QSPLINE__
#define GLM_FORCE_SIMD_AVX2
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <iostream>

#include "spline.hpp"

class QSpline : public ISpline<glm::fquat>{
public:
    QSpline(std::vector<glm::fquat> _points):ISpline<glm::fquat>(){
        this->points = _points;
        this->pointConversion();
    }
    void pointConversion(){
        this->cPoints.clear();
        auto N = this->points.size();
        std::vector<glm::fquat> dPoints;
        dPoints.push_back(glm::fquat());
        for(int i=1;i<N-1;i++){
            dPoints.push_back(glm::exp(
                1.0f/6.0f*glm::log(glm::conjugate(this->points[i-1])*this->points[i+1])
                )
            );
        }
        this->cPoints.push_back(this->points[0]);
        this->cPoints.push_back(this->points[0]);
        //this->cPoints.push_back(glm::slerp(this->cPoints[0],this->cPoints[1],0.5f));
        for(int i=1; i<N-1;i++){
            this->cPoints.push_back(this->points[i]
                *dPoints[i]);
            this->cPoints.push_back(this->points[i]);
            this->cPoints.push_back(this->points[i]);
            this->cPoints.push_back(this->points[i]
                *glm::conjugate(dPoints[i]));
        }
        //this->cPoints.push_back(glm::slerp(this->cPoints[N-2],this->points.back(),0.5f));
        this->cPoints.push_back(this->points.back());
        this->cPoints.push_back(this->points.back());
    }
    void interpol(int N){
        auto dt = 1.0f/(float)N;
        float t;
        this->points.clear();
        int M = this->cPoints.size();
        for(int i=0;i<M;i+=4){
            for(int j=0;j<N;j++){
                    t = dt*j;
                    auto q1 = glm::slerp(this->cPoints[i],this->cPoints[i+1],t);
                    auto q2 = glm::slerp(this->cPoints[i+1],this->cPoints[i+2],t);
                    auto q3 = glm::slerp(this->cPoints[i+2],this->cPoints[i+3],t);
                    auto q4 = glm::slerp(q1,q2,t);
                    auto q5 = glm::slerp(q2,q3,t);
                    auto q6 = glm::slerp(q4,q5,t);
                    this->points.push_back(q6);
            }
        }
        this->points.push_back(this->cPoints.back());
    }
    glm::fquat pow(glm::fquat q, float t){
        return glm::exp(t*glm::log(q));
    }
};
#endif