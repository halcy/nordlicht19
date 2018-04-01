#ifndef __PIXELS_H__
#define __PIXELS_H__

#include <stdint.h>
#include <stdbool.h>

#if defined(PremultipliedABGR32Pixels)
#include "Pixels/PremultipliedABGR32.h"
#elif defined(PremultipliedARGB32Pixels)
#include "Pixels/PremultipliedARGB32.h"
#elif defined(PremultipliedBGRA32Pixels)
#include "Pixels/PremultipliedBGRA32.h"
#elif defined(PremultipliedRGBA32Pixels)
#include "Pixels/PremultipliedRGBA32.h"

#elif defined(ABGR32Pixels)
#include "Pixels/ABGR32.h"
#elif defined(ARGB32Pixels)
#include "Pixels/ARGB32.h"
#elif defined(BGRA32Pixels)
#include "Pixels/BGRA32.h"
#elif defined(RGBA32Pixels)
#include "Pixels/RGBA32.h"

#elif defined(ARGB16Pixels)
#include "Pixels/ARGB16.h"
#elif defined(RGB16Pixels)
#include "Pixels/RGB16.h"

#elif defined(RGB8Pixels)
#include "Pixels/RGB8.h"

#endif

#define RGBf(r,g,b) RGB((r)*255,(g)*255,(b)*255)
#define RGBAf(r,g,b,a) RGBA((r)*255,(g)*255,(b)*255,(a)*255)
#define White(l) RGB(l,l,l)
#define Whitef(l) Grey(l*255)
#define Grey(v) White(v)

#endif
