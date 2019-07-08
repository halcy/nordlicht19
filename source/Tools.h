#ifndef __TOOLS_H__
#define __TOOLS_H__

#include <3ds.h>
#include <citro3d.h>
#include <string.h>
#include <stdio.h>
#include <tex3ds.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>
#include <stdarg.h>
#include <unistd.h>

#include <fcntl.h>

#include <sys/types.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "Pixels.h"
#include "Bitmap.h"
#include "Drawing.h"
#include "Perlin.h"
#include "VectorLibrary/Vector.h"
#include "VectorLibrary/Matrix.h"

#include "Rasterize.h"

#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))

#include "Rocket/sync.h"
struct sync_device *rocket;

#define SCREEN_WIDTH 400
#define SCREEN_HEIGHT 240

#define SCREEN_TEXTURE_WIDTH 512
#define SCREEN_TEXTURE_HEIGHT 512

#define DISPLAY_TRANSFER_FLAGS \
    (GX_TRANSFER_FLIP_VERT(0) | GX_TRANSFER_OUT_TILED(0) | GX_TRANSFER_RAW_COPY(0) | \
    GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8) | GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGBA8) | \
    GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO))

#define TEXTURE_TRANSFER_FLAGS \
    (GX_TRANSFER_FLIP_VERT(0) | GX_TRANSFER_OUT_TILED(1) | GX_TRANSFER_RAW_COPY(0) | \
    GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8) | GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGBA8) | \
    GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO))

#define DISPLAY_TOBOT_FLAGS \
    (GX_TRANSFER_FLIP_VERT(0) | GX_TRANSFER_OUT_TILED(0) | GX_TRANSFER_RAW_COPY(0) | \
    GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8) | GX_TRANSFER_OUT_FORMAT(GSP_RGBA8_OES) | \
    GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO))
    
extern int RandomInteger();
extern int nnnoise(int x, int y, int oct);

extern float badFresnel(float input, float expo);
extern float lutAbsLinear(float input, float offset);
extern float lutAbsInverseLinear(float input, float offset);
extern float lutOne(float input, float offset);
extern float lutZero(float input, float offset);
extern float lutPosPower(float input, float expo);

extern void fullscreenQuad(C3D_Tex texture, float iod, float iodmult);
extern void fullscreenQuadFlat(C3D_Tex texture);
extern void fullscreenQuadGlitch(C3D_Tex texture, int parts, float time, float amount);

extern int32_t mulf32(int32_t a, int32_t b);
extern int32_t divf32(int32_t a, int32_t b);

typedef struct { float position[3]; float texcoord[2]; float normal[3]; } vertex;

inline void setVert(vertex* vert, vec3_t p, vec2_t t) {
    vert->position[0] = p.x;
    vert->position[1] = p.y;
    vert->position[2] = p.z;
    vert->texcoord[0] = t.x;
    vert->texcoord[1] = t.y;
}

inline void setVertNorm(vertex* vert, vec3_t p, vec2_t t, vec3_t n) {
    vert->position[0] = p.x;
    vert->position[1] = p.y;
    vert->position[2] = p.z;
    
    vert->normal[0] = n.x;
    vert->normal[1] = n.y;
    vert->normal[2] = n.z;
    
    vert->texcoord[0] = t.x;
    vert->texcoord[1] = t.y;
}

// a -- b
// |    |
// d -- c
inline int buildQuad(vertex* vert, vec3_t a, vec3_t b, vec3_t c, vec3_t d, vec2_t ta, vec2_t tb, vec2_t tc, vec2_t td) {
        setVert(vert, a, ta); vert++;
        setVert(vert, b, tb); vert++;
        setVert(vert, c, tc); vert++;        
        setVert(vert, a, ta); vert++;
        setVert(vert, c, tc); vert++;
        setVert(vert, d, td); vert++;
        
        return 6;
}

inline int buildQuadNormal(vertex* vert, vec3_t a, vec3_t b, vec3_t c, vec3_t d, vec2_t ta, vec2_t tb, vec2_t tc, vec2_t td, vec3_t n) {
        setVertNorm(vert, a, ta, n); vert++;
        setVertNorm(vert, b, tb, n); vert++;
        setVertNorm(vert, c, tc, n); vert++;        
        setVertNorm(vert, a, ta, n); vert++;
        setVertNorm(vert, c, tc, n); vert++;
        setVertNorm(vert, d, td, n); vert++;
        
        return 6;
}

inline int buildQuadProjectiveXY(vertex* vert, vec3_t a, vec3_t b, vec3_t c, vec3_t d, float biasx, float biasy, float scale) {
        vec2_t ta = vec2((a.x + biasx) * scale, (a.y + biasy) * scale);
        vec2_t tb = vec2((b.x + biasx) * scale, (b.y + biasy) * scale);
        vec2_t tc = vec2((c.x + biasx) * scale, (c.y + biasy) * scale);
        vec2_t td = vec2((d.x + biasx) * scale, (d.y + biasy) * scale);
        setVert(vert, a, ta); vert++;
        setVert(vert, b, tb); vert++;
        setVert(vert, c, td); vert++;        
        setVert(vert, a, ta); vert++;
        setVert(vert, c, td); vert++;
        setVert(vert, d, tc); vert++;
        
        return 6;
}

