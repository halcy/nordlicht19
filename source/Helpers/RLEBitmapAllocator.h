#ifndef __HELPERS_RLE_BITMAP_ALLOCATOR_H__
#define __HELPERS_RLE_BITMAP_ALLOCATOR_H__

#include "../RLEBitmap.h"

#include <stdlib.h>

static inline RLEBitmap *AllocateRLEBitmapWithBitmap(const Bitmap *bitmap)
{
	RLEBitmap *self=malloc(SizeOfRLEBitmapFromBitmap(bitmap));
	if(!self) return NULL;

	InitialiseRLEBitmapFromBitmap(self,bitmap);

	return self;
}

static inline RLEBitmap *AllocateRLEBitmapWithPartialBitmap(const Bitmap *bitmap,int x,int y,int width,int height)
{
	RLEBitmap *self=malloc(SizeOfRLEBitmapFromBitmap(bitmap));
	if(!self) return NULL;

	InitialiseRLEBitmapFromPartialBitmap(self,bitmap,x,y,width,height);

	return self;
}

static inline void FreeRLEBitmap(RLEBitmap *self)
{
	free(self);
}

#endif
