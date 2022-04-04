#ifndef __IMAGE_WRITE__
#define __IMAGE_WRITE__
#define GLM_FORCE_SIMD_AVX2
#include <glm/glm.hpp>
#include <math.h>
#include <GL/freeglut.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "parameters.hpp"

const int dim = 3;

class Image{
    const int w;
    const int h;
    glm::vec3* data;
public:
    Image(int _w, int _h):w(_w),h(_h){
        this->data = new glm::vec3[w*h];
    }

    inline glm::vec3 at(int x,int y){
        return this->data[y*this->w+x];
    }
    inline void set(int x, int y, const glm::vec3 p){
        this->data[y*this->w+x] = p;
    }

    void exportImage(const char* path1,const char* path2){
        unsigned char* pixels = new unsigned char[this->w*this->h*dim];
        auto idx = 0;
        for(int j=0;j<this->h;j++){
            for(int i=0;i<this->w;i++){
                auto p = this->at(i,j);
                pixels[idx++] = (unsigned char)std::lroundf((p[0]*255.0f));
                pixels[idx++] = (unsigned char)std::lroundf((p[1]*255.0f));
                pixels[idx++] = (unsigned char)std::lroundf((p[2]*255.0f));
            }
        }
        if(path1!=nullptr)
            stbi_write_png(path1,this->w,this->h,dim,pixels,this->w * dim);
        //stbi_write_bmp(path,this->w,this->h,dim,pixels);
        size_t _idx;
        unsigned char* _glPixels = new unsigned char[this->w*this->h*dim];
        unsigned char* glPixels = new unsigned char[this->w*this->h*dim];
        glReadPixels(0, 0, this->w,this->h, GL_RGB, GL_UNSIGNED_BYTE, _glPixels);
        for(int j=0; j<this->h; j++){
            for(int i=0; i< this->w; i++){
                _idx = dim*this->w*j+dim*i;
                idx = dim*this->w*(this->h-j-1)+dim*i;
                glPixels[idx++] = _glPixels[_idx++];
                glPixels[idx++] = _glPixels[_idx++];
                glPixels[idx++] = _glPixels[_idx++];
            }
        }
        if(path2!=nullptr)
            stbi_write_png(path2,this->w,this->h,dim,glPixels,this->w * dim);

        delete pixels;
        delete glPixels;
    }

    ~Image(){
        delete data;
    }
};

#endif