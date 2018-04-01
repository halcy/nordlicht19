#ifndef __HSV_H__
#define __HSV_H__

#include "Pixels.h"

// h is 0..1536, and repeats outside that range.
// s,v, and l are 0..255, and values outside that range are clamped.

Pixel Hue(int h);
Pixel HSVA(int h,int s,int v,int a);
Pixel HSLA(int h,int s,int l,int a);

static inline Pixel HSV(int h,int s,int v) { return HSVA(h,s,v,255); }
static inline Pixel HSL(int h,int s,int l) { return HSLA(h,s,l,255); }


// h is 0..1, and repeats outside that range.
// s,v, and l are 0..1, and values outside that range are clamped.

static inline Pixel Huef(float h) { return Hue(h*1536); }
static inline Pixel HSVAf(float h,float s,float v,float a) { return HSVA(h*1536,s*255,v*255,a*255); }
static inline Pixel HSLAf(float h,float s,float l,float a) { return HSLA(h*1536,s*255,l*255,a*255); }
static inline Pixel HSVf(float h,float s,float v) { return HSV(h*1536,s*255,v*255); }
static inline Pixel HSLf(float h,float s,float l) { return HSL(h*1536,s*255,l*255); }

#endif
