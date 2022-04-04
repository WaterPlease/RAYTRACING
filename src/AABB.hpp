#ifndef __AABB__
#define __AABB__
#define GLM_FORCE_SIMD_AVX2
#include <glm/glm.hpp>

#include "Ray.hpp"

class AABB{
public:
    glm::vec3 min;
    glm::vec3 max;
    AABB(){}
    AABB(const glm::vec3& _min, const glm::vec3& _max):
        min(_min),max(_max){}
        
    bool hit(const Ray& r,float t_min,float t_max){
    // Optimized AABB hit algorithm by Andrew Kensler.
        for (int a = 0; a < 3; a++) {
            float invD = 1.0f / r.dir[a];
            float t0 = (this->min[a] - r.ori[a]) * invD;
            float t1 = (this->max[a] - r.ori[a]) * invD;
            if (invD < 0.0f)
                std::swap(t0, t1);
            t_min = t0 > t_min ? t0 : t_min;
            t_max = t1 < t_max ? t1 : t_max;
            if (t_max <= t_min)
                return false;
        }
        return true;
    }
};

#endif