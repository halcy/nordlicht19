#include "Quaternion.h"

quat_t quatmat3x3(mat3x3_t m)
{
	if(mat3x3_11(m)>mat3x3_22(m) && mat3x3_11(m)>mat3x3_33(m))
	{
		float r=sqrtf(1+mat3x3_11(m)-mat3x3_22(m)-mat3x3_33(m));
		if(r==0) return quatone;
		return quat(
			(mat3x3_32(m)-mat3x3_23(m))/(2*r),
			vec3(
				r/2,
				(mat3x3_12(m)+mat3x3_21(m))/(2*r),
				(mat3x3_31(m)+mat3x3_13(m))/(2*r)
			)
		);
	}
	else if(mat3x3_22(m)>mat3x3_11(m) && mat3x3_22(m)>mat3x3_33(m))
	{
		float r=sqrtf(1+mat3x3_22(m)-mat3x3_33(m)-mat3x3_11(m));
		if(r==0) return quatone;
		return quat(
			(mat3x3_13(m)-mat3x3_31(m))/(2*r),
			vec3(
				(mat3x3_12(m)+mat3x3_21(m))/(2*r),
				r/2,
				(mat3x3_23(m)+mat3x3_32(m))/(2*r)
			)
		);
	}
	else
	{
		float r=sqrtf(1+mat3x3_33(m)-mat3x3_11(m)-mat3x3_22(m));
		if(r==0) return quatone;
		return quat(
			(mat3x3_21(m)-mat3x3_12(m))/(2*r),
			vec3(
				(mat3x3_31(m)+mat3x3_13(m))/(2*r),
				(mat3x3_23(m)+mat3x3_32(m))/(2*r),
				r/2
			)
		);
	}
}

mat3x3_t mat3x3quat(quat_t q)
{
	float s2=q.r*q.r;
	float x2=q.i.x*q.i.x;
	float y2=q.i.y*q.i.y;
	float z2=q.i.z*q.i.z;

	return mat3x3(
	s2+x2-y2-z2,
	2*(q.i.x*q.i.y-q.r*q.i.z),
	2*(q.i.x*q.i.z+q.r*q.i.y),

	2*(q.i.x*q.i.y+q.r*q.i.z),
	s2-x2+y2-z2,
	2*(q.i.y*q.i.z-q.r*q.i.x),

	2*(q.i.x*q.i.z-q.r*q.i.y),
	2*(q.i.y*q.i.z+q.r*q.i.x),
	s2-x2-y2+z2);
}
