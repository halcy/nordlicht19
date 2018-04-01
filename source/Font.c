#include "Font.h"

int WidthOfSimpleString(const Font *font,const void *str)
{
	const uint8_t *cstring=str;
	int len=0;
	while(*cstring) len+=WidthOfCharacter(font,*cstring++);
	return len;
}

void DrawSimpleString(Bitmap *bitmap,const Font *font,int x,int y,Pixel col,const void *str)
{
	const uint8_t *cstring=str;
	while(*cstring)
	{
		DrawCharacter(bitmap,font,x,y,col,*cstring);
		x+=WidthOfCharacter(font,*cstring);
		cstring++;
	}
}

void CompositeSimpleString(Bitmap *bitmap,const Font *font,int x,int y,Pixel col,CompositionMode comp,const void *str)
{
	const uint8_t *cstring=str;
	while(*cstring)
	{
		CompositeCharacter(bitmap,font,x,y,col,comp,*cstring);
		x+=WidthOfCharacter(font,*cstring);
		cstring++;
	}
}
