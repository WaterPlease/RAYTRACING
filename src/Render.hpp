#ifndef __RENDER__
#define __RENDER__
#define GLM_FORCE_SIMD_AVX2

#include <time.h>
#include <thread>

#include "Image.hpp"
#include "Ray.hpp"
#include "cam.hpp"
#include "scene.hpp"
#include "parameters.hpp"
#include "util.hpp"

#include "objLoader.hpp"
#include "Drawables.hpp"
//#include "KDTREE.hpp"
#include "BVH.hpp"

class Renderer{
public:
    Image img;
    int w;
    int h;
    float max_t;
    Cam* camera;
    glm::vec3 camPos;
    glm::vec3 upperLeft;
    glm::vec3 nsX;
    glm::vec3 nsY;
    Scene* scene;
    //kTree* tree;
    BVHTree* tree;

public:
    Renderer(Cam* _cam,Scene* _scene,int _w, int _h,float _max_t)
        :camera(_cam),scene(_scene),img(_w,_h),w(_w),h(_h),max_t(_max_t){
        auto r = (this->h/2.0f)/tanf(glm::radians(this->camera->fovy/2.0f));
        this->nsX = this->camera->camRight;
        this->nsY = -this->camera->camUp;
        this->camPos = this->camera->r  * this->camera->camOri+this->camera->camOffset;
        this->upperLeft = ((this->camera->r - r) * this->camera->camOri) + this->camera->camOffset
                            - ((float)this->h/2.0f) * this->nsY
                            - ((float)this->w/2.0f) * this->nsX;
    }
    inline Ray getEyeRay(int i, int j,bool bMultisample){
        return Ray(this->camPos,this->upperLeft + this->nsX * ((float)i+pRand()) + this->nsY * ((float)j+pRand()) +-this->camPos);
    }
    void thread_RenderPixel(int x, int y){
            COLOR pixel(0.0f);
            for(int k = 0; k<sample_factor;k++){
                auto c = trace(this->getEyeRay(x,y,true),1);
                //if(c[0]<0) c = color_sky;
                pixel += c;
            }
            if(glm::dot(pixel,pixel) < eps) pixel = COLOR(0.0f);
            this->img.set(x,y,glm::clamp(pixel/(float)sample_factor,0.0f,1.0f));
    }
    void Render(){
        auto start = time(NULL);
        int numThread = 0;
        std::cout<<"Start Render"<<std::endl;
        std::cout<<"# of thread : "<<max_thread<<std::endl;
        std::cout<<"# of Sample per pixel : "<<sample_factor<<std::endl;
        std::cout<<"# of shadow sample : "<<sample_shadow<<std::endl;
        std::cout<<"soft shadow coefficient : "<<soft_shadow<<std::endl;
        std::vector<std::thread*> threads;
        this->imgExport("raytrace_tmp.png","opengl.png");
        for(int j=0;j<this->h;j++){
            for(int i=0;i<this->w;i++){
                /*
                if(threads.size() >= max_thread){
                    for(auto iter=threads.begin(); iter != threads.end(); iter++){
                        std::thread* t = *iter;
                        t->join();
                        delete t;
                    }
                    threads.clear();
                }*/
                std::thread* t = new std::thread(
                    &Renderer::thread_RenderPixel,this,
                    i,j
                );
                threads.push_back(t);
                
                //this->thread_RenderPixel(i,j);
            }
            for(auto iter=threads.begin(); iter != threads.end(); iter++){
                std::thread* t = *iter;
                t->join();
                delete t;
            }
            threads.clear();
            if(j%30==0){
                std::cout<<j+1<<" of "<<this->h<<"rows rendered"<<std::endl;
                this->imgExport("raytrace_tmp.png",nullptr);
            }
        }
        auto end = time(NULL);
        std::cout<<"Time : "<<(double)(end-start)<<std::endl;
        this->imgExport("raytrace.png",nullptr);
        std::cout<<"Image exported"<<std::endl;
    }
    bool shootRay(const Ray& r, float& t,HitRecord* rec,bool quickEnd){
        bool hitInScene = false;
        if(!bKDtree){
            Hittable* hittable;
            auto eIter = this->scene->hittables.end();
            for(auto iter = this->scene->hittables.begin();iter != eIter; iter++){
                HitRecord tmp_rec;
                hittable = (*iter);
                if(hittable->hit(r,eps,t,tmp_rec)){
                    t = tmp_rec.t;
                    *rec = tmp_rec;
                    hitInScene = true;
                    if(quickEnd) return hitInScene;
                }
            }
            return hitInScene;
        }else{
            Hittable* hittable;
            std::vector<Hittable*> hitLst;
            this->tree->ray_traversal(r,t,hitLst);
            auto eIter = hitLst.end();
            for(auto iter = hitLst.begin();iter != eIter; iter++){
                HitRecord tmp_rec;
                hittable = (*iter);
                if(hittable->hit(r,eps,t,tmp_rec)){
                    t = tmp_rec.t;
                    *rec = tmp_rec;
                    hitInScene = true;
                    if(quickEnd) return hitInScene;
                }
            }
            return hitInScene;
        }
    }
    COLOR calcShawdow(const HitRecord& rec,const glm::vec3& P,const glm::vec3& N){
        COLOR color = COLOR(0.0f);
        auto eIter = this->scene->lights.end();
        HitRecord sRec;
        for(auto iter=this->scene->lights.begin();iter != eIter; iter++){
            Light* ptr_light = *iter;
            glm::vec3 dir;
            float t_max;
            if(ptr_light->pos[3]<1e-6){ // directional light
                dir = (glm::vec3(ptr_light->pos[0],ptr_light->pos[1],ptr_light->pos[2]));
                t_max = 10000.0f;
            }
            else{                       // point light
                dir = glm::vec3(ptr_light->pos[0],ptr_light->pos[1],ptr_light->pos[2]) - P;
                t_max = glm::length(dir);
            }
            auto _dir = glm::normalize(dir) + random_point() * soft_shadow;
            _dir = glm::normalize(_dir);
            float shawdow_t = t_max;
            Ray shawdow_ray(P+(float)(1e-3)*glm::normalize(dir),_dir); // don't care refractional index
            auto isHit = this->shootRay(shawdow_ray,shawdow_t,&sRec,true);
            if(isHit){continue;}
            auto diffVec = glm::vec3(ptr_light->diff[0],ptr_light->diff[1],ptr_light->diff[2]);
            auto attenue = (ptr_light->pos[3]<1e-6)? 1.0f:1.0f/(ptr_light->att[0]+ptr_light->att[1]*t_max+ptr_light->att[2]*t_max*t_max);
            auto I = glm::dot(N,_dir);
            //color += (attenue*I)*diffVec*rec.ptr_hitObj->getDiffuseColor(glm::vec3(rec.u,rec.v,0.0f));
            if(I>0.0){
                color += (attenue*I)*diffVec*rec.ptr_hitObj->getDiffuseColor((void*)(&rec));
                /*
                if( == OBJ_TYPE::TRIANGLE){
                    color += (attenue*I)*diffVec*rec.ptr_hitObj->getDiffuseColor(glm::vec3(rec.u,rec.v,0.0f));
                }
                else{
                    printf("calcShadow - after shadow ray shot : P = %f %f %f\n",P[0],P[1],P[2]);
                    color += (attenue*I)*diffVec*rec.ptr_hitObj->getDiffuseColor(P);
                }*/
            }
            auto H = glm::normalize(dir-rec.dir);
            auto NH = glm::abs(glm::dot(H,N));
            auto specVec = glm::vec3(ptr_light->spec[0],ptr_light->spec[1],ptr_light->spec[2]);
            color += (attenue*glm::pow(NH,rec.ptr_hitObj->glossy))*specVec*rec.ptr_hitObj->getSpecularColor(P);
        }
        return color;
    }
    inline COLOR calcReflection(const Ray& r,int depth){
        COLOR c  = this->trace(r,depth+1);
        //if(c[0] < 0) return COLOR(0.0f);
        return this->trace(r,depth+1);
    }
    inline COLOR calcRefraction(const Ray& r,int depth){
        COLOR c  = this->trace(r,depth+1);
        //if(c[0] < 0) return COLOR(0.0f);
        return this->trace(r,depth+1);
    }
    COLOR trace(const Ray& r,int depth){
        if(depth > max_depth){
            return color_sky;
        }
        // hit calc
        float t = this->max_t;
        COLOR c = COLOR(0.0f);
        HitRecord rec;
        auto hitInScene = this->shootRay(r,t,&rec,false);
        if(hitInScene){
            auto P = r.at(t);
            auto ks = rec.ptr_hitObj->ks;
            auto kt = rec.ptr_hitObj->kt;
            auto trRidx = rec.ptr_hitObj->refractIdx;
            for(int i = 0; i < sample_shadow; i++){
                c += calcShawdow(rec,P,rec.N);
            }
            c = (1-ks-kt) * (c/(float)sample_shadow);
            if(rec.ptr_hitObj->mat_type == MAT_TYPE::TEXTURE){
                c += rec.ptr_hitObj->getDiffuseColor((void*)(&rec)) * rec.ptr_hitObj->getAmbientColor(nullptr);
            }else{
                c += rec.ptr_hitObj->getAmbientColor(nullptr);
            }
            auto spec_c = glm::vec3(0.0f);
            auto trans_c = glm::vec3(0.0f);
            auto fuzzy = rec.ptr_hitObj->fuzzySpec;
            if(ks > eps){
                glm::vec3 reflectedVec = glm::normalize(r.dir - 2.0f * (glm::dot(r.dir,rec.N)) * rec.N);
                if(glm::dot(reflectedVec,rec.N)>0){
                    if(fuzzy > eps){
                        for(int i=0;i<sample_spec;i++){
                            auto fuzzied = reflectedVec+fuzzy*random_point();
                            Ray reflectedRay(P+(float)(1e-3)*rec.N,glm::normalize(fuzzied));
                            spec_c += (calcReflection(reflectedRay,depth));
                        }
                        spec_c = ks*rec.ptr_hitObj->getDiffuseColor((void*)(&rec))*spec_c/(float)sample_spec;
                    }else{
                        Ray reflectedRay(P+(float)(1e-3)*rec.N,reflectedVec);
                        spec_c += ks*rec.ptr_hitObj->getDiffuseColor((void*)(&rec))*(calcReflection(reflectedRay,depth));
                    }
                }
            }  
            if(kt > eps){
                float ior;
                if(rec.front_face){
                    ior = 1.0f/trRidx;
                }else{
                    ior = trRidx;              
                }
                glm::vec3 _v;
                if(this->refract(r.dir,rec.N,ior,_v)){
                    Ray _r(P-((float)(1e-3))*rec.N,_v);
                    trans_c += kt * rec.ptr_hitObj->getDiffuseColor((void*)(&rec)) * calcRefraction(_r,depth);
                }else{
                    Ray _r(P+((float)(1e-3))*rec.N,glm::normalize(r.dir - 2.0f * (glm::dot(r.dir,rec.N)) * rec.N));
                    trans_c += kt * rec.ptr_hitObj->getDiffuseColor((void*)(&rec)) * calcReflection(_r,depth);
                }
            }
            c += spec_c;
            c += trans_c;
            return c;
        }
        return color_sky;
    }
    inline bool refract (const glm::vec3& i, const glm::vec3& n, float ior_ratio, glm::vec3& r) {
        auto cos_i = glm::dot(-i, n);
        auto cos_t2 = (1.0f) - ior_ratio * ior_ratio *  (1.0f- cos_i * cos_i);
        if (cos_t2 <= 0)
            return false;
        r = ior_ratio * i + ((ior_ratio * cos_i - sqrt(abs(cos_t2))) * n);
        return true;
    }
    inline float reflectance(float cosine, float ref_idx){
        // Use Schlick's approximation for reflectance.
        auto r0 = (1-ref_idx) / (1+ref_idx);
        r0 = r0*r0;
        return r0 + (1-r0)*pow((1 - cosine),5);
    }
    void imgExport(const char* path1, const char* path2){
        this->img.exportImage(path1,path2);
    }
};

#endif