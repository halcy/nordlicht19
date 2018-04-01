#ifndef __ADAPTIVE_BITMAP_H__
#define __ADAPTIVE_BITMAP_H__

#include "Bitmap.h"
#include "RLEBitmap.h"
#include "Drawing.h"

typedef struct AdaptiveBitmap {} AdaptiveBitmap;

#define StartBitmapToAdaptiveBitmapCast (const AdaptiveBitmap *)
#define EndBitmapToAdaptiveBitmapCast 
#define StartRLEBitmapToAdaptiveBitmapCast (const AdaptiveBitmap *)(1+(uintptr_t)(
#define EndRLEBitmapToAdaptiveBitmapCast ))

static inline const AdaptiveBitmap *AdaptiveBitmapWithBitmap(const Bitmap *bitmap)
{
	return StartBitmapToAdaptiveBitmapCast bitmap EndBitmapToAdaptiveBitmapCast;
}

static inline const AdaptiveBitmap *AdaptiveBitmapWithRLEBitmap(const RLEBitmap *rle)
{
	return StartRLEBitmapToAdaptiveBitmapCast rle EndRLEBitmapToAdaptiveBitmapCast;
}

static inline bool IsAdaptiveBitmapRLE(const AdaptiveBitmap *self)
{
	return ((uintptr_t)self)&1;
}

static inline bool IsAdaptiveBitmapTransparent(const AdaptiveBitmap *self)
{
	if(!self) return true;
	return IsAdaptiveBitmapRLE(self);
}

static inline const void *PointerForAdaptiveBitmap(const AdaptiveBitmap *self)
{
	return (const void *)(((uintptr_t)self)&~1);
}

static inline int AdaptiveBitmapWidth(const AdaptiveBitmap *self)
{
	if(!self) return 0;
	return ((const struct { int16_t width,height; } *)PointerForAdaptiveBitmap(self))->width;
}

static inline int AdaptiveBitmapHeight(const AdaptiveBitmap *self)
{
	if(!self) return 0;
	return ((const struct { int16_t width,height; } *)PointerForAdaptiveBitmap(self))->height;
}

static inline void DrawAdaptiveBitmap(Bitmap *bitmap,const AdaptiveBitmap *src,int x,int y)
{
	if(!src) return;
	if(IsAdaptiveBitmapRLE(src)) DrawRLEBitmap(bitmap,PointerForAdaptiveBitmap(src),x,y);
	else DrawBitmap(bitmap,PointerForAdaptiveBitmap(src),x,y);
}

static inline void DrawAdaptiveBitmapNoClip(Bitmap *bitmap,const AdaptiveBitmap *src,int x,int y)
{
	if(!src) return;
	if(IsAdaptiveBitmapRLE(src)) DrawRLEBitmapNoClip(bitmap,PointerForAdaptiveBitmap(src),x,y);
	else DrawBitmapNoClip(bitmap,PointerForAdaptiveBitmap(src),x,y);
}

static inline void CompositeAdaptiveBitmap(Bitmap *bitmap,const AdaptiveBitmap *src,int x,int y,CompositionMode mode)
{
	if(!src) return;
	if(IsAdaptiveBitmapRLE(src)) CompositeRLEBitmap(bitmap,PointerForAdaptiveBitmap(src),x,y,mode);
	else CompositeBitmap(bitmap,PointerForAdaptiveBitmap(src),x,y,mode);
}

static inline void CompositeAdaptiveBitmapNoClip(Bitmap *bitmap,const AdaptiveBitmap *src,int x,int y,CompositionMode mode)
{
	if(!src) return;
	if(IsAdaptiveBitmapRLE(src)) CompositeRLEBitmapNoClip(bitmap,PointerForAdaptiveBitmap(src),x,y,mode);
	else CompositeBitmapNoClip(bitmap,PointerForAdaptiveBitmap(src),x,y,mode);
}

#endif
