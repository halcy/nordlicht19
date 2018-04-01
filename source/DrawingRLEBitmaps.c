#include "RLEBitmap.h"
#include "Clipping.h"

#include <string.h>

static void DrawRLEBitmapSpans(Bitmap *bitmap,RLEBitmapSpanIterator *iterator,int x0,int y0);
static void DrawRLEBitmapSpansNoClip(Bitmap *bitmap,RLEBitmapSpanIterator *iterator,int x0,int y0);
static void CompositeRLEBitmapSpans(Bitmap *bitmap,RLEBitmapSpanIterator *iterator,int x0,int y0,CompositionMode mode);
static void CompositeRLEBitmapSpansNoClip(Bitmap *bitmap,RLEBitmapSpanIterator *iterator,int x0,int y0,CompositionMode mode);

void DrawRLEBitmap(Bitmap *bitmap,const RLEBitmap *src,int x,int y)
{
	if(IsCompletelyOutside(x,y,src->width,src->height,bitmap->width,bitmap->height)) return;

	RLEBitmapSpanIterator iterator;
	InitialiseRLEBitmapSpanIterator(&iterator,src);
	ClipRLEBitmapSpanIteratorRows(&iterator,bitmap,y);

	if(IsSpanCompletelyInside(x,src->width,bitmap->width)) DrawRLEBitmapSpansNoClip(bitmap,&iterator,x,y);
	else DrawRLEBitmapSpans(bitmap,&iterator,x,y);
}

void DrawRLEBitmapNoClip(Bitmap *bitmap,const RLEBitmap *src,int x,int y)
{
	RLEBitmapSpanIterator iterator;
	InitialiseRLEBitmapSpanIterator(&iterator,src);
	DrawRLEBitmapSpansNoClip(bitmap,&iterator,x,y);
}

void CompositeRLEBitmap(Bitmap *bitmap,const RLEBitmap *src,int x,int y,CompositionMode mode)
{
	if(IsCompletelyOutside(x,y,src->width,src->height,bitmap->width,bitmap->height)) return;

	RLEBitmapSpanIterator iterator;
	InitialiseRLEBitmapSpanIterator(&iterator,src);
	ClipRLEBitmapSpanIteratorRows(&iterator,bitmap,y);

	if(IsSpanCompletelyInside(x,src->width,bitmap->width)) CompositeRLEBitmapSpansNoClip(bitmap,&iterator,x,y,mode);
	else CompositeRLEBitmapSpans(bitmap,&iterator,x,y,mode);
}

void CompositeRLEBitmapNoClip(Bitmap *bitmap,const RLEBitmap *src,int x,int y,CompositionMode mode)
{
	RLEBitmapSpanIterator iterator;
	InitialiseRLEBitmapSpanIterator(&iterator,src);
	CompositeRLEBitmapSpansNoClip(bitmap,&iterator,x,y,mode);
}




static void DrawFilledRLEBitmapSpans(Bitmap *bitmap,RLEBitmapSpanIterator *iterator,int x0,int y0,Pixel c);
static void DrawFilledRLEBitmapSpansNoClip(Bitmap *bitmap,RLEBitmapSpanIterator *iterator,int x0,int y0,Pixel c);
static void CompositeFilledRLEBitmapSpans(Bitmap *bitmap,RLEBitmapSpanIterator *iterator,int x0,int y0,Pixel c,CompositionMode mode);
static void CompositeFilledRLEBitmapSpansNoClip(Bitmap *bitmap,RLEBitmapSpanIterator *iterator,int x0,int y0,Pixel c,CompositionMode mode);

void DrawFilledRLEBitmap(Bitmap *bitmap,const RLEBitmap *src,int x,int y,Pixel c)
{
	if(IsCompletelyOutside(x,y,src->width,src->height,bitmap->width,bitmap->height)) return;

	RLEBitmapSpanIterator iterator;
	InitialiseRLEBitmapSpanIterator(&iterator,src);
	ClipRLEBitmapSpanIteratorRows(&iterator,bitmap,y);

	if(IsSpanCompletelyInside(x,src->width,bitmap->width)) DrawFilledRLEBitmapSpansNoClip(bitmap,&iterator,x,y,c);
	else DrawFilledRLEBitmapSpans(bitmap,&iterator,x,y,c);
}

void DrawFilledRLEBitmapNoClip(Bitmap *bitmap,const RLEBitmap *src,int x,int y,Pixel c)
{
	RLEBitmapSpanIterator iterator;
	InitialiseRLEBitmapSpanIterator(&iterator,src);
	DrawFilledRLEBitmapSpansNoClip(bitmap,&iterator,x,y,c);
}

void CompositeFilledRLEBitmap(Bitmap *bitmap,const RLEBitmap *src,int x,int y,Pixel c,CompositionMode mode)
{
	if(IsCompletelyOutside(x,y,src->width,src->height,bitmap->width,bitmap->height)) return;

	RLEBitmapSpanIterator iterator;
	InitialiseRLEBitmapSpanIterator(&iterator,src);
	ClipRLEBitmapSpanIteratorRows(&iterator,bitmap,y);

	if(IsSpanCompletelyInside(x,src->width,bitmap->width)) CompositeFilledRLEBitmapSpansNoClip(bitmap,&iterator,x,y,c,mode);
	else CompositeFilledRLEBitmapSpans(bitmap,&iterator,x,y,c,mode);
}

