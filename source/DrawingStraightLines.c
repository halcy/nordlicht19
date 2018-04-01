#include "Drawing.h"
#include "Clipping.h"

void DrawHorizontalLine(Bitmap *bitmap,int x,int y,int len,Pixel c)
{
	if(ClipLine(&x,&y,&len,bitmap->width,bitmap->height))
	DrawHorizontalLineNoClip(bitmap,x,y,len,c);
}

/*void DrawHorizontalLineNoClip(Bitmap *bitmap,int x,int y,int len,Pixel c)
{
	// TODO: Specialized 32-bit (or 64-bit?) write versions.
	for(int i=0;i<len;i++) DrawPixelNoClip(bitmap,x+i,y,c);
}*/

void CompositeHorizontalLine(Bitmap *bitmap,int x,int y,int len,Pixel c,CompositionMode comp)
{
	if(ClipLine(&x,&y,&len,bitmap->width,bitmap->height))
	CompositeHorizontalLineNoClip(bitmap,x,y,len,c,comp);
}

void CompositeHorizontalLineNoClip(Bitmap *bitmap,int x,int y,int len,Pixel c,CompositionMode comp)
{
	for(int i=0;i<len;i++) DrawPixelNoClip(bitmap,x+i,y,comp(ReadPixelNoClip(bitmap,x+i,y),c));
}

void DrawVerticalLine(Bitmap *bitmap,int x,int y,int len,Pixel c)
{
	if(ClipLine(&y,&x,&len,bitmap->height,bitmap->width))
	DrawVerticalLineNoClip(bitmap,x,y,len,c);
}

void DrawVerticalLineNoClip(Bitmap *bitmap,int x,int y,int len,Pixel c)
{
	for(int i=0;i<len;i++) DrawPixelNoClip(bitmap,x,y+i,c);
}

void CompositeVerticalLine(Bitmap *bitmap,int x,int y,int len,Pixel c,CompositionMode comp)
{
	if(ClipLine(&y,&x,&len,bitmap->height,bitmap->width))
	CompositeVerticalLineNoClip(bitmap,x,y,len,c,comp);
}

void CompositeVerticalLineNoClip(Bitmap *bitmap,int x,int y,int len,Pixel c,CompositionMode comp)
{
	for(int i=0;i<len;i++) DrawPixelNoClip(bitmap,x,y+i,comp(ReadPixelNoClip(bitmap,x,y+i),c));
}

