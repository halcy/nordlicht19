#ifndef __FONT_H__
#define __FONT_H__

#include "Bitmap.h"
#include "Drawing.h"

#include <stdint.h>

typedef struct Font Font;

typedef int CharacterWidthFontFunction(const Font *font,int c);
typedef int StringWidthFontFunction(const Font *font,const void *str);
typedef void DrawCharacterFontFunction(Bitmap *bitmap,const Font *font,int x,int y,Pixel col,int c);
typedef void DrawStringFontFunction(Bitmap *bitmap,const Font *font,int x,int y,Pixel col,const void *str);
typedef void CompositeCharacterFontFunction(Bitmap *bitmap,const Font *font,int x,int y,Pixel col,CompositionMode comp,int c);
typedef void CompositeStringFontFunction(Bitmap *bitmap,const Font *font,int x,int y,Pixel col,CompositionMode comp,const void *str);

struct Font
{
	int height;

	CharacterWidthFontFunction *charwidthfunc;
	StringWidthFontFunction *stringwidthfunc;
	DrawCharacterFontFunction *drawcharfunc;
	DrawStringFontFunction *drawstringfunc;
	CompositeCharacterFontFunction *compcharfunc;
	CompositeStringFontFunction *compstringfunc;
};

static inline int HeightOfFont(const Font *font)
{
	return font->height;
}

static inline int WidthOfCharacter(const Font *font,int c)
{
	return font->charwidthfunc(font,c);
}

static inline int WidthOfString(const Font *font,const void *str)
{
	return font->stringwidthfunc(font,str);
}

static inline void DrawCharacter(Bitmap *bitmap,const Font *font,int x,int y,Pixel col,int c)
{
	font->drawcharfunc(bitmap,font,x,y,col,c);
}

static inline void DrawString(Bitmap *bitmap,const Font *font,int x,int y,Pixel col,const void *str)
{
	font->drawstringfunc(bitmap,font,x,y,col,str);
}

static inline void CompositeCharacter(Bitmap *bitmap,const Font *font,int x,int y,Pixel col,CompositionMode comp,int c)
{
	font->compcharfunc(bitmap,font,x,y,col,comp,c);
}

static inline void CompositeString(Bitmap *bitmap,const Font *font,int x,int y,Pixel col,CompositionMode comp,const void *str)
{
	font->compstringfunc(bitmap,font,x,y,col,comp,str);
}

static inline void DrawStringToTheLeftOf(Bitmap *bitmap,const Font *font,
int x,int y,Pixel col,const void *str)
{
	int stringwidth=WidthOfString(font,str);
	DrawString(bitmap,font,x-stringwidth+1,y,col,str);
}

static inline void DrawStringCenteredInRectangle(Bitmap *bitmap,const Font *font,
int x,int y,int w,int h,Pixel col,const void *str)
{
	int stringwidth=WidthOfString(font,str);
	int stringheight=HeightOfFont(font);
	DrawString(bitmap,font,x+(w-stringwidth)/2,y+(h-stringheight)/2,col,str);
}

static inline void CompositeStringToTheLeftOf(Bitmap *bitmap,const Font *font,
int x,int y,Pixel col,CompositionMode comp,const void *str)
{
	int stringwidth=WidthOfString(font,str);
	CompositeString(bitmap,font,x-stringwidth+1,y,col,comp,str);
}

static inline void CompositeStringCenteredInRectangle(Bitmap *bitmap,const Font *font,
int x,int y,int w,int h,Pixel col,CompositionMode comp,const void *str)
{
	int stringwidth=WidthOfString(font,str);
	int stringheight=HeightOfFont(font);
	CompositeString(bitmap,font,x+(w-stringwidth)/2,y+(h-stringheight)/2,col,comp,str);
}

int WidthOfSimpleString(const Font *font,const void *str);
void DrawSimpleString(Bitmap *bitmap,const Font *font,int x,int y,Pixel col,const void *str);
void CompositeSimpleString(Bitmap *bitmap,const Font *font,int x,int y,Pixel col,CompositionMode comp,const void *str);

#endif
