#include "Drawing.h"
#include "Clipping.h"

#include <string.h>

void DrawBitmap(Bitmap *dest,const Bitmap *src,int x,int y)
{
	DrawPartialBitmap(dest,src,x,y,0,0,src->width,src->height);
}

void DrawBitmapNoClip(Bitmap *dest,const Bitmap *src,int x,int y)
{
	DrawPartialBitmapNoClip(dest,src,x,y,0,0,src->width,src->height);
}

void DrawPartialBitmap(Bitmap *dest,const Bitmap *src,int x,int y,int u,int v,int w,int h)
{
	if(ClipRectangle2(&x,&y,&u,&v,&w,&h,dest->width,dest->height))
	if(ClipRectangle2(&u,&v,&x,&y,&w,&h,src->width,src->height))
	DrawPartialBitmapNoClip(dest,src,x,y,u,v,w,h);
}

void DrawPartialBitmapNoClip(Bitmap *dest,const Bitmap *src,int x,int y,int u,int v,int w,int h)
{
	if(x==0&&u==0&&w==dest->width&&src->bytesperrow==dest->bytesperrow) // Yes, dest->width.
	{
		// Fast path for full bitmap row copies.
		const void *srcptr=ConstBitmapPixelPointer(src,0,v);
		void *destptr=BitmapPixelPointer(dest,0,y);
		memmove(destptr,srcptr,src->bytesperrow*h);
	}
	else
	{
		// Normal copy.
		for(int i=0;i<h;i++)
		{
			const void *srcptr=ConstBitmapPixelPointer(src,u,v+i);
			void *destptr=BitmapPixelPointer(dest,x,y+i);
			memmove(destptr,srcptr,w*sizeof(Pixel));
		}
	}
}

void CompositeBitmap(Bitmap *dest,const Bitmap *src,int x,int y,CompositionMode comp)
{
	CompositePartialBitmap(dest,src,x,y,0,0,src->width,src->height,comp);
}

void CompositeBitmapNoClip(Bitmap *dest,const Bitmap *src,int x,int y,CompositionMode comp)
{
	CompositePartialBitmapNoClip(dest,src,x,y,0,0,src->width,src->height,comp);
}

void CompositePartialBitmap(Bitmap *dest,const Bitmap *src,int x,int y,int u,int v,int w,int h,CompositionMode comp)
{
	if(ClipRectangle2(&x,&y,&u,&v,&w,&h,dest->width,dest->height))
	if(ClipRectangle2(&u,&v,&x,&y,&w,&h,src->width,src->height))
	CompositePartialBitmapNoClip(dest,src,x,y,u,v,w,h,comp);
}

void CompositePartialBitmapNoClip(Bitmap *dest,const Bitmap *src,int x,int y,int u,int v,int w,int h,CompositionMode comp)
{
	for(int j=0;j<h;j++)
	for(int i=0;i<w;i++)
	{
		Pixel below=ReadPixelNoClip(dest,x+i,y+j);
		Pixel above=ReadPixelNoClip(src,u+i,v+j);
		DrawPixelNoClip(dest,x+i,y+j,comp(below,above));
	}
}

