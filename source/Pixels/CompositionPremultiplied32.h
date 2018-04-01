#ifndef __PIXELS_COMPOSITION_PREMULTIPLIED_32_H__
#define __PIXELS_COMPOSITION_PREMULTIPLIED_32_H__

#include "Functions.h"
#include "CompositionFast.h"

static inline Pixel SourceOverCompositionMode(Pixel below,Pixel above)
{
	int r1=ExtractRawRed(below);
	int g1=ExtractRawGreen(below);
	int b1=ExtractRawBlue(below);
	int a1=ExtractRawAlpha(below);
	int r2=ExtractRawRed(above);
	int g2=ExtractRawGreen(above);
	int b2=ExtractRawBlue(above);
	int a2=ExtractRawAlpha(above);

	int r=r2+PixelMul(255-a2,r1);
	int g=g2+PixelMul(255-a2,g1);
	int b=b2+PixelMul(255-a2,b1);
	int a=a2+PixelMul(255-a2,a1);

	return RawRGBA(r,g,b,a);
}

#endif
