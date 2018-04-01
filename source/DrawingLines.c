#include "Drawing.h"
#include "GenericDrawing.h"

static inline void GenericDrawLine(Bitmap *bitmap,int x1,int y1,int x2,int y2,Pixel c,CompositionMode comp,
GenericDrawPixelFunction *pixelfunc,GenericDrawHorizontalLineFunction *hlinefunc,GenericDrawVerticalLineFunction *vlinefunc);

void DrawLine(Bitmap *bitmap,int x1,int y1,int x2,int y2,Pixel c)
{
	if(x1>=0 && y1>=0 && x1<bitmap->width && y1<bitmap->height &&
	x2>=0 && y2>=0 && x2<bitmap->width && y2<bitmap->height)
	{
		DrawLineNoClip(bitmap,x1,y1,x2,y2,c);
		return;
	}

	if(x1<0 && x2<0) return;
	if(y1<0 && y2<0) return;
	if(x1>=bitmap->width && x2>=bitmap->width) return;
	if(y1>=bitmap->height && y2>=bitmap->height) return;

	GenericDrawLine(bitmap,x1,y1,x2,y2,c,NULL,DrawPixelFunction,DrawHorizontalLineFunction,DrawVerticalLineFunction);
}

void DrawLineNoClip(Bitmap *bitmap,int x1,int y1,int x2,int y2,Pixel c)
{
	GenericDrawLine(bitmap,x1,y1,x2,y2,c,NULL,DrawPixelNoClipFunction,DrawHorizontalLineNoClipFunction,DrawVerticalLineNoClipFunction);
}

void CompositeLine(Bitmap *bitmap,int x1,int y1,int x2,int y2,Pixel c,CompositionMode comp)
{
	if(x1>=0 && y1>=0 && x1<bitmap->width && y1<bitmap->height &&
	x2>=0 && y2>=0 && x2<bitmap->width && y2<bitmap->height)
	{
		CompositeLineNoClip(bitmap,x1,y1,x2,y2,c,comp);
		return;
	}

	if(x1<0 && x2<0) return;
	if(y1<0 && y2<0) return;
	if(x1>=bitmap->width && x2>=bitmap->width) return;
	if(y1>=bitmap->height && y2>=bitmap->height) return;

	GenericDrawLine(bitmap,x1,y1,x2,y2,c,comp,CompositePixelFunction,CompositeHorizontalLineFunction,CompositeVerticalLineFunction);
}

void CompositeLineNoClip(Bitmap *bitmap,int x1,int y1,int x2,int y2,Pixel c,CompositionMode comp)
{
	GenericDrawLine(bitmap,x1,y1,x2,y2,c,comp,CompositePixelNoClipFunction,CompositeHorizontalLineNoClipFunction,CompositeVerticalLineNoClipFunction);

}

static inline void GenericDrawLine(Bitmap *bitmap,int x1,int y1,int x2,int y2,Pixel c,CompositionMode comp,
GenericDrawPixelFunction *pixelfunc,GenericDrawHorizontalLineFunction *hlinefunc,GenericDrawVerticalLineFunction *vlinefunc)
{
	if(x1==x2)
	{
		if(y1<y2) GenericDrawVerticalLine(bitmap,x1,y1,y2-y1+1,c,comp,vlinefunc);
		else GenericDrawVerticalLine(bitmap,x1,y2,y1-y2+1,c,comp,vlinefunc);
		return;
	}
	else if(y1==y2)
	{
		if(x1<x2) GenericDrawHorizontalLine(bitmap,x1,y1,x2-x1+1,c,comp,hlinefunc);
		else GenericDrawHorizontalLine(bitmap,x2,y1,x1-x2+1,c,comp,hlinefunc);
		return;
	}

	bool steep=abs(y2-y1)>abs(x2-x1);

	if(steep)
	{
		int t;
		t=x1; x1=y1; y1=t;
		t=x2; x2=y2; y2=t;
	}

	if(x1>x2)
	{
		int t;
		t=x1; x1=x2; x2=t;
		t=y1; y1=y2; y2=t;
	}

	int deltax=x2-x1;
	int deltay,ydir;
	if(y1<y2) { deltay=y2-y1; ydir=1; }
	else { deltay=y1-y2; ydir=-1; }

	int error=deltax>>1;
	int y=y1;

	if(steep)
	{
		for(int x=x1;x<=x2;x++)
		{
			GenericDrawPixel(bitmap,y,x,c,comp,pixelfunc);
			error-=deltay;
			if(error<0)
			{
				y+=ydir;
				error+=deltax;
			}
		}
	}
	else
	{
		for(int x=x1;x<=x2;x++)
		{
			GenericDrawPixel(bitmap,x,y,c,comp,pixelfunc);
			error-=deltay;
			if(error<0)
			{
				y+=ydir;
				error+=deltax;
			}
		}
	}
}
