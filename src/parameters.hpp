// INCLUDE HERE
#ifndef __PARAMETER__
#define __PARAMETER__
#define GLM_FORCE_SIMD_AVX2
#include <time.h>
#include "GL/freeglut.h"
#include "glm/glm.hpp"


#define COLOR glm::vec3

int max_thread = 6;

int max_depth = 3;
int sample_factor = 32;
float soft_shadow = 0.1f;
int sample_shadow = 8;
int sample_spec = 8;

COLOR color_sky = COLOR(0.31f,0.74f,0.87f);

float emptyRefIdx = 1.0f;

bool bKDtree = false;
int KD_MAX_Depth = 1;
int KD_MAX_TRI = 10;

// PARAMETERS

const float eps = 1e-6;

GLfloat fov = 45.0f;
clock_t min_deltaTime = 16;

GLfloat AngleDeltaFactor = 5.0f;

int numOnplaneSegment = 3;
int numOnmodelSegment = 3;

// pre-render mode
bool bWireMode = false;
int render_kd_depth = -1;

#endif