// a -- b     .
// |\   |\    .
// d -- c \   .
//  \ e -\ f  .
//   \|   \|  .
//    h -- g  .
inline int buildCube(vertex* vert, vec3_t cp, float r, float hh, float vv) {
        // Texcoords
        vec2_t t1 = vec2(hh + 0.0, vv + 0.0);
        vec2_t t2 = vec2(hh + 0.0, vv + 0.5);
        vec2_t t3 = vec2(hh + 0.5, vv + 0.5);
        vec2_t t4 = vec2(hh + 0.5, vv + 0.0);
        
        // Corners
        vec3_t a = vec3add(cp, vec3(-r, -r, -r));
        vec3_t b = vec3add(cp, vec3( r, -r, -r));
        vec3_t c = vec3add(cp, vec3( r, -r,  r));
        vec3_t d = vec3add(cp, vec3(-r, -r,  r));
        vec3_t e = vec3add(cp, vec3(-r,  r, -r));
        vec3_t f = vec3add(cp, vec3( r,  r, -r));
        vec3_t g = vec3add(cp, vec3( r,  r,  r));
        vec3_t h = vec3add(cp, vec3(-r,  r,  r));
        
        // Normals
        vec3_t n1 = vec3( 1,  0,  0);
        vec3_t n2 = vec3(-1,  0,  0);
        vec3_t n3 = vec3( 0,  1,  0);
        vec3_t n4 = vec3( 0, -1,  0);
        vec3_t n5 = vec3( 0,  0,  1);
        vec3_t n6 = vec3( 0,  0, -1);
        
        // Faces
        vert += buildQuadNormal(vert, a, b, c, d, t1, t2, t3, t4, n5);
        vert += buildQuadNormal(vert, d, c, g, h, t1, t2, t3, t4, n4);
        vert += buildQuadNormal(vert, h, g, f, e, t1, t2, t3, t4, n6);
        vert += buildQuadNormal(vert, e, f, b, a, t1, t2, t3, t4, n3);
        vert += buildQuadNormal(vert, b, f, g, c, t1, t2, t3, t4, n1);
        vert += buildQuadNormal(vert, e, a, d, h, t1, t2, t3, t4, n2);
        
        // Done
        return(6 * 6);
}

inline int buildCuboid(vertex* vert, vec3_t cp, vec3_t r, float hh, float vv) {
        // Texcoords
        vec2_t t1 = vec2(hh + 0.0, vv + 0.0);
        vec2_t t2 = vec2(hh + 0.0, vv + 0.5);
        vec2_t t3 = vec2(hh + 0.5, vv + 0.5);
        vec2_t t4 = vec2(hh + 0.5, vv + 0.0);
        
        // Corners
        vec3_t a = vec3add(cp, vec3(-r.x, -r.y, -r.z));
        vec3_t b = vec3add(cp, vec3( r.x, -r.y, -r.z));
        vec3_t c = vec3add(cp, vec3( r.x, -r.y,  r.z));
        vec3_t d = vec3add(cp, vec3(-r.x, -r.y,  r.z));
        vec3_t e = vec3add(cp, vec3(-r.x,  r.y, -r.z));
        vec3_t f = vec3add(cp, vec3( r.x,  r.y, -r.z));
        vec3_t g = vec3add(cp, vec3( r.x,  r.y,  r.z));
        vec3_t h = vec3add(cp, vec3(-r.x,  r.y,  r.z));
        
        // Normals
        vec3_t n1 = vec3( 1,  0,  0);
        vec3_t n2 = vec3(-1,  0,  0);
        vec3_t n3 = vec3( 0,  1,  0);
        vec3_t n4 = vec3( 0, -1,  0);
        vec3_t n5 = vec3( 0,  0,  1);
        vec3_t n6 = vec3( 0,  0, -1);
        
        // Faces
        vert += buildQuadNormal(vert, a, b, c, d, t1, t2, t3, t4, n5);
        vert += buildQuadNormal(vert, d, c, g, h, t1, t2, t3, t4, n4);
//         vert += buildQuadNormal(vert, h, g, f, e, t1, t2, t3, t4, n6);
//         vert += buildQuadNormal(vert, e, f, b, a, t1, t2, t3, t4, n3);
        vert += buildQuadNormal(vert, b, f, g, c, t1, t2, t3, t4, n1);
        vert += buildQuadNormal(vert, e, a, d, h, t1, t2, t3, t4, n2);
        
        // Done
        return(6 * 6);
}

int32_t loadObject(int32_t numFaces, const index_triangle_t* faces, const init_vertex_t* vertices, const init_vertex_t* normals, const vec2_t* texcoords, vertex* vbo);
int32_t loadObject2(int32_t numFaces, const index_trianglepv_t* faces, const init_vertex_t* vertices, const init_vertex_t* normals, const vec2_t* texcoords, vertex* vbo);

extern void fade();
extern void resetShadeEnv();

extern void startPerfCounter(int idx);
extern void stopPerfCounter(int idx);
extern float readPerfCounter(int idx);

extern bool loadTex3DS(C3D_Tex* tex, C3D_TexCube* cube, const char* path);
#endif
