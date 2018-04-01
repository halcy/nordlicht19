#ifndef __BITSTREAM_READER_H__
#define __BITSTREAM_READER_H__

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

typedef struct BitStreamReader BitStreamReader;

typedef void BitStreamInputFunction(BitStreamReader *self,void *context);

struct BitStreamReader
{
	BitStreamInputFunction *callback;
	void *context;

	const uint8_t *pos,*end;

	uint32_t bits;
	int numbits;
};

static void FillBits(BitStreamReader *self,int required);
static void DiscardBits(BitStreamReader *self,int length);

static void InitialiseBitStreamReader(BitStreamReader *self)
{
	self->callback=NULL;
	self->context=NULL;
	self->pos=NULL;
	self->end=NULL;
	self->bits=0;
	self->numbits=0;
}

static void SetBitStreamInputCallback(BitStreamReader *self,BitStreamInputFunction *callback,void *context)
{
	self->callback=callback;
	self->context=context;
}

static void ProvideBitStreamInputBuffer(BitStreamReader *self,const void *bytes,size_t length)
{
	self->pos=bytes;
	self->end=self->pos+length;
}

static int ReadBitString(BitStreamReader *self,int length)
{
	if(length==0) return 0;

	FillBits(self,length);
	if(self->numbits<length) return -1;

	uint32_t bitstring=self->bits&((1<<length)-1);

	DiscardBits(self,length);

	return bitstring;
}

static void FlushBitStream(BitStreamReader *self)
{
	int extrabits=self->numbits&7;
	if(extrabits==0) return;

	DiscardBits(self,extrabits);
}

static void FillBits(BitStreamReader *self,int required)
{
	while(self->numbits<required)
	{
		if(self->pos>=self->end)
		{
			if(self->callback) self->callback(self,self->context);
			else return;

			if(self->pos>=self->end) return;
		}

		uint32_t b=*self->pos++;

		self->bits|=b<<self->numbits;
		self->numbits+=8;
	}
}

static void DiscardBits(BitStreamReader *self,int length)
{
	self->bits>>=length;
	self->numbits-=length;
}




static inline uint16_t MakeHuffmanTableEntry(int value,int length) { return (length<<12)|value; }
static inline int ValueOfHuffmanTableEntry(uint16_t entry) { return entry&0xfff; }
static inline int LengthOfHuffmanTableEntry(uint16_t entry) { return entry>>12; }

static void InitialiseHuffmanTable(uint16_t *table,int maxlength)
{
	memset(table,0,sizeof(*table)*(1<<maxlength));
}

static bool AddHuffmanCodeToTable(uint16_t *table,int maxlength,uint16_t code,int length,int value)
{
	for(int i=0;i<1<<(maxlength-length);i++)
	{
		if(table[code+(i<<length)]) return false;
		table[code+(i<<length)]=MakeHuffmanTableEntry(value,length);
		//table[(code<<maxlength-length)+i]=MakeHuffmanTableEntry(value,length);
	}
	return true;
}

static uint16_t ReverseBits(uint16_t val)
{
	val=((val>>1)&0x5555)|((val&0x5555)<<1);
	val=((val>>2)&0x3333)|((val&0x3333)<<2);
	val=((val>>4)&0x0F0F)|((val&0x0F0F)<<4);
	return (val>>8)|(val<<8);
}

static bool BuildHuffmanTableFromLengths(uint16_t *table,int maxlength,int *lengths,int numcodes)
{
	InitialiseHuffmanTable(table,maxlength);

	uint32_t code=0;
	for(int len=1;len<=maxlength;len++)
	{
		for(int i=0;i<numcodes;i++)
		{
			if(lengths[i]!=len) continue;

			if(!AddHuffmanCodeToTable(table,maxlength,ReverseBits(code),len,i)) return false;
			code+=1<<(16-len);
		}
	}
	return true;
}

static int ReadHuffmanCode(BitStreamReader *self,uint16_t *table,int maxlength)
{
	FillBits(self,maxlength);

	int index=self->bits&((1<<maxlength)-1);
	uint16_t entry=table[index];
	int value=ValueOfHuffmanTableEntry(entry);
	int length=LengthOfHuffmanTableEntry(entry);

	if(self->numbits<length) return -1;
	if(length==0) return -2;

	DiscardBits(self,length);

	return value;
}

#endif

