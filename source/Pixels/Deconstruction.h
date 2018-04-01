#ifndef __PIXELS_DECONSTRUCTION_H__
#define __PIXELS_DECONSTRUCTION_H__

#include "Functions.h"

#define ExtractRawRed(c) ( ((c)>>PixelRedShift)&BitMaskAt(0,PixelRedBits) )
#define ExtractRawGreen(c) ( ((c)>>PixelGreenShift)&BitMaskAt(0,PixelGreenBits) )
#define ExtractRawBlue(c) ( ((c)>>PixelBlueShift)&BitMaskAt(0,PixelBlueBits) )
#define ExtractRawAlpha(c) ( ((c)>>PixelAlphaShift)&BitMaskAt(0,PixelAlphaBits) )

#if PixelRedBits==8
#define ExtractRed ExtractRawRed
#else
#define ExtractRed(c) ExtractAndExpandBitsAt((c),PixelRedShift,PixelRedBits)
#endif

#if PixelGreenBits==8
#define ExtractGreen ExtractRawGreen
#else
#define ExtractGreen(c) ExtractAndExpandBitsAt((c),PixelGreenShift,PixelGreenBits)
#endif

#if PixelBlueBits==8
#define ExtractBlue ExtractRawBlue
#else
#define ExtractBlue(c) ExtractAndExpandBitsAt((c),PixelBlueShift,PixelBlueBits)
#endif

#if PixelAlphaBits==8
#define ExtractAlpha ExtractRawAlpha
#else
#define ExtractAlpha(c) ExtractAndExpandBitsAt((c),PixelAlphaShift,PixelAlphaBits)
#endif

#define IsPixelTransparent(c) ( ((c)&UnclampedRGBA(0,0,0,0x80))?false:true )

#endif
