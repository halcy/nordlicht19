#ifndef __EFFECTS_H__
#define __EFFECTS_H__

#include "Tools.h"

extern void effectScrollerInit();
extern void effectScrollerRender(C3D_RenderTarget* targetLeft, C3D_RenderTarget* targetRight, float iod, float time);
extern void effectScrollerExit(void);

extern void effectTunnelInit();
extern void effectTunnelRender(C3D_RenderTarget* targetLeft, C3D_RenderTarget* targetRight, float iod, float time);
extern void effectTunnelExit(void);

#endif