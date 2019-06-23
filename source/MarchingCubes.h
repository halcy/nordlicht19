// Simple marching cubes implementation
// Based mostly on code by Paul Bourke (Tables, whee)

#ifndef MARCHING_CUBES_H
#define MARCHING_CUBES_H

#include "Tools.h"

extern uint32_t polygonise(vec3_t* corners, float* values, vec3_t* normals, uint32_t isolevel, vertex* vertices);

#endif
