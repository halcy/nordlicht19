#ifndef __RLE_BITMAP_H__
#define __RLE_BITMAP_H__

#include "Pixels.h"
#include "Bitmap.h"
#include "Drawing.h"

#include <stdlib.h>
#include <stdint.h>

typedef struct RLEBitmap
{
	int16_t width,height;
	Pixel codes[0];
} RLEBitmap;

typedef struct RLEBitmapSpanIterator
{
	int x,y,spanlength;
	const Pixel *pointer;
	int width,height;
} RLEBitmapSpanIterator;

#if PixelSize!=1
#define RLECode(empty,filled) ( ((empty)<<8)|(filled) )
#define RLEEndCode(empty) RLECode(empty,0)
#define RLECodeCount(pixels,codes,endcodes) ((pixels)+(codes)+(endcodes))
#else
#define RLECode(empty,filled) (empty),(filled)
#define RLEEndCode(empty) (empty)
#define RLECodeCount(pixels,codes,endcodes) ((pixels)+(codes)*2+(endcodes))
#endif

size_t SizeOfRLEBitmapFromBitmap(const Bitmap *bitmap);
size_t SizeOfRLEBitmapFromPartialBitmap(const Bitmap *bitmap,int x,int y,int width,int height);

void InitialiseRLEBitmapFromBitmap(RLEBitmap *self,const Bitmap *bitmap);
void InitialiseRLEBitmapFromPartialBitmap(RLEBitmap *self,const Bitmap *bitmap,int x,int y,int width,int height);
#define InitializeRLEBitmapFromBitmap InitialiseRLEBitmapFromBitmap
#define InitializeRLEBitmapFromPartialBitmap InitialiseRLEBitmapFromPartialBitmap

void DrawRLEBitmap(Bitmap *bitmap,const RLEBitmap *src,int x,int y);
void DrawRLEBitmapNoClip(Bitmap *bitmap,const RLEBitmap *src,int x,int y);
void CompositeRLEBitmap(Bitmap *bitmap,const RLEBitmap *src,int x,int y,CompositionMode mode);
void CompositeRLEBitmapNoClip(Bitmap *bitmap,const RLEBitmap *src,int x,int y,CompositionMode mode);

void DrawFilledRLEBitmap(Bitmap *bitmap,const RLEBitmap *src,int x,int y,Pixel c);
void DrawFilledRLEBitmapNoClip(Bitmap *bitmap,const RLEBitmap *src,int x,int y,Pixel c);
void CompositeFilledRLEBitmap(Bitmap *bitmap,const RLEBitmap *src,int x,int y,Pixel c,CompositionMode mode);
void CompositeFilledRLEBitmapNoClip(Bitmap *bitmap,const RLEBitmap *src,int x,int y,Pixel c,CompositionMode mode);

static inline bool NextRLEBitmapSpan(RLEBitmapSpanIterator *self);

static inline void InitialiseRLEBitmapSpanIterator(RLEBitmapSpanIterator *self,const RLEBitmap *rle)
{
	self->x=0;
	self->y=0;
	self->spanlength=0;
	self->pointer=rle->codes;
	self->width=rle->width;
	self->height=rle->height;
}
#define InitializeRLEBitmapSpanIterator InitialiseRLEBitmapSpanIterator

static inline int RLEBitmapRow(RLEBitmapSpanIterator *self) { return self->y; }
static inline int RLEBitmapSpanStart(RLEBitmapSpanIterator *self) { return self->x; }
static inline int RLEBitmapSpanLength(RLEBitmapSpanIterator *self) { return self->spanlength; }
static inline const Pixel *RLEBitmapSpanPixels(RLEBitmapSpanIterator *self) { return self->pointer; }

static inline void ClipRLEBitmapSpanIteratorRows(RLEBitmapSpanIterator *self,const Bitmap *bitmap,int y)
{
	if(y+self->height>bitmap->height) self->height=bitmap->height-y;

	if(y<0)
	{
		int oldheight=self->height;
		self->height=-y;
		while(NextRLEBitmapSpan(self));
		self->height=oldheight;
	}
}

#if PixelSize!=1

static inline bool NextRLEBitmapSpan(RLEBitmapSpanIterator *self)
{
	self->x+=self->spanlength;
	self->pointer+=self->spanlength;
	if(self->x>=self->width)
	{
		self->x=0;
		self->y++;
		if(self->y>=self->height) { self->spanlength=0; return false; }
	}

	Pixel spans=*self->pointer++;
	int empty=spans>>8;
	int filled=spans&0xff;

	self->x+=empty;
	self->spanlength=filled;

	if(!self->spanlength) return NextRLEBitmapSpan(self);
	return true;
}

#else

static inline bool NextRLEBitmapSpan(RLEBitmapSpanIterator *self)
{
	self->x+=self->spanlength;
	self->pointer+=self->spanlength;
	if(self->x>=self->width)
	{
		self->x=0;
		self->y++;
		if(self->y>=self->height) { self->spanlength=0; return false; }
	}

	Pixel empty=*self->pointer++;
	self->x+=empty;
	if(self->x>=self->width)
	{
		self->spanlength=0;
	}
	else
	{
		Pixel filled=*self->pointer++;
		self->spanlength=filled;
	}

	if(!self->spanlength) return NextRLEBitmapSpan(self);
	return true;
}

#endif

#endif
