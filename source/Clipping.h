#ifndef __CLIPPING_H__
#define __CLIPPING_H__

#include <stdbool.h>

static inline bool IsSpanCompletelyInside(int start,int len,int max)
{
	if(start<0) return false;
	if(start+len>max) return false;
	return true;
}

static inline bool IsSpanCompletelyOutside(int start,int len,int max)
{
	if(start>=max) return true;
	if(start+len<=0) return true;
	return false;
}

static inline bool IsCompletelyInside(int x,int y,int w,int h,int clipw,int cliph)
{
	return IsSpanCompletelyInside(x,w,clipw)&&IsSpanCompletelyInside(y,h,cliph);
}

static inline bool IsCompletelyOutside(int x,int y,int w,int h,int clipw,int cliph)
{
	return IsSpanCompletelyOutside(x,w,clipw)||IsSpanCompletelyOutside(y,h,cliph);
}

static inline void AdjustSpan(int *start,int *len,int max)
{
	if(*start<0) { (*len)+=*start; *start=0; }
	if(*start+*len>max) *len=max-*start;
}

static inline void AdjustSpan2(int *start,int *u,int *len,int max)
{
	if(*start<0) { (*len)+=*start; (*u)-=*start; *start=0; }
	if(*start+*len>max) *len=max-*start;
}

static inline bool ClipLine(int *u,int *v,int *len,int clipu,int clipv)
{
	if(IsCompletelyOutside(*u,*v,*len,1,clipu,clipv)) return false;
	AdjustSpan(u,len,clipu);
	return true;
}

static inline bool ClipRectangle(int *x,int *y,int *w,int *h,int clipw,int cliph)
{
	if(IsCompletelyOutside(*x,*y,*w,*h,clipw,cliph)) return false;
	AdjustSpan(x,w,clipw);
	AdjustSpan(y,h,cliph);
	return true;
}

static inline bool ClipRectangle2(int *x,int *y,int *u,int *v,int *w,int *h,int clipw,int cliph)
{
	if(IsCompletelyOutside(*x,*y,*w,*h,clipw,cliph)) return false;
	AdjustSpan2(x,u,w,clipw);
	AdjustSpan2(y,v,h,cliph);
	return true;
}

#endif

