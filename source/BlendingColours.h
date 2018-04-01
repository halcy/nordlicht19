#ifndef __BLENDING_COLOURS_H__
#define __BLENDING_COLOURS_H__

#include "Pixels.h"

Pixel MultiplyColours(Pixel c1,Pixel c2);
Pixel MultiplyColoursAndAlpha(Pixel c1,Pixel c2);
Pixel BlendColours(Pixel c1,Pixel c2,int t);
Pixel BlendColoursAndAlpha(Pixel c1,Pixel c2,int t);

#endif