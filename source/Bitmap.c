#include "Bitmap.h"
#include "Clipping.h"

#include <string.h>

static void FillBitmapRow(Bitmap *self,int y,Pixel c);
static void FillPartialBitmapRow(Bitmap *self,int x,int y,int width,Pixel c);
static void ScrollBitmapRow(Bitmap *self,int dx,int desty,int srcy);
static void ScrollAndFillBitmapRow(Bitmap *self,int dx,int desty,int srcy,Pixel c);

void InitialiseBitmap(Bitmap *self,int width,int height,int bytesperrow,Pixel *pixels)
{
	self->width=width;
	self->height=height;
	self->bytesperrow=bytesperrow;
	self->pixels=pixels;
}

void InitialiseSubBitmap(Bitmap *self,Bitmap *parent,int x,int y,int width,int height)
{
	if(ClipRectangle(&x,&y,&width,&height,parent->width,parent->height))
	{
		InitialiseBitmap(self,width,height,parent->bytesperrow,
		BitmapPixelPointer(parent,x,y));
	}
	else
	{
		InitialiseBitmap(self,0,0,0,NULL);
	}
}

void ClearBitmap(Bitmap *self)
{
	memset(self->pixels,0,self->bytesperrow*self->height);
}

void FillBitmap(Bitmap *self,Pixel c)
{
	for(int y=0;y<self->height;y++) FillBitmapRow(self,y,c);
}

void ScrollBitmap(Bitmap *self,int dx,int dy)
{
	if(dx==0 && dy==0) return;
	if(dx>=self->width || dx<=-self->width) return;
	if(dy>=self->height || dy<=-self->height) return;

	if(dx==0)
	{
		if(dy<=0) memmove(BitmapPixelPointer(self,0,0),BitmapPixelPointer(self,0,-dy),(self->height+dy)*self->bytesperrow);
		else memmove(BitmapPixelPointer(self,0,dy),BitmapPixelPointer(self,0,0),(self->height-dy)*self->bytesperrow);
	}
	else
	{
		if(dy<=0) for(int y=0;y<self->height+dy;y++) ScrollBitmapRow(self,dx,y,y-dy);
		else for(int y=self->height-1;y>=dy;y--) ScrollBitmapRow(self,dx,y,y-dy);
	}
}

void ScrollAndFillBitmap(Bitmap *self,int dx,int dy,Pixel c)
{
	if(dx==0 && dy==0) return;
	if(dx>=self->width || dx<=-self->width) { FillBitmap(self,c); return; }
	if(dy>=self->height || dy<=-self->height) { FillBitmap(self,c); return; }

	if(dy>0) for(int y=0;y<dy;y++) FillBitmapRow(self,y,c);

	if(dx==0)
	{
		if(dy<=0) memmove(BitmapPixelPointer(self,0,0),BitmapPixelPointer(self,0,-dy),(self->height+dy)*self->bytesperrow);
		else memmove(BitmapPixelPointer(self,0,dy),BitmapPixelPointer(self,0,0),(self->height-dy)*self->bytesperrow);
	}
	else
	{
		if(dy<=0) for(int y=0;y<self->height+dy;y++) ScrollAndFillBitmapRow(self,dx,y,y-dy,c);
		else for(int y=self->height-1;y>=dy;y--) ScrollAndFillBitmapRow(self,dx,y,y-dy,c);
	}

	if(dy<=0) for(int y=self->height+dy;y<self->height;y++) FillBitmapRow(self,y,c);
}

void _FillPartialBitmapRow(Bitmap *self,int x,int y,int width,Pixel c)
{
	FillPartialBitmapRow(self,x,y,width,c);
}




static void FillBitmapRow(Bitmap *self,int y,Pixel c)
{
	FillPartialBitmapRow(self,0,y,self->width,c);
}

static void FillPartialBitmapRow(Bitmap *self,int x,int y,int width,Pixel c)
{
	if(sizeof(Pixel)==4)
	{
		uint32_t *row=(uint32_t *)BitmapPixelPointer(self,x,y);
		for(int x=0;x<width;x++) row[x]=c;
	}
	else if(sizeof(Pixel)==2)
	{
		uint32_t *row;
		if(x&1)
		{
			uint16_t *row16=(uint16_t *)BitmapPixelPointer(self,x,y);
			*row16=c;

			row=(uint32_t *)(row16+1);
			width--;
		}
		else
		{
			row=(uint32_t *)BitmapPixelPointer(self,x,y);
		}

		for(int x=0;x<width/2;x++) row[x]=((uint32_t)c<<16)|c;

		if(width&1)
		{
			uint16_t *alignedrow16=(uint16_t *)row;
			alignedrow16[width-1]=c;
		}
	}
	else if(sizeof(Pixel)==1)
	{
		uint32_t *row=(uint32_t *)BitmapPixelPointer(self,x,y);
		memset(row,c,width);
	}
}

static void ScrollBitmapRow(Bitmap *self,int dx,int desty,int srcy)
{
	if(dx<=0) memmove(BitmapPixelPointer(self,0,desty),BitmapPixelPointer(self,-dx,srcy),(self->width+dx)*sizeof(Pixel));
	else memmove(BitmapPixelPointer(self,dx,desty),BitmapPixelPointer(self,0,srcy),(self->width-dx)*sizeof(Pixel));
}

static void ScrollAndFillBitmapRow(Bitmap *self,int dx,int desty,int srcy,Pixel c)
{
	if(dx>0) FillPartialBitmapRow(self,0,desty,dx,c);
	ScrollBitmapRow(self,dx,desty,srcy);
	if(dx<=0) FillPartialBitmapRow(self,self->width+dx,desty,-dx,c);
}

