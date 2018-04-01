#include "BlendingColours.h"

static unsigned int Multiply8(unsigned int a,unsigned int b)
{
	return a*b/255;
}

Pixel MultiplyColours(Pixel c1,Pixel c2)
{
	unsigned int r=Multiply8(ExtractRed(c1),ExtractRed(c2));
	unsigned int g=Multiply8(ExtractGreen(c1),ExtractGreen(c2));
	unsigned int b=Multiply8(ExtractBlue(c1),ExtractBlue(c2));
	return RGB(r,g,b);
}

Pixel MultiplyColoursAndAlpha(Pixel c1,Pixel c2)
{
	unsigned int r=Multiply8(ExtractRed(c1),ExtractRed(c2));
	unsigned int g=Multiply8(ExtractGreen(c1),ExtractGreen(c2));
	unsigned int b=Multiply8(ExtractBlue(c1),ExtractBlue(c2));
	unsigned int a=Multiply8(ExtractAlpha(c1),ExtractAlpha(c2));
	return RGBA(r,g,b,a);
}

static unsigned int Blend8(unsigned int a,unsigned int b,unsigned int t)
{
	return a+(((b-a)*t)>>8);
}

Pixel BlendColours(Pixel c1,Pixel c2,int t)
{
	unsigned int r=Blend8(ExtractRed(c1),ExtractRed(c2),t);
	unsigned int g=Blend8(ExtractGreen(c1),ExtractGreen(c2),t);
	unsigned int b=Blend8(ExtractBlue(c1),ExtractBlue(c2),t);
	return RGB(r,g,b);
}

Pixel BlendColoursAndAlpha(Pixel c1,Pixel c2,int t)
{
	unsigned int r=Blend8(ExtractRed(c1),ExtractRed(c2),t);
	unsigned int g=Blend8(ExtractGreen(c1),ExtractGreen(c2),t);
	unsigned int b=Blend8(ExtractBlue(c1),ExtractBlue(c2),t);
	unsigned int a=Blend8(ExtractAlpha(c1),ExtractAlpha(c2),t);
	return RGBA(r,g,b,a);
}
