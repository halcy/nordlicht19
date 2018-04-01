#ifndef __PIXELS_COMPOSITION_FAST_H__
#define __PIXELS_COMPOSITION_FAST_H__

#include "Functions.h"

#if PixelAlphaBits

#define PixelLowBits ( \
	(1<<PixelRedShift)| \
	(1<<PixelGreenShift)| \
	(1<<PixelBlueShift)| \
	(1<<PixelAlphaShift) )
#define PixelAllButHighBits ( (Pixel)~(\
	(1<<(PixelRedShift+PixelRedBits-1))| \
	(1<<(PixelGreenShift+PixelGreenBits-1))| \
	(1<<(PixelBlueShift+PixelBlueBits-1))| \
	(1<<(PixelAlphaShift+PixelAlphaBits-1)) ))

#else

#define PixelLowBits ( \
	(1<<PixelRedShift)| \
	(1<<PixelGreenShift)| \
	(1<<PixelBlueShift) )
#define PixelAllButHighBits ( (Pixel)~(\
	(1<<(PixelRedShift+PixelRedBits-1))| \
	(1<<(PixelGreenShift+PixelGreenBits-1))| \
	(1<<(PixelBlueShift+PixelBlueBits-1)) ))

#endif

static inline Pixel FastTransparencyCompositionMode(Pixel below,Pixel above)
{
	if(IsPixelTransparent(above)) return below;
	else return above;
}

static inline Pixel FastHalfTransparentCompositionMode(Pixel below,Pixel above)
{
	if(!IsPixelTransparent(above))
	{
		Pixel halfabove=(above>>1)&PixelAllButHighBits;
		Pixel halfbelow=(below>>1)&PixelAllButHighBits;
		Pixel carry=(above&below&PixelLowBits);

		return halfabove+halfbelow+carry;
	}
	else return below;
}

static inline Pixel DummyCompositionMode(Pixel below,Pixel above)
{
	return above;
}

#endif
