#ifndef __LOGO_H__
#define __LOGO_H__

#define numVerticesLogo 448
#define numNormalsLogo 188
#define numFacesLogo 856

#define numVerticesLogoNordlicht 572
#define numNormalsLogoNordlicht 258
#define numFacesLogoNordlicht 1120

#include "Rasterize.h"

extern const init_vertex_t verticesLogo[];
extern const init_vertex_t normalsLogo[];
extern const index_triangle_t facesLogo[];

extern const init_vertex_t verticesLogoNordlicht[];
extern const init_vertex_t normalsLogoNordlicht[];
extern const index_triangle_t facesLogoNordlicht[];

#endif
