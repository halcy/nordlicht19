#ifndef __ICOSPHERE_H__
#define __ICOSPHERE_H__

#include "Rasterize.h"

#define icosphereNumVertices 162
#define icosphereNumNormals 320
#define icosphereNumFaces 320

extern const init_vertex_t icosphereVertices[];
extern const init_vertex_t icosphereNormals[];
extern const index_triangle_t icosphereFaces[];

#endif
