#ifndef __EFFECTS_H__
#define __EFFECTS_H__

#include "Tools.h"

extern void effectScrollerInit();
extern void effectScrollerRender(C3D_RenderTarget* targetLeft, C3D_RenderTarget* targetRight, float iod, float time);
extern void effectScrollerExit(void);

extern void effectTunnelInit();
extern void effectTunnelRender(C3D_RenderTarget* targetLeft, C3D_RenderTarget* targetRight, float iod, float time);
extern void effectTunnelExit();

extern void effectTunnel2Init();
extern void effectTunnel2Render(C3D_RenderTarget* targetLeft, C3D_RenderTarget* targetRight, float iod, float time);
extern void effectTunnel2Exit();

extern void effectLogoInit();
extern void effectLogoRender(C3D_RenderTarget* targetLeft, C3D_RenderTarget* targetRight, float iod, float time);
extern void effectLogoExit();

extern void effectMetaobjectsInit();
extern void effectMetaobjectsRender(C3D_RenderTarget* targetLeft, C3D_RenderTarget* targetRight, float iod, float time);
extern void effectMetaobjectsExit();

extern void effectCatteInit();
extern void effectCatteRender(C3D_RenderTarget* targetLeft, C3D_RenderTarget* targetRight, float iod, float time);
extern void effectCatteExit();

typedef void (*init_fun_t)();
typedef void (*render_fun_t)(C3D_RenderTarget* targetLeft, C3D_RenderTarget* targetRight, float iod, float time);
typedef void (*exit_fun_t)();

typedef struct effect { 
    init_fun_t init; 
    render_fun_t render; 
    exit_fun_t exit; 
} effect;

#endif
