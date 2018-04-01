#ifndef __DEFLATE_READER_H__
#define __DEFLATE_READER_H__

#include <stdint.h>
#include <stdbool.h>

#include "BitStreamReader.h"

typedef struct DeflateReader DeflateReader;

typedef void DeflateInputFunction(DeflateReader *self,void *context);

struct DeflateReader
{
	BitStreamReader bitstream;

	uint32_t pos;

	bool isstoredblock,islastblock;
	int storedbytesleft;

	uint8_t window[32768];
	uint16_t literalcode[32768],distancecode[32768];
};




static void InitialiseDeflateReader(DeflateReader *self);
static void SetDeflateInputCallback(DeflateReader *self,DeflateInputFunction *callback,void *context);
static void ProvideDeflateInputBuffer(DeflateReader *self,const void *bytes,size_t length);

static bool ReadDeflateStreamToPosition(DeflateReader *self,uint32_t pos);
static bool ReadDeflateCode(DeflateReader *self);
static bool ReadDeflateBlockHeader(DeflateReader *self);
static bool ReadDeflateMetaCode(DeflateReader *self,uint16_t *table,int size);

static void InitialiseDeflateFixedLiteralCode(uint16_t *table);
static void InitialiseDeflateFixedDistanceCode(uint16_t *table);

static void OutputDeflateLiteral(DeflateReader *self,uint8_t literal);
static void OutputDeflateMatch(DeflateReader *self,int offset,int length);

static uint32_t DeflateStreamPosition(DeflateReader *self);
static uint8_t ByteFromDeflateWindow(DeflateReader *self,uint32_t pos);
static void CopyBytesFromDeflateWindow(DeflateReader *self,void *destination,uint32_t pos,uint32_t length);




static void InitialiseDeflateReader(DeflateReader *self)
{
	InitialiseBitStreamReader(&self->bitstream);

	self->pos=0;
	self->isstoredblock=true; // This will trigger a header read.
	self->islastblock=false;
	self->storedbytesleft=0;
}

static void SetDeflateInputCallback(DeflateReader *self,DeflateInputFunction *callback,void *context)
{
	SetBitStreamInputCallback(&self->bitstream,(BitStreamInputFunction *)callback,context);
}

static void ProvideDeflateInputBuffer(DeflateReader *self,const void *bytes,size_t length)
{
	ProvideBitStreamInputBuffer(&self->bitstream,bytes,length);
}




static bool ReadDeflateStreamToPosition(DeflateReader *self,uint32_t pos)
{
	while(DeflateStreamPosition(self)<pos) if(!ReadDeflateCode(self)) return false;
	return true;
}

static bool ReadDeflateCode(DeflateReader *self)
{
	if(self->isstoredblock)
	{
		if(self->storedbytesleft>0)
		{
			self->storedbytesleft--;
			OutputDeflateLiteral(self,ReadBitString(&self->bitstream,8)); // Could be faster.
		}
		else
		{
			if(self->islastblock) return false;
			if(!ReadDeflateBlockHeader(self)) return false;
		}
	}
	else
	{
		int literal=ReadHuffmanCode(&self->bitstream,self->literalcode,15);
		if(literal<0) return false;

		if(literal<256)
		{
			OutputDeflateLiteral(self,literal);
		}
		else if(literal==256)
		{
			if(self->islastblock) return false; // Should set a flag to block future calls?
			if(!ReadDeflateBlockHeader(self)) return false; // Also here.
		}
		else
		{
			int length;
			if(literal<265) length=literal-254;
			else if(literal<285)
			{
				static const int baselengths[]={11,13,15,17,19,23,27,31,35,43,51,59,67,83,99,115,131,163,195,227};
				int size=(literal-261)/4;
				length=baselengths[literal-265]+ReadBitString(&self->bitstream,size);
			}
			else // literal==285
			{
				length=258;
			}

			int distcode=ReadHuffmanCode(&self->bitstream,self->distancecode,15);
			if(distcode<0) return false;

			int offset;
			if(distcode<4) offset=distcode+1;
			else
			{
				static const int baseoffsets[]={5,7,9,13,17,25,33,49,65,97,129,193,257,
				385,513,769,1025,1537,2049,3073,4097,6145,8193,12289,16385,24577,32769,49153};
				int size=(distcode-2)/2;
				offset=baseoffsets[distcode-4]+ReadBitString(&self->bitstream,size);
			}

			OutputDeflateMatch(self,offset,length);
		}
	}

	return true;
}

