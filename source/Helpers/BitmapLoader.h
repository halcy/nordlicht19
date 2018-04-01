#ifndef __HELPERS_BITMAP_LOADER_H__
#define __HELPERS_BITMAP_LOADER_H__

#include "BitmapAllocator.h"
#include "PNG/PNG.h"

#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <io.h>
#include <sys/fcntl.h>
int _fileno(FILE *fh); // Why isn't this defined? Who knows.
#endif

static void *BitmapLoaderPixelFunc(PNGLoader *loader,void *destination,uint8_t r,uint8_t g,uint8_t b,uint8_t a,int x,int y);

static Bitmap *AllocateBitmapWithContentsOfPNGFile(const char *filename)
{
	long size=0;
	uint8_t *bytes=NULL;

	if(strcmp(filename,"-")!=0)
	{
		FILE *fh=fopen(filename,"rb");
		if(!fh) return NULL;

		fseek(fh,0,SEEK_END);
		size=ftell(fh);
		fseek(fh,0,SEEK_SET);

		bytes=malloc(size);
		if(!bytes) { fclose(fh); return NULL; }

		if(fread(bytes,1,size,fh)!=size) { fclose(fh); free(bytes); return NULL; }
		fclose(fh);
	}
	else
	{
		#ifdef _WIN32
		setmode(_fileno(stdin),O_BINARY);
		#endif

		while(!feof(stdin))
		{
			char buf[65536];
			size_t actual=fread(buf,1,sizeof(buf),stdin);
			if(ferror(stdin)) { free(bytes); return NULL; }

			bytes=realloc(bytes,size+actual);
			if(!bytes) return NULL;

			memcpy(&bytes[size],buf,actual);
			size+=actual;
		}
	}

	PNGLoader loader;
	InitialisePNGLoader(&loader,bytes,size);

	if(!LoadPNGHeader(&loader)) { free(bytes); return NULL; }

	Bitmap *self=AllocateBitmap(loader.width,loader.height);
	if(!self) { free(bytes); return NULL; }

	if(!LoadPNGImageData(&loader,self->pixels,self->bytesperrow,BitmapLoaderPixelFunc))
	{ free(bytes); FreeBitmap(self); return NULL; }

	free(bytes);

	return self;
}

static void *BitmapLoaderPixelFunc(PNGLoader *loader,void *destination,uint8_t r,uint8_t g,uint8_t b,uint8_t a,int x,int y)
{
	Pixel *pixel=destination;
	*pixel=RGBA(r,g,b,a);
	return pixel+1;
}

#endif

