#ifndef __PIXELS_CONSTRUCTION_H__
#define __PIXELS_CONSTRUCTION_H__

#include "Functions.h"

#define RawRGB(r,g,b) ( (((int)(r))<<PixelRedShift)|(((int)(g))<<PixelGreenShift)|(((int)(b))<<PixelBlueShift)|BitMaskAt(PixelAlphaShift,PixelAlphaBits) )
#define RawRGBA(r,g,b,a) ( (((int)(r))<<PixelRedShift)|(((int)(g))<<PixelGreenShift)|(((int)(b))<<PixelBlueShift)|(((int)(a))<<PixelAlphaShift) )

#define UnclampedRGB(r,g,b) RawRGB( \
	((int)(r))>>(8-PixelRedBits), \
	((int)(g))>>(8-PixelGreenBits), \
	((int)(b))>>(8-PixelBlueBits))

#define UnclampedRGBA(r,g,b,a) RawRGBA( \
	((int)(r))>>(8-PixelRedBits), \
	((int)(g))>>(8-PixelGreenBits), \
	((int)(b))>>(8-PixelBlueBits), \
	((int)(a))>>(8-PixelAlphaBits))

#define RGB(r,g,b) UnclampedRGB(Clamp(r,0,0xff),Clamp(g,0,0xff),Clamp(b,0,0xff))

#define RGBA(r,g,b,a) UnclampedRGBA(Clamp(r,0,0xff),Clamp(g,0,0xff),Clamp(b,0,0xff),Clamp(a,0,0xff))

#endif
