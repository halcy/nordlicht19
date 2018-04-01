#ifndef __PNG_H__
#define __PNG_H__

#include "BitStreamReader.h"
#include "DeflateReader.h"

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct PNGLoader
{
	const uint8_t *bytes;
	size_t length;

	int width,height;
	int bitdepth,colourtype;
	int bytesperrow,bytesperpixel;

	const uint8_t *palettestart;
	int palettecount;

	const uint8_t *transparencystart;
	int transparencycount;

	const uint8_t *idatstart;
	uint32_t idatsize;
} PNGLoader;

typedef void *PNGPixelFunction(PNGLoader *loader,void *destination,uint8_t r,uint8_t g,uint8_t b,uint8_t a,int x,int y);

static void InitialisePNGLoader(PNGLoader *self,const void *bytes,size_t length);
#define InitializePNGLoader InitialisePNGLoader
static bool LoadPNGHeader(PNGLoader *self);
static bool LoadPNGImageData(PNGLoader *self,void *pixels,int bytesperrow,PNGPixelFunction *pixelfunc);
static bool LoadPNGImageDataWithTemporaryStorage(PNGLoader *self,void *pixels,int bytesperrow,PNGPixelFunction *pixelfunc,void *temporarystorage);
static void PNGInputCallback(DeflateReader *reader,void *context);

static inline uint32_t PNGGetUInt32BE(const uint8_t *ptr) { return (ptr[0]<<24)|(ptr[1]<<16)|(ptr[2]<<8)|ptr[3]; }
static inline int PNGAbs(int val) { return val<0?-val:val; }
static inline size_t TemporaryBytesForPNGLoader(PNGLoader *self)
{
	return sizeof(DeflateReader)+(self->bytesperrow+self->bytesperpixel)*2;
}

#define PNGMakeID(c1,c2,c3,c4) (((c1)<<24)|((c2)<<16)|((c3)<<8)|(c4))

static void InitialisePNGLoader(PNGLoader *self,const void *bytes,size_t length)
{
	self->bytes=bytes;
	self->length=length;
	self->width=0;
	self->height=0;

	self->bitdepth=0;
	self->colourtype=0;
	self->bytesperrow=0;
	self->bytesperpixel=0;

	self->palettestart=NULL;
	self->palettecount=0;

	self->transparencystart=NULL;
	self->transparencycount=0;

	self->idatstart=NULL;
	self->idatsize=0;
}

static bool LoadPNGHeader(PNGLoader *self)
{
	const uint8_t *ptr=self->bytes;
	const uint8_t *end=self->bytes+self->length;

	if(ptr+8>end) return false;
	if(*ptr++!=137) return false;
	if(*ptr++!=80) return false;
	if(*ptr++!=78) return false;
	if(*ptr++!=71) return false;
	if(*ptr++!=13) return false;
	if(*ptr++!=10) return false;
	if(*ptr++!=26) return false;
	if(*ptr++!=10) return false;

	while(ptr+12<end)
	{
		uint32_t size=PNGGetUInt32BE(&ptr[0]);
		uint32_t type=PNGGetUInt32BE(&ptr[4]);
		const uint8_t *next=ptr+size+12;
		if(next>end) return false;

		ptr+=8;

		switch(type)
		{
			case PNGMakeID('I','H','D','R'):
				if(size<13) return false;
				if(ptr[10]!=0) return false; // Compression type
				if(ptr[11]!=0) return false; // Filter method
				if(ptr[12]!=0) return false; // Interlace method

				self->width=PNGGetUInt32BE(&ptr[0]);
				self->height=PNGGetUInt32BE(&ptr[4]);
				self->bitdepth=ptr[8];
				self->colourtype=ptr[9];

				switch(self->colourtype)
				{
					case 0: // Grayscale
					case 3: // Palette
						if(self->bitdepth!=1&&self->bitdepth!=2&&
						self->bitdepth!=4&&self->bitdepth!=8) return false;

						self->bytesperrow=(self->bitdepth*self->width+7)/8;
						self->bytesperpixel=1;
					break;

					case 2: // RGB
						if(self->bitdepth!=8) return false;

						self->bytesperrow=3*self->width;
						self->bytesperpixel=3;
					break;

					case 4: // Gray+alpha
						if(self->bitdepth!=8) return false;

						self->bytesperrow=2*self->width;
						self->bytesperpixel=2;
					break;

					case 6: // RGB+alpha
						if(self->bitdepth!=8) return false;

						self->bytesperrow=4*self->width;
						self->bytesperpixel=4;
					break;

					default: return false;
				}
			break;

			case PNGMakeID('P','L','T','E'):
				self->palettestart=ptr;
				self->palettecount=size/3;
			break;

			case PNGMakeID('t','R','N','S'):
				self->transparencystart=ptr;
				self->transparencycount=size;
			break;

			case PNGMakeID('I','D','A','T'):
				self->idatstart=ptr;
				self->idatsize=size;
				return true;
			break;
		}

		ptr=next;
	}

	return false;
}

static bool LoadPNGImageData(PNGLoader *self,void *pixels,int bytesperrow,PNGPixelFunction *pixelfunc)
{
	uint8_t bytes[TemporaryBytesForPNGLoader(self)];
	return LoadPNGImageDataWithTemporaryStorage(self,pixels,bytesperrow,pixelfunc,bytes);
/*	void *bytes=malloc(TemporaryBytesForPNGLoader(self));
	bool res=LoadPNGImageDataWithTemporaryStorage(self,pixels,bytesperrow,pixelfunc,bytes);
	free(bytes);
	return res;*/
}

