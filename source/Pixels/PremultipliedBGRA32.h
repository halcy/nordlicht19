#ifndef __PIXELS_PREMULTIPLIED_BGRA_32_H__
#define __PIXELS_PREMULTIPLIED_BGRA_32_H__

typedef uint32_t Pixel;
#define PixelSize 4

#define PixelRedBits 8
#define PixelGreenBits 8
#define PixelBlueBits 8
#define PixelAlphaBits 8

#include "Endianness.h"
#define PixelBlueShift FirstByteShift
#define PixelGreenShift SecondByteShift
#define PixelRedShift ThirdByteShift
#define PixelAlphaShift FourthByteShift

#include "ConstructionPremultiplied.h"
#include "DeconstructionPremultiplied.h"
#include "CompositionPremultiplied32.h"

#endif
