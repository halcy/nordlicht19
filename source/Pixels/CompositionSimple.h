#ifndef __PIXELS_COMPOSITION_SIMPLE_H__
#define __PIXELS_COMPOSITION_SIMPLE_H__

#include "Functions.h"
#include "CompositionFast.h"

static inline Pixel SourceOverCompositionMode(Pixel below,Pixel above)
{
	return FastTransparencyCompositionMode(below,above);
}

static inline Pixel AddCompositionMode(Pixel below,Pixel above)
{
	int r1=ExtractRawRed(below);
	int g1=ExtractRawGreen(below);
	int b1=ExtractRawBlue(below);
	int r2=ExtractRawRed(above);
	int g2=ExtractRawGreen(above);
	int b2=ExtractRawBlue(above);

	int r=r1+r2;
	int g=g1+g2;
	int b=b1+b2;
	if(r>=1<<PixelRedBits) r=(1<<PixelRedBits)-1;
	if(g>=1<<PixelGreenBits) g=(1<<PixelGreenBits)-1;
	if(b>=1<<PixelBlueBits) b=(1<<PixelBlueBits)-1;

	return RawRGB(r,g,b);
}

#endif
