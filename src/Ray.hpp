#ifndef __RAY__
#define __RAY__
#define GLM_FORCE_SIMD_AVX2
#include <glm/glm.hpp>

class Ray{
public:
    glm::vec3 ori;
    glm::vec3 dir;
    Ray(const glm::vec3& _ori, const glm::vec3& _dir)
        :  ori(_ori),dir(glm::normalize(_dir)){ }

    glm::vec3 at(float t) const{
        return this->ori + t * this->dir;
    }
};

#endif