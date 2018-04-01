#include "Drawing.h"
#include "GenericDrawing.h"
#include "Clipping.h"

static inline void GenericDrawRectangle(Bitmap *bitmap,int x,int y,int w,int h,Pixel c,CompositionMode comp,
GenericDrawHorizontalLineFunction *hlinefunc,GenericDrawVerticalLineFunction *vlinefunc);

void DrawRectangle(Bitmap *bitmap,int x,int y,int w,int h,Pixel c)
{
	GenericDrawRectangle(bitmap,x,y,w,h,c,NULL,DrawHorizontalLineFunction,DrawVerticalLineFunction);
}

void DrawRectangleNoClip(Bitmap *bitmap,int x,int y,int w,int h,Pixel c)
{
	GenericDrawRectangle(bitmap,x,y,w,h,c,NULL,DrawHorizontalLineNoClipFunction,DrawVerticalLineNoClipFunction);
}

void CompositeRectangle(Bitmap *bitmap,int x,int y,int w,int h,Pixel c,CompositionMode comp)
{
	GenericDrawRectangle(bitmap,x,y,w,h,c,comp,CompositeHorizontalLineFunction,CompositeVerticalLineFunction);
}

void CompositeRectangleNoClip(Bitmap *bitmap,int x,int y,int w,int h,Pixel c,CompositionMode comp)
{
	GenericDrawRectangle(bitmap,x,y,w,h,c,comp,CompositeHorizontalLineNoClipFunction,CompositeVerticalLineNoClipFunction);
}

static inline void GenericDrawRectangle(Bitmap *bitmap,int x,int y,int w,int h,Pixel c,CompositionMode comp,
GenericDrawHorizontalLineFunction *hlinefunc,GenericDrawVerticalLineFunction *vlinefunc)
{
	if(w==0 || h==0) return;
	if(w==1) GenericDrawVerticalLine(bitmap,x,y,h,c,comp,vlinefunc);
	else if(h==1) GenericDrawHorizontalLine(bitmap,x,y,w,c,comp,hlinefunc);
	else
	{
		GenericDrawHorizontalLine(bitmap,x,y,w,c,comp,hlinefunc);
		GenericDrawHorizontalLine(bitmap,x,y+h-1,w,c,comp,hlinefunc);
		if(h<2) return;
		GenericDrawVerticalLine(bitmap,x,y+1,h-2,c,comp,vlinefunc);
		GenericDrawVerticalLine(bitmap,x+w-1,y+1,h-2,c,comp,vlinefunc);
	}
}




void DrawFilledRectangle(Bitmap *bitmap,int x,int y,int w,int h,Pixel c)
{
	if(ClipRectangle(&x,&y,&w,&h,bitmap->width,bitmap->height))
	DrawFilledRectangleNoClip(bitmap,x,y,w,h,c);
}

void DrawFilledRectangleNoClip(Bitmap *bitmap,int x,int y,int w,int h,Pixel c)
{
	for(int i=0;i<h;i++) DrawHorizontalLineNoClip(bitmap,x,y+i,w,c);
}

void CompositeFilledRectangle(Bitmap *bitmap,int x,int y,int w,int h,Pixel c,CompositionMode comp)
{
	if(ClipRectangle(&x,&y,&w,&h,bitmap->width,bitmap->height))
	CompositeFilledRectangleNoClip(bitmap,x,y,w,h,c,comp);
}

void CompositeFilledRectangleNoClip(Bitmap *bitmap,int x,int y,int w,int h,Pixel c,CompositionMode comp)
{
	for(int i=0;i<h;i++) CompositeHorizontalLineNoClip(bitmap,x,y+i,w,c,comp);
}

