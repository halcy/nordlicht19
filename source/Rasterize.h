#ifndef __RASTERIZE_H__
#define __RASTERIZE_H__

#include "VectorLibrary/Vector.h"
#include "VectorLibrary/Matrix.h"

#include <stdint.h>

typedef struct {
    int32_t v[4];
} index_triangle_t;

typedef struct {
    int32_t v[9];
} index_trianglepv_t;

typedef vec3_t init_vertex_t;

#define F(x) (x)

#endif
