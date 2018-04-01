#ifndef __PIXELS_ENDIANNESS_H__
#define __PIXELS_ENDIANNESS_H__

#ifdef __BIG_ENDIAN__ // TODO: Figure out proper define

#define FirstByteShift 24
#define SecondByteShift 16
#define ThirdByteShift 8
#define FourthByteShift 0

#else

#define FirstByteShift 0
#define SecondByteShift 8
#define ThirdByteShift 16
#define FourthByteShift 24

#endif

#endif
