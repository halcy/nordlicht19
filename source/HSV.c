#include "HSV.h"

Pixel Hue(int h)
{
	unsigned int c=h&0xff;
	switch((h>>8)%6)
	{
		default:
		case 0: return UnclampedRGB(0xff,c,0);
		case 1: case -5: return UnclampedRGB(0xff-c,0xff,0);
		case 2: case -4: return UnclampedRGB(0,0xff,c);
		case 3: case -3: return UnclampedRGB(0,0xff-c,0xff);
		case 4: case -2: return UnclampedRGB(c,0,0xff);
		case 5: case -1: return UnclampedRGB(0xff,0,0xff-c);
	}
}

Pixel HSVA(int h,int s,int v,int a)
{
	if(v<0) return Grey(0); if(v>255) v=255;
	if(s<0) return Grey(v); if(s>255) s=255;
	if(a<0) a=0; if(a>255) a=255;

	unsigned int f=h&0xff;
	unsigned int p=v*(255-s)/255;
	unsigned int q=v*(255-f*s/255)/255;
	unsigned int t=v*(255-s+f*s/255)/255;

	switch((h>>8)%6)
	{
		default:
		case 0: return RGBA(v,t,p,a);
		case 1: case -5: return RGBA(q,v,p,a);
		case 2: case -4: return RGBA(p,v,t,a);
		case 3: case -3: return RGBA(p,q,v,a);
		case 4: case -2: return RGBA(t,p,v,a);
		case 5: case -1: return RGBA(v,p,q,a);
	}
}

Pixel HSLA(int h,int s,int l,int a)
{
	if(l<=0) return Grey(0); if(l>255) l=255;
	if(s<=0) return Grey(l); if(s>255) s=255;
	if(a<0) a=0; if(a>255) a=255;

	unsigned int tmp;
	if(l<=127) tmp=s*l/255;
	else tmp=s*(255-l)/255;

	return HSVA(h,2*255*tmp/(l+tmp),l+tmp,a);
}
