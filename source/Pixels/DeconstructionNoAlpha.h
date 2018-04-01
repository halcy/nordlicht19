#ifndef __PIXELS_DECONSTRUCTION_NO_ALPHA_H__
#define __PIXELS_DECONSTRUCTION_NO_ALPHA_H__

#include "Functions.h"

#define ExtractRawRed(c) ( ((c)>>PixelRedShift)&BitMaskAt(0,PixelRedBits) )
#define ExtractRawGreen(c) ( ((c)>>PixelGreenShift)&BitMaskAt(0,PixelGreenBits) )
#define ExtractRawBlue(c) ( ((c)>>PixelBlueShift)&BitMaskAt(0,PixelBlueBits) )

#define ExtractRed(c) ExtractAndExpandBitsAt((c),PixelRedShift,PixelRedBits)
#define ExtractGreen(c) ExtractAndExpandBitsAt((c),PixelGreenShift,PixelGreenBits)
#define ExtractBlue(c) ExtractAndExpandBitsAt((c),PixelBlueShift,PixelBlueBits)
#define ExtractAlpha(c) ( (c)==TransparentPixel?0:0xff )

#define IsPixelTransparent(c) ( (c)==TransparentPixel?true:false )

#endif
