#ifndef __PIXELS_CONSTRUCTION_NO_ALPHA_H__
#define __PIXELS_CONSTRUCTION_NO_ALPHA_H__

#include "Functions.h"

#define RawRGB(r,g,b) ( (((int)(r))<<PixelRedShift)|(((int)(g))<<PixelGreenShift)|(((int)(b))<<PixelBlueShift) )

#define UnclampedRGB(r,g,b) RawRGB( \
	((int)(r))>>(8-PixelRedBits), \
	((int)(g))>>(8-PixelGreenBits), \
	((int)(b))>>(8-PixelBlueBits))

#define RGB(r,g,b) UnclampedRGB(Clamp(r,0,0xff),Clamp(g,0,0xff),Clamp(b,0,0xff))

#define RGBA(r,g,b,a) ( ((a)<0x80)?TransparentPixel: \
	UnclampedRGB(Clamp(r,0,0xff),Clamp(g,0,0xff),Clamp(b,0,0xff)) )

#endif
