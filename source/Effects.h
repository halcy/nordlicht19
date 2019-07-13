#ifndef __EFFECTS_H__
#define __EFFECTS_H__

#include "Tools.h"

extern void effectSunInit();
extern void effectSunRender(C3D_RenderTarget* targetLeft, C3D_RenderTarget* targetRight, float iod, float time);
extern void effectSunExit();

extern void effectSun2Init();
extern void effectSun2Render(C3D_RenderTarget* targetLeft, C3D_RenderTarget* targetRight, float iod, float time);
extern void effectSun2Exit();

extern void effectDanceInit();
extern void effectDanceRender(C3D_RenderTarget* targetLeft, C3D_RenderTarget* targetRight, float iod, float time);
extern void effectDanceExit();

typedef void (*init_fun_t)();
typedef void (*render_fun_t)(C3D_RenderTarget* targetLeft, C3D_RenderTarget* targetRight, float iod, float time);
typedef void (*exit_fun_t)();

typedef struct effect { 
    init_fun_t init; 
    render_fun_t render; 
    exit_fun_t exit; 
} effect;

#endif
