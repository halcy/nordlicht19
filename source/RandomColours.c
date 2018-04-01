#include "RandomColours.h"

uint32_t RandomInteger();

Pixel RandomColour()
{
	unsigned int r=RandomInteger()&0xff;
	unsigned int g=RandomInteger()&0xff;
	unsigned int b=RandomInteger()&0xff;
	return RGB(r,g,b);
}

Pixel RandomBrightColour()
{
	unsigned int c1=RandomInteger()&0xff;
	unsigned int c2=RandomInteger()&0xff;
	switch(RandomInteger()%3)
	{
		default:
		case 0: return UnclampedRGB(0xff,c1,c2);
		case 1: return UnclampedRGB(c2,0xff,c2);
		case 2: return UnclampedRGB(c1,c2,0xff);
	}
}

Pixel RandomBrightSaturatedColour()
{
	unsigned int c=RandomInteger()&0xff;
	switch(RandomInteger()%6)
	{
		default:
		case 0: return UnclampedRGB(0xff,c,0);
		case 1: return UnclampedRGB(c,0xff,0);
		case 2: return UnclampedRGB(0,0xff,c);
		case 3: return UnclampedRGB(0,c,0xff);
		case 4: return UnclampedRGB(c,0,0xff);
		case 5: return UnclampedRGB(0xff,0,c);
	}
}
