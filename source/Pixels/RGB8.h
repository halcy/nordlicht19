#ifndef __PIXELS_RGB_8_H__
#define __PIXELS_RGB_8_H__

typedef uint8_t Pixel;
#define PixelSize 1

#define PixelRedBits 3
#define PixelGreenBits 3
#define PixelBlueBits 2
#define PixelAlphaBits 0

#define PixelRedShift 5
#define PixelGreenShift 2
#define PixelBlueShift 0
#define PixelAlphaShift 0

#define TransparentPixel 0xe3

#include "ConstructionNoAlpha.h"
#include "DeconstructionNoAlpha.h"
#include "CompositionSimple.h"
#include "Error.h"

#endif
