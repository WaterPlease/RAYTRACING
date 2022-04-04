#ifndef __MY_UTIL__
#define __MY_UTIL__
#define GLM_FORCE_SIMD_AVX2
#include <iostream>
#include <vector>
#include <cstdlib>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/glm.hpp>
#include <string>
#include <stdlib.h>
#include "math.h"
#include "Exception.hpp"

glm::vec3 getTrackballPos(int w, int h, int x, int y){
    int cX = w/2;
    int cY = h/2;
    float r = sqrtf((float)(cX*cX+cY*cY));
    float nX = (float)(x-cX)/r;
    float nY = (float)(y-cY)/r;
    float nZ = sqrtf(1-nX*nX-nY*nY);
    return glm::vec3(nX,-nY,nZ);
}

float getFovX(float fovy, int w, int h){
    fovy = fovy * M_PI / 180.0;
    auto fovx = 2 * atanf((float)w*tan(fovy/2)/(float)h);
    return fovx*180.0/M_PI;
}

std::string trim(const std::string& s){
    if(s.length() == 0)
       return s;

   std::size_t beg = s.find_first_not_of(" \a\b\f\n\r\t\v");
   std::size_t end = s.find_last_not_of(" \a\b\f\n\r\t\v");
   if(beg == std::string::npos) // No non-spaces
       return "";

   return std::string(s, beg, end - beg + 1);
}

std::string removeComment(const std::string& s){
    auto beg = s.find_first_of('#');
    if(beg==std::string::npos)
        return std::string(s);
    return trim(s.substr(0,beg));
}
template <int d>
glm::vec<d,float, glm::packed_highp> parseVec(std::string s){
    std::string delim(" ");
    size_t beg;
    size_t end;
    glm::vec<d,float, glm::packed_highp> v;
    int i=0;
    s = trim(s);
    while((beg = s.find_first_not_of(delim)) != std::string::npos){
        end = s.find_first_of(delim);
        v[i] = atof(s.substr(beg,end).c_str());
        i++;
        if(i==d){
            break;
        }
        s.erase(0,end);
        s = trim(s);
    }
    return v;
}

float parseFloat(std::string s){
    std::string delim(" ");
    size_t beg;
    size_t end;
    float v;
    try{    
        s = trim(s);
        if((beg = s.find_first_not_of(delim)) != std::string::npos){
            end = s.find_first_of(delim);
            v = atof(s.substr(beg,end).c_str());
        }
    }catch(std::exception e){
        throw ERRPARSE();
    }
    return v;
}

int parseInt(std::string s){
    std::string delim(" ");
    size_t beg;
    size_t end;
    int v;
    try{    
        s = trim(s);
        if((beg = s.find_first_not_of(delim)) != std::string::npos){
            end = s.find_first_of(delim);
            v = atoi(s.substr(beg,end).c_str());
        }
    }catch(std::exception e){
        throw ERRPARSE();
    }
    return v;
}

void tokenizer(const std::string& str,std::vector<std::string>& tokens){
    auto line = trim(str);
    bool isWord = true;
    tokens.clear();
    auto tempStr = std::string("");
    tempStr.reserve(256);
    for(auto iter = line.begin(); iter != line.end(); iter++){
        auto c = (*iter);
        if(c==' '){
            if(isWord){
                tokens.push_back(std::string(tempStr));
                tempStr.clear();
                isWord = false;
            }
            else continue;
        }else{
            tempStr.push_back(c);
            isWord = true;
        }
    }
    tokens.push_back(std::string(tempStr));
}

inline float pRand(){
    return (float)std::rand()/(float)RAND_MAX;
}

inline glm::vec3 random_point(){
    glm::vec3 r;
    float x,y,z;
    while(true){
        x = (float)std::rand()/(float)(RAND_MAX>>1)-1.0f;
        y = (float)std::rand()/(float)(RAND_MAX>>1)-1.0f;
        z = (float)std::rand()/(float)(RAND_MAX>>1)-1.0f;
        r = glm::vec3(x,y,z);
        if(glm::dot(r,r) < 1.0f) return glm::normalize(r);
    }
}

inline glm::vec3 random_point_cosine(const glm::vec3& N,const float c){
    glm::vec3 r;
    while(true){
       r = random_point(); 
       if(glm::dot(r,N) > c) return r;
    }
}

#endif