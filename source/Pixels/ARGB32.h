#ifndef __PIXELS_ARGB_32_H__
#define __PIXELS_ARGB_32_H__

typedef uint32_t Pixel;
#define PixelSize 4

#define PixelRedBits 8
#define PixelGreenBits 8
#define PixelBlueBits 8
#define PixelAlphaBits 8

#include "Endianness.h"
#define PixelAlphaShift FirstByteShift
#define PixelRedShift SecondByteShift
#define PixelGreenShift ThirdByteShift
#define PixelBlueShift FourthByteShift

#include "Construction.h"
#include "Deconstruction.h"
#include "Composition32.h"
#include "Error.h"

#endif
