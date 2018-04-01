#ifndef __GENERIC_DRAWING_H__
#define __GENERIC_DRAWING_H__

#include "Drawing.h"

typedef void GenericDrawPixelFunction(Bitmap *bitmap,int x,int y,Pixel c,CompositionMode comp);
typedef void GenericDrawHorizontalLineFunction(Bitmap *bitmap,int x,int y,int len,Pixel c,CompositionMode comp);
typedef void GenericDrawVerticalLineFunction(Bitmap *bitmap,int x,int y,int len,Pixel c,CompositionMode comp);

static inline void GenericDrawPixel(Bitmap *bitmap,int x,int y,Pixel c,CompositionMode comp,GenericDrawPixelFunction *drawfunc)
{
	drawfunc(bitmap,x,y,c,comp);
}

static inline void DrawPixelFunction(Bitmap *bitmap,int x,int y,Pixel c,CompositionMode comp) { DrawPixel(bitmap,x,y,c); }
static inline void DrawPixelNoClipFunction(Bitmap *bitmap,int x,int y,Pixel c,CompositionMode comp) { DrawPixelNoClip(bitmap,x,y,c); }
static inline void CompositePixelFunction(Bitmap *bitmap,int x,int y,Pixel c,CompositionMode comp) { CompositePixel(bitmap,x,y,c,comp); }
static inline void CompositePixelNoClipFunction(Bitmap *bitmap,int x,int y,Pixel c,CompositionMode comp) { CompositePixelNoClip(bitmap,x,y,c,comp); }

static inline void GenericDrawHorizontalLine(Bitmap *bitmap,int x,int y,int len,Pixel c,CompositionMode comp,GenericDrawHorizontalLineFunction *drawfunc)
{
	drawfunc(bitmap,x,y,len,c,comp);
}

static inline void DrawHorizontalLineFunction(Bitmap *bitmap,int x,int y,int len,Pixel c,CompositionMode comp) { DrawHorizontalLine(bitmap,x,y,len,c); }
static inline void DrawHorizontalLineNoClipFunction(Bitmap *bitmap,int x,int y,int len,Pixel c,CompositionMode comp) { DrawHorizontalLineNoClip(bitmap,x,y,len,c); }
static inline void CompositeHorizontalLineFunction(Bitmap *bitmap,int x,int y,int len,Pixel c,CompositionMode comp) { CompositeHorizontalLine(bitmap,x,y,len,c,comp); }
static inline void CompositeHorizontalLineNoClipFunction(Bitmap *bitmap,int x,int y,int len,Pixel c,CompositionMode comp) { CompositeHorizontalLineNoClip(bitmap,x,y,len,c,comp); }

static inline void GenericDrawVerticalLine(Bitmap *bitmap,int x,int y,int len,Pixel c,CompositionMode comp,GenericDrawVerticalLineFunction *drawfunc)
{
	drawfunc(bitmap,x,y,len,c,comp);
}

static inline void DrawVerticalLineFunction(Bitmap *bitmap,int x,int y,int len,Pixel c,CompositionMode comp) { DrawVerticalLine(bitmap,x,y,len,c); }
static inline void DrawVerticalLineNoClipFunction(Bitmap *bitmap,int x,int y,int len,Pixel c,CompositionMode comp) { DrawVerticalLineNoClip(bitmap,x,y,len,c); }
static inline void CompositeVerticalLineFunction(Bitmap *bitmap,int x,int y,int len,Pixel c,CompositionMode comp) { CompositeVerticalLine(bitmap,x,y,len,c,comp); }
static inline void CompositeVerticalLineNoClipFunction(Bitmap *bitmap,int x,int y,int len,Pixel c,CompositionMode comp) { CompositeVerticalLineNoClip(bitmap,x,y,len,c,comp); }

#endif
