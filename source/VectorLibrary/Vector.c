#include "Vector.h"

vec3_t vec3cross(vec3_t a,vec3_t b)
{
	return vec3(a.y*b.z-a.z*b.y,
	            a.z*b.x-a.x*b.z,
	            a.x*b.y-a.y*b.x);
}

