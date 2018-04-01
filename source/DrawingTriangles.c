#include "Drawing.h"
#include "GenericDrawing.h"

static inline void GenericDrawTriangle(Bitmap *bitmap,int x1,int y1,int x2,int y2,int x3,int y3,
Pixel c,CompositionMode comp,GenericDrawHorizontalLineFunction *hlinefunc);

void DrawTriangle(Bitmap *bitmap,int x1,int y1,int x2,int y2,int x3,int y3,Pixel c)
{
	if(x1>=0 && y1>=0 && x1<bitmap->width && y1<bitmap->height &&
	x2>=0 && y2>=0 && x2<bitmap->width && y2<bitmap->height &&
	x3>=0 && y3>=0 && x3<bitmap->width && y3<bitmap->height)
	{
		DrawTriangleNoClip(bitmap,x1,y1,x2,y2,x3,y3,c);
		return;
	}

	if(x1<0 && x2<0 && x3<0) return;
	if(y1<0 && y2<0 && y3<0) return;
	if(x1>=bitmap->width && x2>=bitmap->width && x3>=bitmap->width) return;
	if(y1>=bitmap->height && y2>=bitmap->height && y3>=bitmap->height) return;

	GenericDrawTriangle(bitmap,x1,y1,x2,y2,x3,y3,c,NULL,DrawHorizontalLineFunction);
}

void DrawTriangleNoClip(Bitmap *bitmap,int x1,int y1,int x2,int y2,int x3,int y3,Pixel c)
{
	GenericDrawTriangle(bitmap,x1,y1,x2,y2,x3,y3,c,NULL,DrawHorizontalLineNoClipFunction);
}

void CompositeTriangle(Bitmap *bitmap,int x1,int y1,int x2,int y2,int x3,int y3,Pixel c,CompositionMode comp)
{
	if(x1>=0 && y1>=0 && x1<bitmap->width && y1<bitmap->height &&
	x2>=0 && y2>=0 && x2<bitmap->width && y2<bitmap->height &&
	x3>=0 && y3>=0 && x3<bitmap->width && y3<bitmap->height)
	{
		CompositeTriangleNoClip(bitmap,x1,y1,x2,y2,x3,y3,c,comp);
		return;
	}

	if(x1<0 && x2<0 && x3<0) return;
	if(y1<0 && y2<0 && y3<0) return;
	if(x1>=bitmap->width && x2>=bitmap->width && x3>=bitmap->width) return;
	if(y1>=bitmap->height && y2>=bitmap->height && y3>=bitmap->height) return;

	GenericDrawTriangle(bitmap,x1,y1,x2,y2,x3,y3,c,comp,CompositeHorizontalLineFunction);
}

void CompositeTriangleNoClip(Bitmap *bitmap,int x1,int y1,int x2,int y2,int x3,int y3,Pixel c,CompositionMode comp)
{
	GenericDrawTriangle(bitmap,x1,y1,x2,y2,x3,y3,c,comp,CompositeHorizontalLineNoClipFunction);
}

void DrawQuad(Bitmap *bitmap,int x1,int y1,int x2,int y2,int x3,int y3,int x4,int y4,Pixel c)
{
	DrawTriangle(bitmap,x1,y1,x2,y2,x3,y3,c);
	DrawTriangle(bitmap,x1,y1,x3,y3,x4,y4,c);
}

void DrawQuadNoClip(Bitmap *bitmap,int x1,int y1,int x2,int y2,int x3,int y3,int x4,int y4,Pixel c)
{
	DrawTriangleNoClip(bitmap,x1,y1,x2,y2,x3,y3,c);
	DrawTriangleNoClip(bitmap,x1,y1,x3,y3,x4,y4,c);
}

void CompositeQuad(Bitmap *bitmap,int x1,int y1,int x2,int y2,int x3,int y3,int x4,int y4,Pixel c,CompositionMode comp)
{
	CompositeTriangle(bitmap,x1,y1,x2,y2,x3,y3,c,comp);
	CompositeTriangle(bitmap,x1,y1,x3,y3,x4,y4,c,comp);
}

void CompositeQuadNoClip(Bitmap *bitmap,int x1,int y1,int x2,int y2,int x3,int y3,int x4,int y4,Pixel c,CompositionMode comp)
{
	CompositeTriangleNoClip(bitmap,x1,y1,x2,y2,x3,y3,c,comp);
	CompositeTriangleNoClip(bitmap,x1,y1,x3,y3,x4,y4,c,comp);
}




