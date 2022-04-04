#ifndef __PLANE__
#define __PLANE__
#define GLM_FORCE_SIMD_AVX2
#include <vector>
#include <glm/glm.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <GL/glut.h>
#include <iostream>

#include "parameters.hpp"
#include "spline.hpp"
#include "qSpline.hpp"

class plane{
public:
    glm::vec3 pos;
    glm::vec1 scale;
    glm::fquat rot;
    glm::mat4x4 mMat;
    std::vector<glm::vec3> points;
    std::vector<glm::vec3> splineP;
    std::vector<glm::vec3> splineCP;
    std::vector<glm::vec3> bufPoints;
    SPLINETYPE curveType;
    int visibleCurve;
    plane(SPLINETYPE _curveType,const std::vector<glm::vec3>& _points,const glm::vec3& _pos,const glm::vec1& _scale,const glm::fquat& _rot):curveType(_curveType),pos(_pos),scale(_scale),rot(_rot){
        this->points = std::vector<glm::vec3>(_points);
        visibleCurve= -1;
        if(curveType==SPLINETYPE::CATMULL_ROM){
            CATMULL_ROM_SPLINE<glm::vec3> spline((this->points));
            spline.interpol(numOnplaneSegment);
            this->splineP = spline.points;
            this->splineCP = spline.cPoints;
        }else{
            BSpline<glm::vec3> spline((this->points));
            spline.interpol(numOnplaneSegment);
            this->splineP = spline.points;
            this->splineCP = spline.cPoints;
        }

        glm::mat4x4 sMat = glm::scale(glm::identity<glm::mat4x4>(),glm::vec3(_scale[0]));
        glm::mat4x4 rMat = glm::toMat4(_rot);
        glm::mat4x4 tMat = glm::translate(glm::identity<glm::mat4x4>(),pos);
        this->mMat = tMat * rMat * sMat;
        this->toggleInterpol();
    }
    void draw(){
        glBegin(GL_LINE_LOOP);
        for(auto iter = this->bufPoints.begin(); iter != this->bufPoints.end(); iter++){
            glVertex3f((*iter)[0],(*iter)[1],(*iter)[2]);
        }
        glEnd();
    }
    void toggleInterpol(){
        visibleCurve++;
        visibleCurve %= 2;
        switch(visibleCurve){
            case 0:
                this->transform(this->points);
                break;
            case 1:
            /*  this->transform(this->splineCP);
                break;
            case 2:*/
                this->transform(this->splineP);
                break;
        }
    }
    void transform(std::vector<glm::vec3>& points){
        this->bufPoints.clear();
        for(auto iter = points.begin(); iter != points.end(); iter++){
            auto tP = this->mMat*glm::vec4((*iter)[0],(*iter)[1],(*iter)[2],1.0f);
            this->bufPoints.push_back(glm::vec3(tP[0],tP[1],tP[2]));
        }
    }
};

void planeSpline(const std::vector<plane>& planes, int N, std::vector<plane>& interpoledPlanes){
    std::vector<glm::vec3> pos;
    std::vector<glm::vec1> scale;
    std::vector<glm::fquat> rot;
    pos.clear();
    scale.clear();
    rot.clear();
    for(auto iter = planes.begin(); iter != planes.end(); iter++){
        pos.push_back((*iter).pos);
        scale.push_back((*iter).scale);
        rot.push_back((*iter).rot);
    }
    CATMULL_ROM_SPLINE_OPEN<glm::vec3> posSpline(pos);
    posSpline.interpol(N);
    CATMULL_ROM_SPLINE_OPEN<glm::vec1> scaleSpline(scale);
    scaleSpline.interpol(N);
    QSpline rotSpline(rot);
    rotSpline.interpol(N);
    pos.clear();
    scale.clear();
    rot.clear();
    scale = scaleSpline.points;
    pos = posSpline.points;
    rot = rotSpline.points;

    std::vector<std::vector<glm::vec3>> points; // points[N][M] : N th point of M th plane;
    points.clear();
    int numCP = planes[0].points.size();
    int numPlanes = planes.size();
    for(int n=0;n<numCP;n++){
        std::vector<glm::vec3> _points;
        _points.clear();
        for(int m=0; m<numPlanes; m++){
            _points.push_back(planes[m].points[n]);
        }
        CATMULL_ROM_SPLINE_OPEN<glm::vec3> ithPointSpline(_points);
        ithPointSpline.interpol(N);
        points.push_back(ithPointSpline.points);
    }
    numPlanes = points[0].size();
    std::cout<<scale.size()<<" "<<rot.size()<<" "<<pos.size()<<" "<<numPlanes<<std::endl;
    std::vector<std::vector<glm::vec3>> pl2pt; // pl2pt[M][N] : N th point of M th plane;
    pl2pt.clear();
    for(int m=0;m<numPlanes;m++){
        std::vector<glm::vec3> tmpPoints;
        tmpPoints.clear();
        for(int n=0; n<numCP; n++){
            tmpPoints.push_back(points[n][m]);
        }
        pl2pt.push_back(tmpPoints);
    }
    interpoledPlanes.clear();
    for(int i=0;i<numPlanes; i++){
        interpoledPlanes.push_back(plane(planes[0].curveType,pl2pt[i],pos[i],scale[i],rot[i]));
    }
}
#endif