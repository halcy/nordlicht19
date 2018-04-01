#ifndef __HELPERS_RLE_BITMAP_LOADER_H__
#define __HELPERS_RLE_BITMAP_LOADER_H__

#include "RLEBitmapAllocator.h"
#include "BitmapLoader.h"

static RLEBitmap *AllocateRLEBitmapWithContentsOfPNGFile(const char *filename)
{
	Bitmap *bitmap=AllocateBitmapWithContentsOfPNGFile(filename);
	if(!bitmap) return NULL;

	RLEBitmap *self=AllocateRLEBitmapWithBitmap(bitmap);

	FreeBitmap(bitmap);

	return self;
}

#endif