static inline void GenericDrawTriangleSection(Bitmap *bitmap,
int32_t *leftx,int32_t *rightx,int32_t leftdelta,int32_t rightdelta,int y1,int y2,
Pixel c,CompositionMode comp,GenericDrawHorizontalLineFunction *hlinefunc);

static inline void GenericDrawTriangle(Bitmap *bitmap,int x1,int y1,int x2,int y2,int x3,int y3,
Pixel c,CompositionMode comp,GenericDrawHorizontalLineFunction *hlinefunc)
{
	if(x1==x2 && x2==x3) return;
	if(y1==y2 && y2==y3) return;

	// Y-sort vertices.
	if(y1>y2)
	{
		int t;
		t=x1; x1=x2; x2=t;
		t=y1; y1=y2; y2=t;
	}

	if(y2>y3)
	{
		int t;
		t=x2; x2=x3; x3=t;
		t=y2; y2=y3; y3=t;
	}

	if(y1>y2)
	{
		int t;
		t=x1; x1=x2; x2=t;
		t=y1; y1=y2; y2=t;
	}

	if(y1==y2)
	{
		int32_t delta13=65536*(x3-x1)/(y3-y1);
		int32_t delta23=65536*(x3-x2)/(y3-y2);
		if(delta13>delta23)
		{
			int32_t leftx=x1<<16,rightx=x2<<16;
			GenericDrawTriangleSection(bitmap,&leftx,&rightx,delta13,delta23,y2,y3,c,comp,hlinefunc);
		}
		else
		{
			int32_t leftx=x2<<16,rightx=x1<<16;
			GenericDrawTriangleSection(bitmap,&leftx,&rightx,delta23,delta13,y2,y3,c,comp,hlinefunc);
		}
	}
	else if(y2==y3)
	{
		int32_t delta12=65536*(x2-x1)/(y2-y1);
		int32_t delta13=65536*(x3-x1)/(y3-y1);
		int32_t leftx=x1<<16,rightx=x1<<16;
		if(delta12<delta13)
		{
			GenericDrawTriangleSection(bitmap,&leftx,&rightx,delta12,delta13,y1,y2,c,comp,hlinefunc);
		}
		else
		{
			GenericDrawTriangleSection(bitmap,&leftx,&rightx,delta13,delta12,y1,y2,c,comp,hlinefunc);
		}
	}
	else
	{
		int32_t delta12=65536*(x2-x1)/(y2-y1);
		int32_t delta13=65536*(x3-x1)/(y3-y1);
		int32_t delta23=65536*(x3-x2)/(y3-y2);

		if(delta12<delta13)
		{
			int32_t leftx1=x1<<16,leftx2=x2<<16,rightx=x1<<16;
			GenericDrawTriangleSection(bitmap,&leftx1,&rightx,delta12,delta13,y1,y2,c,comp,hlinefunc);
			GenericDrawTriangleSection(bitmap,&leftx2,&rightx,delta23,delta13,y2,y3,c,comp,hlinefunc);
		}
		else
		{
			int32_t leftx=x1<<16,rightx1=x1<<16,rightx2=x2<<16;
			GenericDrawTriangleSection(bitmap,&leftx,&rightx1,delta13,delta12,y1,y2,c,comp,hlinefunc);
			GenericDrawTriangleSection(bitmap,&leftx,&rightx2,delta13,delta23,y2,y3,c,comp,hlinefunc);
		}
	}
}

static inline void GenericDrawTriangleSection(Bitmap *bitmap,
int32_t *leftx,int32_t *rightx,int32_t leftdelta,int32_t rightdelta,int y1,int y2,
Pixel c,CompositionMode comp,GenericDrawHorizontalLineFunction *hlinefunc)
{
	// TODO: Figure out subpixel positioning.
	*leftx+=leftdelta/2;
	*rightx+=rightdelta/2;
	for(int y=y1;y<y2;y++)
	{
		int x1=*leftx>>16;
		int x2=*rightx>>16;
		GenericDrawHorizontalLine(bitmap,x1,y,x2-x1,c,comp,hlinefunc);
		*leftx+=leftdelta;
		*rightx+=rightdelta;
	}
}