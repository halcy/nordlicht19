#ifndef __COLOUR_FONT_H__
#define __COLOUR_FONT_H__

#include "Font.h"
#include "RLEBitmap.h"

typedef struct ColourGlyph {
	int8_t kerning;
	uint8_t spacing;
	const RLEBitmap *rle;
} ColourGlyph;

typedef struct ColourFont
{
	Font font;
	int firstglyph,lastglyph;
	const ColourGlyph *glyphs[0];
} ColourFont;

int KerningForColourFontCharacters(const Font *font,int c,int prev);
int SpacingForColourFontCharacter(const Font *font,int c);
void DrawColourFontCharacter(Bitmap *bitmap,const Font *font,int x,int y,Pixel col,int c);
void CompositeColourFontCharacter(Bitmap *bitmap,const Font *font,int x,int y,Pixel col,CompositionMode comp,int c);

#endif
