#ifndef __PIXELS_ARGB_16_H__
#define __PIXELS_ARGB_16_H__

typedef uint16_t Pixel;
#define PixelSize 2

#define PixelRedBits 5
#define PixelGreenBits 5
#define PixelBlueBits 5
#define PixelAlphaBits 1

#define PixelRedShift 10
#define PixelGreenShift 5
#define PixelBlueShift 0
#define PixelAlphaShift 15

#include "Construction.h"
#include "Deconstruction.h"
#include "CompositionSimple.h"
#include "Error.h"

#endif
