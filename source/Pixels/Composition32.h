#ifndef __PIXELS_COMPOSITION_32_H__
#define __PIXELS_COMPOSITION_32_H__

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

	int r=PixelMul(a2,r2)+PixelMul(255-a2,r1);
	int g=PixelMul(a2,g2)+PixelMul(255-a2,g1);
	int b=PixelMul(a2,b2)+PixelMul(255-a2,b1);
	int a=PixelMul(a2,a2)+PixelMul(255-a2,a1);

	return RawRGBA(r,g,b,a);
}

static inline Pixel AddCompositionMode(Pixel below,Pixel above)
{
	int r1=ExtractRawRed(below);
	int g1=ExtractRawGreen(below);
	int b1=ExtractRawBlue(below);
	int a1=ExtractRawAlpha(below);
	int r2=ExtractRawRed(above);
	int g2=ExtractRawGreen(above);
	int b2=ExtractRawBlue(above);
	int a2=ExtractRawAlpha(above);

	int r=r1+r2;
	int g=g1+g2;
	int b=b1+b2;
	int a=a1+a2;
	if(r>=1<<PixelRedBits) r=(1<<PixelRedBits)-1;
	if(g>=1<<PixelGreenBits) g=(1<<PixelGreenBits)-1;
	if(b>=1<<PixelBlueBits) b=(1<<PixelBlueBits)-1;
	if(a>=1<<PixelAlphaBits) a=(1<<PixelAlphaBits)-1;

	return RawRGBA(r,g,b,a);
}

#endif