void CompositeFilledRLEBitmapNoClip(Bitmap *bitmap,const RLEBitmap *src,int x,int y,Pixel c,CompositionMode mode)
{
	RLEBitmapSpanIterator iterator;
	InitialiseRLEBitmapSpanIterator(&iterator,src);
	CompositeFilledRLEBitmapSpansNoClip(bitmap,&iterator,x,y,c,mode);
}





static void DrawRLEBitmapSpans(Bitmap *bitmap,RLEBitmapSpanIterator *iterator,int x0,int y0)
{
	while(NextRLEBitmapSpan(iterator))
	{
		int x=RLEBitmapSpanStart(iterator);
		int y=RLEBitmapRow(iterator);
		int length=RLEBitmapSpanLength(iterator);
		const Pixel *pixels=RLEBitmapSpanPixels(iterator);
		Pixel *dest=BitmapPixelPointer(bitmap,x+x0,y+y0);

		int start=0;
		if(x+x0+length>bitmap->width) length=bitmap->width-(x+x0);

		if(x+x0<0)
		{
			int skippixels=-(x+x0);
			start=skippixels;
			length-=skippixels;
		}

		if(length>0) memcpy(&dest[start],&pixels[start],length*sizeof(Pixel));
	}
}

static void DrawRLEBitmapSpansNoClip(Bitmap *bitmap,RLEBitmapSpanIterator *iterator,int x0,int y0)
{
	while(NextRLEBitmapSpan(iterator))
	{
		int x=RLEBitmapSpanStart(iterator);
		int y=RLEBitmapRow(iterator);
		int length=RLEBitmapSpanLength(iterator);
		const Pixel *pixels=RLEBitmapSpanPixels(iterator);
		Pixel *dest=BitmapPixelPointer(bitmap,x+x0,y+y0);

		memcpy(dest,pixels,length*sizeof(Pixel));
	}
}

static void CompositeRLEBitmapSpans(Bitmap *bitmap,RLEBitmapSpanIterator *iterator,int x0,int y0,CompositionMode mode)
{
	while(NextRLEBitmapSpan(iterator))
	{
		int x=RLEBitmapSpanStart(iterator);
		int y=RLEBitmapRow(iterator);
		int length=RLEBitmapSpanLength(iterator);
		const Pixel *pixels=RLEBitmapSpanPixels(iterator);

		// TODO: Is it worth checking the span and using NoClip?
		for(int i=0;i<length;i++) CompositePixel(bitmap,x+x0+i,y+y0,pixels[i],mode);
	}
}

static void CompositeRLEBitmapSpansNoClip(Bitmap *bitmap,RLEBitmapSpanIterator *iterator,int x0,int y0,CompositionMode mode)
{
	while(NextRLEBitmapSpan(iterator))
	{
		int x=RLEBitmapSpanStart(iterator);
		int y=RLEBitmapRow(iterator);
		int length=RLEBitmapSpanLength(iterator);
		const Pixel *pixels=RLEBitmapSpanPixels(iterator);

		for(int i=0;i<length;i++) CompositePixelNoClip(bitmap,x+x0+i,y+y0,pixels[i],mode);
	}
}

static void DrawFilledRLEBitmapSpans(Bitmap *bitmap,RLEBitmapSpanIterator *iterator,int x0,int y0,Pixel c)
{
	while(NextRLEBitmapSpan(iterator))
	{
		int x=RLEBitmapSpanStart(iterator);
		int y=RLEBitmapRow(iterator);
		int length=RLEBitmapSpanLength(iterator);

		DrawHorizontalLine(bitmap,x+x0,y+y0,length,c);
	}
}

static void DrawFilledRLEBitmapSpansNoClip(Bitmap *bitmap,RLEBitmapSpanIterator *iterator,int x0,int y0,Pixel c)
{
	while(NextRLEBitmapSpan(iterator))
	{
		int x=RLEBitmapSpanStart(iterator);
		int y=RLEBitmapRow(iterator);
		int length=RLEBitmapSpanLength(iterator);

		DrawHorizontalLineNoClip(bitmap,x+x0,y+y0,length,c);
	}
}

static void CompositeFilledRLEBitmapSpans(Bitmap *bitmap,RLEBitmapSpanIterator *iterator,int x0,int y0,Pixel c,CompositionMode mode)
{
	while(NextRLEBitmapSpan(iterator))
	{
		int x=RLEBitmapSpanStart(iterator);
		int y=RLEBitmapRow(iterator);
		int length=RLEBitmapSpanLength(iterator);

		CompositeHorizontalLine(bitmap,x+x0,y+y0,length,c,mode);
	}
}

static void CompositeFilledRLEBitmapSpansNoClip(Bitmap *bitmap,RLEBitmapSpanIterator *iterator,int x0,int y0,Pixel c,CompositionMode mode)
{
	while(NextRLEBitmapSpan(iterator))
	{
		int x=RLEBitmapSpanStart(iterator);
		int y=RLEBitmapRow(iterator);
		int length=RLEBitmapSpanLength(iterator);

		CompositeHorizontalLineNoClip(bitmap,x+x0,y+y0,length,c,mode);
	}
}
