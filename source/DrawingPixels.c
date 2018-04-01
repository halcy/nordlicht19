#include "Drawing.h"

void DrawPixel(Bitmap *bitmap,int x,int y,Pixel c)
{
	if(x<0) return;
	if(y<0) return;
	if(x>=bitmap->width) return;
	if(y>=bitmap->height) return;
	DrawPixelNoClip(bitmap,x,y,c);
}

void DrawPixelNoClip(Bitmap *bitmap,int x,int y,Pixel c)
{
	// TODO: Inline?
	Pixel *ptr=BitmapPixelPointer(bitmap,x,y);
	*ptr=c;
}

void CompositePixel(Bitmap *bitmap,int x,int y,Pixel c,CompositionMode comp)
{
	if(x<0) return;
	if(y<0) return;
	if(x>=bitmap->width) return;
	if(y>=bitmap->height) return;
	CompositePixelNoClip(bitmap,x,y,c,comp);
}

void CompositePixelNoClip(Bitmap *bitmap,int x,int y,Pixel c,CompositionMode comp)
{
	Pixel below=ReadPixelNoClip(bitmap,x,y);
	DrawPixelNoClip(bitmap,x,y,comp(below,c));
}

Pixel ReadPixel(const Bitmap *bitmap,int x,int y)
{
	if(x<0) return 0;
	if(y<0) return 0;
	if(x>=bitmap->width) return 0;
	if(y>=bitmap->height) return 0;
	return ReadPixelNoClip(bitmap,x,y);
}

Pixel ReadPixelNoClip(const Bitmap *bitmap,int x,int y)
{
	// TODO: Inline?
	const Pixel *ptr=ConstBitmapPixelPointer(bitmap,x,y);
	return *ptr;
}

