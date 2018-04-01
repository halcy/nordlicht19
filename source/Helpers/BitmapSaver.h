#ifndef __HELPERS_BITMAP_SAVER_H__
#define __HELPERS_BITMAP_SAVER_H__

#include "../Bitmap.h"
#include "PNG/PNGSaver.h"

#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <io.h>
#include <sys/fcntl.h>
int _fileno(FILE *fh); // Why isn't this defined? Who knows.
#endif

static bool BitmapSaverWriteFunc(void *context,uint8_t b);
static void *BitmapSaverPixelFunc(void *source,uint8_t *rgba,int x,int y);

static bool SaveBitmapToPNGFile(const Bitmap *bitmap,const char *filename)
{
	FILE *fh;

	if(strcmp(filename,"-")==0)
	{
		fh=stdout;
		#ifdef _WIN32
		setmode(_fileno(stdout),O_BINARY);
		#endif
	}
	else
	{
		fh=fopen(filename,"wb");
		if(!fh) return false;
	}

	SavePNG(BitmapSaverWriteFunc,fh,bitmap->width,bitmap->height,BitmapSaverPixelFunc,(void *)bitmap);

	if(fh!=stdout) fclose(fh);

	return true;
}

static bool BitmapSaverWriteFunc(void *context,uint8_t byte)
{
	FILE *fh=context;
	return fputc(byte,fh)!=EOF;
}

static void *BitmapSaverPixelFunc(void *source,uint8_t *rgba,int x,int y)
{
	const Bitmap *bitmap=source;
	const Pixel *ptr=ConstBitmapPixelPointer(bitmap,x,y);

	Pixel p=*ptr;
	rgba[0]=ExtractRed(p);
	rgba[1]=ExtractGreen(p);
	rgba[2]=ExtractBlue(p);
	rgba[3]=ExtractAlpha(p);

	return source;
}

#endif

