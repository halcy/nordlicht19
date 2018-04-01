#include "Drawing.h"
#include "GenericDrawing.h"

static inline void GenericDrawCircle(Bitmap *bitmap,int x,int y,int r,Pixel c,
CompositionMode comp,GenericDrawPixelFunction *pixelfunc);

void DrawCircle(Bitmap *bitmap,int x,int y,int r,Pixel c)
{
	if(x+r<0 || y+r<0 || x-r>=bitmap->width || y-r>=bitmap->height) return;
	if(x-r>=0 && y-r>=0 && x+r<bitmap->width && y+r<bitmap->height) { DrawCircleNoClip(bitmap,x,y,r,c); return; }
	GenericDrawCircle(bitmap,x,y,r,c,NULL,DrawPixelFunction);
}

void DrawCircleNoClip(Bitmap *bitmap,int x,int y,int r,Pixel c)
{
	GenericDrawCircle(bitmap,x,y,r,c,NULL,DrawPixelNoClipFunction);
}

void CompositeCircle(Bitmap *bitmap,int x,int y,int r,Pixel c,CompositionMode comp)
{
	if(x+r<0 || y+r<0 || x-r>=bitmap->width || y-r>=bitmap->height) return;
	if(x-r>=0 && y-r>=0 && x+r<bitmap->width && y+r<bitmap->height) { CompositeCircleNoClip(bitmap,x,y,r,c,comp); return; }
	GenericDrawCircle(bitmap,x,y,r,c,comp,CompositePixelFunction);
}

void CompositeCircleNoClip(Bitmap *bitmap,int x,int y,int r,Pixel c,CompositionMode comp)
{
	GenericDrawCircle(bitmap,x,y,r,c,comp,CompositePixelNoClipFunction);
}

static inline void GenericDrawCircle(Bitmap *bitmap,int x,int y,int r,Pixel c,
CompositionMode comp,GenericDrawPixelFunction *pixelfunc)
{
	if(r==0)
	{
		GenericDrawPixel(bitmap,x,y,c,comp,pixelfunc);
		return;
	}

	int error=-r;
	int dx=r;
	int dy=0;

	GenericDrawPixel(bitmap,x,y-r,c,comp,pixelfunc);
	GenericDrawPixel(bitmap,x-r,y,c,comp,pixelfunc);
	GenericDrawPixel(bitmap,x+r,y,c,comp,pixelfunc);
	GenericDrawPixel(bitmap,x,y+r,c,comp,pixelfunc);

	for(;;)
	{
		error+=2*dy+1;
		dy++;
 
		if(error>=0)
		{
			error-=2*dx-1;
			dx--;
		}
		if(dx<=dy) break;

		GenericDrawPixel(bitmap,x-dx,y-dy,c,comp,pixelfunc);
		GenericDrawPixel(bitmap,x+dx,y-dy,c,comp,pixelfunc);
		GenericDrawPixel(bitmap,x-dx,y+dy,c,comp,pixelfunc);
		GenericDrawPixel(bitmap,x+dx,y+dy,c,comp,pixelfunc);
		GenericDrawPixel(bitmap,x-dy,y-dx,c,comp,pixelfunc);
		GenericDrawPixel(bitmap,x-dy,y+dx,c,comp,pixelfunc);
		GenericDrawPixel(bitmap,x+dy,y-dx,c,comp,pixelfunc);
		GenericDrawPixel(bitmap,x+dy,y+dx,c,comp,pixelfunc);
	}

	if(dx==dy)
	{
		GenericDrawPixel(bitmap,x-dx,y-dx,c,comp,pixelfunc);
		GenericDrawPixel(bitmap,x+dx,y-dx,c,comp,pixelfunc);
		GenericDrawPixel(bitmap,x-dx,y+dx,c,comp,pixelfunc);
		GenericDrawPixel(bitmap,x+dx,y+dx,c,comp,pixelfunc);
	}
}




static inline void GenericDrawFilledCircle(Bitmap *bitmap,int x,int y,int r,Pixel c,CompositionMode comp,
GenericDrawPixelFunction *drawfunc,GenericDrawHorizontalLineFunction *hlinefunc);

void DrawFilledCircle(Bitmap *bitmap,int x,int y,int r,Pixel c)
{
	if(x-r>=0 && y-r>=0 && x+r<bitmap->width && y+r<bitmap->height) { DrawFilledCircleNoClip(bitmap,x,y,r,c); return; }
	if(x+r<0 || y+r<0 || x-r>=bitmap->width || y-r>=bitmap->height) return;
	GenericDrawFilledCircle(bitmap,x,y,r,c,NULL,DrawPixelFunction,DrawHorizontalLineFunction);
}

void DrawFilledCircleNoClip(Bitmap *bitmap,int x,int y,int r,Pixel c)
{
	GenericDrawFilledCircle(bitmap,x,y,r,c,NULL,DrawPixelNoClipFunction,DrawHorizontalLineNoClipFunction);
}

void CompositeFilledCircle(Bitmap *bitmap,int x,int y,int r,Pixel c,CompositionMode comp)
{
	if(x-r>=0 && y-r>=0 && x+r<bitmap->width && y+r<bitmap->height) { CompositeFilledCircleNoClip(bitmap,x,y,r,c,comp); return; }
	if(x+r<0 || y+r<0 || x-r>=bitmap->width || y-r>=bitmap->height) return;
	GenericDrawFilledCircle(bitmap,x,y,r,c,comp,CompositePixelFunction,CompositeHorizontalLineFunction);
}

void CompositeFilledCircleNoClip(Bitmap *bitmap,int x,int y,int r,Pixel c,CompositionMode comp)
{
	GenericDrawFilledCircle(bitmap,x,y,r,c,comp,CompositePixelNoClipFunction,CompositeHorizontalLineNoClipFunction);
}

static inline void GenericDrawFilledCircle(Bitmap *bitmap,int x,int y,int r,Pixel c,CompositionMode comp,
GenericDrawPixelFunction *drawfunc,GenericDrawHorizontalLineFunction *hlinefunc)
{
	if(r==0)
	{
		GenericDrawPixel(bitmap,x,y,c,comp,drawfunc);
		return;
	}

	int error=-r;
	int dx=r;
	int dy=0;

	GenericDrawHorizontalLine(bitmap,x-r,y,2*r+1,c,comp,hlinefunc);

	for(;;)
	{
		error+=2*dy+1;
		dy++;

		if(error>=0)
		{
			GenericDrawHorizontalLine(bitmap,x-dy+1,y-dx,2*dy-1,c,comp,hlinefunc);
			GenericDrawHorizontalLine(bitmap,x-dy+1,y+dx,2*dy-1,c,comp,hlinefunc);

			error-=2*dx-1;
			dx--;
		}

		if(dx<=dy) break;

		GenericDrawHorizontalLine(bitmap,x-dx,y-dy,2*dx+1,c,comp,hlinefunc);
		GenericDrawHorizontalLine(bitmap,x-dx,y+dy,2*dx+1,c,comp,hlinefunc);
	}

	if(dx==dy)
	{
		GenericDrawHorizontalLine(bitmap,x-dx,y-dx,2*dx+1,c,comp,hlinefunc);
		GenericDrawHorizontalLine(bitmap,x-dx,y+dx,2*dx+1,c,comp,hlinefunc);
	}
}