static bool ReadDeflateBlockHeader(DeflateReader *self)
{
	self->islastblock=ReadBitString(&self->bitstream,1);

	int type=ReadBitString(&self->bitstream,2);
	if(type<0) return false;

	switch(type)
	{
		case 0: // Stored.
		{
			FlushBitStream(&self->bitstream);

			int count=ReadBitString(&self->bitstream,16);
			int count2=ReadBitString(&self->bitstream,16);
			if(count!=(count2^0xffff)) return false;

			self->storedbytesleft=count;
			self->isstoredblock=true;
		}
		break;

		case 1: // Fixed Huffman.
			InitialiseDeflateFixedLiteralCode(self->literalcode);
			InitialiseDeflateFixedDistanceCode(self->distancecode);
			self->isstoredblock=false;
		break;

		case 2: // Dynamic Huffman.
		{
			int numliterals=ReadBitString(&self->bitstream,5)+257;
			int numdistances=ReadBitString(&self->bitstream,5)+1;
			int nummetas=ReadBitString(&self->bitstream,4)+4;

			uint16_t metacode[128];
			if(!ReadDeflateMetaCode(self,metacode,nummetas)) return false;

			int total=numliterals+numdistances;
			int lengths[total];
			int i=0;
			while(i<total)
			{
				int val=ReadHuffmanCode(&self->bitstream,metacode,7);
				if(val<0) return false;

				if(val<16)
				{
					lengths[i++]=val;
				}
				else if(val==16)
				{
					int repeats=ReadBitString(&self->bitstream,2)+3;

					if(i==0||i+repeats>total) return false;

					for(int j=0;j<repeats;j++) lengths[i+j]=lengths[i-1];
					i+=repeats;
				}
				else
				{
					int repeats;
					if(val==17) repeats=ReadBitString(&self->bitstream,3)+3;
					else repeats=ReadBitString(&self->bitstream,7)+11;

					if(i+repeats>total) return false;

					for(int j=0;j<repeats;j++) lengths[i+j]=0;
					i+=repeats;
				}
			}

			if(!BuildHuffmanTableFromLengths(self->literalcode,15,lengths,numliterals)) return false;
			if(!BuildHuffmanTableFromLengths(self->distancecode,15,lengths+numliterals,numdistances)) return false;

			self->isstoredblock=false;
		}
		break;

		default: return false;
	}

	return true;
}

static bool ReadDeflateMetaCode(DeflateReader *self,uint16_t *table,int size)
{
	static const int order[19]={16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15};
	int lengths[19];

	for(int i=0;i<size;i++) lengths[order[i]]=ReadBitString(&self->bitstream,3);
	for(int i=size;i<19;i++) lengths[order[i]]=0;

	return BuildHuffmanTableFromLengths(table,7,lengths,19);
}

static void InitialiseDeflateFixedLiteralCode(uint16_t *table)
{
	int lengths[288];
	for(int i=0;i<144;i++) lengths[i]=8;
	for(int i=144;i<256;i++) lengths[i]=9;
	for(int i=256;i<280;i++) lengths[i]=7;
	for(int i=280;i<288;i++) lengths[i]=8;

	BuildHuffmanTableFromLengths(table,15,lengths,288);
}

static void InitialiseDeflateFixedDistanceCode(uint16_t *table)
{
	int lengths[32];
	for(int i=0;i<32;i++) lengths[i]=5;

	BuildHuffmanTableFromLengths(table,15,lengths,32);
}




static void OutputDeflateLiteral(DeflateReader *self,uint8_t literal)
{
	self->window[self->pos&0x7fff]=literal;
	self->pos++;
}

static void OutputDeflateMatch(DeflateReader *self,int offset,int length)
{
	for(int i=0;i<length;i++) self->window[(self->pos+i)&0x7fff]=self->window[(self->pos-offset+i)&0x7fff];
	self->pos+=length;
}




static inline uint32_t DeflateStreamPosition(DeflateReader *self)
{
	return self->pos;
}

static inline uint8_t ByteFromDeflateWindow(DeflateReader *self,uint32_t pos)
{
	return self->window[pos&0x7fff];
}

static void CopyBytesFromDeflateWindow(DeflateReader *self,void *destination,uint32_t pos,uint32_t length)
{
	uint32_t start=pos&0x7fff;

	if(start+length>0x8000)
	{
		uint32_t firstlength=0x8000-start;
		memcpy(destination,&self->window[start],firstlength);
		memcpy((uint8_t *)destination+firstlength,&self->window[0],length-firstlength);
	}
	else
	{
		memcpy(destination,&self->window[start],length);
	}
}

#endif

