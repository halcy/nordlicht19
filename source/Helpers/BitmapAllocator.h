#ifndef __HELPERS_BITMAP_ALLOCATOR_H__
#define __HELPERS_BITMAP_ALLOCATOR_H__

#include "../Bitmap.h"
#include "../Drawing.h"

#include <stdlib.h>

static inline Bitmap *AllocateBitmap(int width,int height)
{
	Bitmap *bitmap=malloc(sizeof(Bitmap)+SizeOfBitmapDataWithWidthAndHeight(width,height));
	if(!bitmap) return NULL;
	InitialiseBitmap(bitmap,width,height,BytesPerRowForWidth(width),(Pixel *)&bitmap[1]);
	return bitmap;
}

static Bitmap *AllocateSubBitmapAsReference(Bitmap *bitmap,int x,int y,int width,int height)
{
	Bitmap *subbitmap=malloc(sizeof(Bitmap));
	if(!subbitmap) return NULL;
	InitialiseSubBitmap(subbitmap,bitmap,x,y,width,height);
	return subbitmap;
}

static Bitmap *AllocateSubBitmapAsCopy(const Bitmap *bitmap,int x,int y,int width,int height)
{
	Bitmap *subbitmap=AllocateBitmap(width,height);
	if(!subbitmap) return NULL;
	DrawBitmap(subbitmap,bitmap,-x,-y);
	return subbitmap;
}

static inline void FreeBitmap(Bitmap *self)
{
	free(self);
}

#endif
