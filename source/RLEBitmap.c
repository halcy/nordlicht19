#include "RLEBitmap.h"
#include "Drawing.h"

#include <stdbool.h>
#include <string.h>

size_t SizeOfRLEBitmapFromBitmap(const Bitmap *bitmap)
{
	return SizeOfRLEBitmapFromPartialBitmap(bitmap,0,0,bitmap->width,bitmap->height);
}

size_t SizeOfRLEBitmapFromPartialBitmap(const Bitmap *bitmap,int x0,int y0,int width,int height)
{
	size_t numpixels=0;

	for(int y=0;y<height;y++)
	{
		int x=0;
		while(x<width)
		{
			int emptystart=x;
			while(x<width && x-emptystart<0xff)
			{
				Pixel p=ReadPixel(bitmap,x+x0,y+y0);
				if(!IsPixelTransparent(p)) break;
				x++;
			}
			//int empty=x-emptystart;

			int filledstart=x;
			while(x<width && x-filledstart<0xff)
			{
				Pixel p=ReadPixel(bitmap,x+x0,y+y0);
				if(IsPixelTransparent(p)) break;
				x++;
			}
			int filled=x-filledstart;

			numpixels+=1+filled;

			#ifdef SingleBytePixels
			if(x<width || filled!=0) numpixels++;
			#endif
		}
	}

	return sizeof(RLEBitmap)+numpixels*sizeof(Pixel);
}

void InitialiseRLEBitmapFromBitmap(RLEBitmap *self,const Bitmap *bitmap)
{
	InitialiseRLEBitmapFromPartialBitmap(self,bitmap,0,0,bitmap->width,bitmap->height);
}

void InitialiseRLEBitmapFromPartialBitmap(RLEBitmap *self,const Bitmap *bitmap,int x0,int y0,int width,int height)
{
	self->width=width;
	self->height=height;
	Pixel *ptr=self->codes;

	for(int y=0;y<height;y++)
	{
		int x=0;
		while(x<width)
		{
			int emptystart=x;
			while(x<width && x-emptystart<0xff)
			{
				Pixel p=ReadPixel(bitmap,x+x0,y+y0);
				if(!IsPixelTransparent(p)) break;
				x++;
			}
			int empty=x-emptystart;

			int filledstart=x;
			while(x<width && x-filledstart<0xff)
			{
				Pixel p=ReadPixel(bitmap,x+x0,y+y0);
				if(IsPixelTransparent(p)) break;
				x++;
			}
			int filled=x-filledstart;

			#ifdef SingleBytePixels
			*ptr++=empty;
			if(x<width || filled!=0) *ptr++=filled;
			#else
			*ptr++=RLECode(empty,filled);
			memcpy(ptr,ConstBitmapPixelPointer(bitmap,filledstart+x0,y+y0),filled*sizeof(Pixel));
			ptr+=filled;
			#endif
		}
	}
}

