#ifndef __PIXELS_FUNCTIONS_H__
#define __PIXELS_FUNCTIONS_H__

#include <stdint.h>

#define Clamp(x,min,max) ((x)<(min)?(min):(x)>(max)?(max):(x))

#define BitMaskAt(pos,length) (((1<<(length))-1)<<(pos))

#define BitExpansionMultiplier(bits) ( \
	(bits)==7?0x81: \
	(bits)==6?0x41: \
	(bits)==5?0x21: \
	(bits)==4?0x11: \
	(bits)==3?0x49: \
	(bits)==2?0x55: \
	(bits)==1?0xff: \
	1 )

#define BitExpansionShift(bits) ( \
	(bits)==7?6: \
	(bits)==6?4: \
	(bits)==5?2: \
	(bits)==3?1: \
	0 )

#define ExtractAndExpandBitsAt(val,pos,length) ( \
	( ((uint32_t)(val)&BitMaskAt((pos),(length)))*BitExpansionMultiplier((length)) )>> \
	((pos)+BitExpansionShift((length))) )

#define PixelMul(a,b) ( ((uint32_t)(a)*(uint32_t)(b))/255 )
#define PixelUnMul(a,b) ( (b)==0?0:(uint32_t)(a)*255/(uint32_t)(b) )

#endif
