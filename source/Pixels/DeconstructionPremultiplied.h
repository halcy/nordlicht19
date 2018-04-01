#ifndef __PIXELS_DECONSTRUCTION_PREMULTIPLIED_H__
#define __PIXELS_DECONSTRUCTION_PREMULTIPLIED_H__

#include "Functions.h"

#define ExtractRawRed(c) ( ((c)>>PixelRedShift)&BitMaskAt(0,PixelRedBits) )
#define ExtractRawGreen(c) ( ((c)>>PixelGreenShift)&BitMaskAt(0,PixelGreenBits) )
#define ExtractRawBlue(c) ( ((c)>>PixelBlueShift)&BitMaskAt(0,PixelBlueBits) )
#define ExtractRawAlpha(c) ( ((c)>>PixelAlphaShift)&BitMaskAt(0,PixelAlphaBits) )

#if PixelRedBits==8
#define ExtractRed(c) PixelUnMul(ExtractRawRed(c),ExtractRawAlpha(c))
#else
#define ExtractRed(c) PixelUnMul(ExtractAndExpandBitsAt((c),PixelRedShift,PixelRedBits)),ExtractRawAlpha(c))
#endif

#if PixelGreenBits==8
#define ExtractGreen(c) PixelUnMul(ExtractRawGreen(c),ExtractRawAlpha(c))
#else
#define ExtractGreen(c) PixelUnMul(ExtractAndExpandBitsAt((c),PixelGreenShift,PixelGreenBits),ExtractRawAlpha(c))
#endif

#if PixelBlueBits==8
#define ExtractBlue(c) PixelUnMul(ExtractRawBlue(c),ExtractRawAlpha(c))
#else
#define ExtractBlue(c) PixelUnMul(ExtractAndExpandBitsAt((c),PixelBlueShift,PixelBlueBits),ExtractRawAlpha(c))
#endif

#if PixelAlphaBits==8
#define ExtractAlpha(c) PixelUnMul(ExtractRawAlpha(c),ExtractRawAlpha(c))
#else
#define ExtractAlpha(c) PixelUnMul(ExtractAndExpandBitsAt((c),PixelAlphaShift,PixelAlphaBits),ExtractRawAlpha(c))
#endif

#define IsPixelTransparent(c) ( ((c)&UnclampedRGBA(0,0,0,0x80))?false:true )

#endif
