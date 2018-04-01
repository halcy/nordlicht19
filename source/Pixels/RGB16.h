#ifndef __PIXELS_RGB_16_H__
#define __PIXELS_RGB_16_H__

typedef uint16_t Pixel;
#define PixelSize 2

#define PixelRedBits 5
#define PixelGreenBits 6
#define PixelBlueBits 5
#define PixelAlphaBits 0

#define PixelRedShift 11
#define PixelGreenShift 5
#define PixelBlueShift 0
#define PixelAlphaShift 0

#define TransparentPixel 0xf81f

#include "ConstructionNoAlpha.h"
#include "DeconstructionNoAlpha.h"
#include "CompositionSimple.h"
#include "Error.h"

#endif

