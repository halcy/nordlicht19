#ifndef __TOOLS_H__
#define __TOOLS_H__

#include <3ds.h>
#include <citro3d.h>
#include <string.h>
#include <stdio.h>

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

extern void fade();

#endif