static bool LoadPNGImageDataWithTemporaryStorage(PNGLoader *self,void *pixels,int bytesperrow,PNGPixelFunction *pixelfunc,void *temporarystorage)
{
	DeflateReader *reader=temporarystorage;
	uint8_t *buf1=(uint8_t *)temporarystorage+sizeof(DeflateReader);
	uint8_t *buf2=buf1+self->bytesperrow+self->bytesperpixel;

	// TODO: Check first two bytes for zlib header.
	InitialiseDeflateReader(reader);
	ProvideDeflateInputBuffer(reader,self->idatstart+2,self->idatsize-2);
	SetDeflateInputCallback(reader,PNGInputCallback,self);

	memset(buf1,0,self->bytesperrow+self->bytesperpixel);
	memset(buf2,0,self->bytesperpixel);

	uint8_t *lastline=buf1+self->bytesperpixel;
	uint8_t *currline=buf2+self->bytesperpixel;

	for(int y=0;y<self->height;y++)
	{
		uint32_t linestart=y*(self->bytesperrow+1);
		uint32_t lineend=(y+1)*(self->bytesperrow+1);

		// Advance deflate stream until we can see all data, the copy data from window.
		if(!ReadDeflateStreamToPosition(reader,lineend)) return false;

		uint8_t filter=ByteFromDeflateWindow(reader,linestart);
		if(filter>4) return false;

		CopyBytesFromDeflateWindow(reader,currline,linestart+1,self->bytesperrow);

		// Undo filtering.
		for(int i=0;i<self->bytesperrow;i++)
		switch(filter)
		{
			case 0: break; // No filter.

			case 1:
			{
				int left=currline[i-self->bytesperpixel];
				currline[i]+=left;
			}
			break;

			case 2:
			{
				int above=lastline[i];
				currline[i]+=above;
			}
			break;

			case 3:
			{
				int left=currline[i-self->bytesperpixel];
				int above=lastline[i];
				currline[i]+=(left+above)/2;
			}
			break;

			case 4:
			{
				int left=currline[i-self->bytesperpixel];
				int above=lastline[i];
				int aboveleft=lastline[i-self->bytesperpixel];

				int p=left+above-aboveleft;
				int pa=PNGAbs(p-left);
				int pb=PNGAbs(p-above);
				int pc=PNGAbs(p-aboveleft);

				if(pa<=pb && pa<=pc) currline[i]+=left;
				else if(pb<=pc) currline[i]+=above;
				else currline[i]+=aboveleft;
			}
			break;
		}

		// Unpack pixels.
		void *outputline=(uint8_t *)pixels+bytesperrow*y;

		for(int x=0;x<self->width;x++)
		switch(self->colourtype)
		{
			case 0: // Grayscale
			{
				uint8_t gray=0;
				switch(self->bitdepth)
				{
					case 1: gray=((currline[x>>3]>>(x&7))&0x01)*0xff; break;
					case 2: gray=((currline[x>>2]>>(x&3))&0x03)*0x55; break;
					case 4: gray=((currline[x>>1]>>(x&1))&0x0f)*0x11; break;
					case 8: gray=currline[x]; break;
				}

				outputline=pixelfunc(self,outputline,gray,gray,gray,0xff,x,y);
			}
			break;

			case 2: // RGB
				outputline=pixelfunc(self,outputline,currline[3*x+0],currline[3*x+1],currline[3*x+2],0xff,x,y);
			break;

			case 3: // Palette
			{
				uint8_t index=0;
				switch(self->bitdepth)
				{
					case 1: index=(currline[x>>3]>>(x&7))&0x01; break;
					case 2: index=(currline[x>>2]>>(x&3))&0x03; break;
					case 4: index=(currline[x>>1]>>(x&1))&0x0f; break;
					case 8: index=currline[x]; break;
				}

				if(index>=self->palettecount) return false;
				uint8_t r=self->palettestart[3*index+0];
				uint8_t g=self->palettestart[3*index+1];
				uint8_t b=self->palettestart[3*index+2];

				uint8_t a=0xff;
				if(index<self->transparencycount) a=self->transparencystart[index];

				outputline=pixelfunc(self,outputline,r,g,b,a,x,y);
			}
			break;

			case 4: // Gray+alpha
				outputline=pixelfunc(self,outputline,currline[2*x+0],currline[2*x+0],currline[2*x+0],currline[2*x+1],x,y);
			break;

			case 6: // RGB+alpha
				outputline=pixelfunc(self,outputline,currline[4*x+0],currline[4*x+1],currline[4*x+2],currline[4*x+3],x,y);
			break;
		}

		// Swap line buffers.
		uint8_t *tmp=lastline;
		lastline=currline;
		currline=tmp;
	}

	return true;
}

static void PNGInputCallback(DeflateReader *reader,void *context)
{
	PNGLoader *self=context;
	const uint8_t *end=self->bytes+self->length;

	const uint8_t *ptr=self->idatstart+self->idatsize+4;
	if(ptr+12>end) return;

	uint32_t size=PNGGetUInt32BE(&ptr[0]);
	uint32_t type=PNGGetUInt32BE(&ptr[4]);

	if(ptr+12+size>end) return;

	if(type==PNGMakeID('I','D','A','T'))
	{
		self->idatstart=ptr+8;
		self->idatsize=size;
		ProvideDeflateInputBuffer(reader,self->idatstart,self->idatsize);
	}
}

#endif

