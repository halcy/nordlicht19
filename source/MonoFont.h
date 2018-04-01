#ifndef __MONO_FONT_H__
#define __MONO_FONT_H__

#include "Font.h"

typedef struct MonoFont
{
	Font font;
	int firstglyph,lastglyph;
	uint8_t *glyphs[0];
} MonoFont;

int WidthOfMonoFontCharacter(const Font *font,int c);
void DrawMonoFontCharacter(Bitmap *bitmap,const Font *font,int x,int y,Pixel col,int c);

#endif
