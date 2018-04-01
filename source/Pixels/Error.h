#ifndef __PIXELS_ERROR_H__
#define __PIXELS_ERROR_H__

static inline int ErrorForRed(int red)
{
	#if PixelRedBits==8
	return 0;
	#else
	return red-ExtractAndExpandBitsAt(red,8-PixelRedBits,PixelRedBits);
	#endif
}

static inline int ErrorForGreen(int green)
{
	#if PixelRedBits==8
	return 0;
	#else
	return green-ExtractAndExpandBitsAt(green,8-PixelGreenBits,PixelGreenBits);
	#endif
}

static inline int ErrorForBlue(int blue)
{
	#if PixelRedBits==8
	return 0;
	#else
	return blue-ExtractAndExpandBitsAt(blue,8-PixelBlueBits,PixelBlueBits);
	#endif
}

static inline unsigned int ExpandPixelColourBitsTo12(unsigned int val,unsigned int bits)
{
	switch(bits)
	{
		case 1: return val*0xfff;
		case 2: return val*0x555;
		case 3: return val*0x249;
		case 4: return val*0x111;
		case 5: return val*0x421>>3;
		case 6: return val*0x041;
		case 7: return val*0x081>>2;
		case 8: return val*0x101>>4;
		case 9: return val*0x201>>6;
		case 10: return val*0x401>>8;
		case 11: return val*0x801>>10;
		default: case 12: return val;
	}
}

static inline Pixel RGBWithErrors(int red,int green,int blue,int *rederror,int *blueerror,int *greenerror)
{
	int red12=(red*0x101>>4)+*rederror;
	int green12=(green*0x101>>4)+*greenerror;
	int blue12=(blue*0x101>>4)+*blueerror;

	int rednative=red12>>(12-PixelRedBits);
	int greennative=green12>>(12-PixelGreenBits);
	int bluenative=blue12>>(12-PixelBlueBits);

	if(rednative<0) rednative=0; if(rednative>=1<<PixelRedBits) rednative=(1<<PixelRedBits)-1;
	if(greennative<0) greennative=0; if(greennative>=1<<PixelGreenBits) greennative=(1<<PixelGreenBits)-1;
	if(bluenative<0) bluenative=0; if(bluenative>=1<<PixelBlueBits) bluenative=(1<<PixelBlueBits)-1;

	int actualred12=ExpandPixelColourBitsTo12(rednative,PixelRedBits);
	int actualgreen12=ExpandPixelColourBitsTo12(greennative,PixelGreenBits);
	int actualblue12=ExpandPixelColourBitsTo12(bluenative,PixelBlueBits);

	*rederror=red12-actualred12;
	*greenerror=green12-actualgreen12;
	*blueerror=blue12-actualblue12;

	return RawRGB(rednative,greennative,bluenative);
}

#endif

