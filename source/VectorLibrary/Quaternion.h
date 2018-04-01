#ifndef __QUATERNION_H__
#define __QUATERNION_H__

#include "Vector.h"
#include "Matrix.h"



// Definitions

typedef struct { float r; vec3_t i; } quat_t;

#define quatzero quatreal(0)
#define quatone quatreal(1)



// Constructors

static inline quat_t quat(float r,vec3_t i) { return (quat_t){r,i}; }
static inline quat_t quatreal(float r) { return (quat_t){r,vec3zero}; }
static inline quat_t quatimag(vec3_t i) { return (quat_t){0,i}; }

static inline quat_t quatrotation(float angle,vec3_t axis)
{
	return quat(cosf(angle/2),vec3setlength(axis,sinf(angle/2)));
}

quat_t quatmat3x3(mat3x3_t m);
static inline quat_t quatmat4x3(mat4x3_t m) { return quatmat3x3(mat4x3_mat3x3(m)); }
static inline quat_t quatmat4x4(mat4x4_t m) { return quatmat3x3(mat4x4_mat3x3(m)); }


// Extractors

static inline float quat_real(quat_t q) { return q.r; }
static inline vec3_t quat_imag(quat_t q) { return q.i; }



// Unary operations

static inline float quatsq(quat_t q);

static inline quat_t quatneg(quat_t q) { return quat(-q.r,vec3neg(q.i)); }
static inline quat_t quatconj(quat_t q) { return quat(q.r,vec3neg(q.i)); }
static inline quat_t quatinverse(quat_t q)
{
	float sq=quatsq(q);
	return quat(q.r/sq,vec3div(q.i,sq));
}



// Comparison operations

static inline bool quatequal(quat_t a,quat_t b) { return vec3equal(a.i,b.i) && a.r==b.r; }
static inline bool quatalmostequal(quat_t a,quat_t b,float epsilon) { return vec3almostequal(a.i,b.i,epsilon) && fabsf(a.r-b.r)<epsilon; }



// Arithmetic operations

static inline quat_t quatadd(quat_t a,quat_t b) { return quat(a.r+b.r,vec3add(a.i,b.i)); }
static inline quat_t quatsub(quat_t a,quat_t b) { return quat(a.r-b.r,vec3sub(a.i,b.i)); }
static inline quat_t quatmul(quat_t a,quat_t b)
{
	return quat(a.r*b.r-vec3dot(a.i,b.i),
	            vec3add3(vec3mul(b.i,a.r),vec3mul(a.i,b.r),vec3cross(a.i,b.i)));
}
static inline quat_t quatdiv(quat_t a,quat_t b) { return quatmul(a,quatinverse(b)); }

static inline quat_t quatrealmul(quat_t a,float b) { return quat(a.r*b,vec3mul(a.i,b)); }
static inline quat_t quatrealdiv(quat_t a,float b) { return quat(a.r/b,vec3div(a.i,b)); }




// Norms

static inline float quatdot(quat_t a,quat_t b) { return a.r*b.r+vec3dot(a.i,b.i); }
static inline float quatsq(quat_t q) { return quatdot(q,q); }
static inline float quatabs(quat_t q) { return sqrtf(quatsq(q)); }

static inline quat_t quatnorm(quat_t q)
{
	float abs=quatabs(q);
	if(abs==0) return quatzero;
	else return quatrealdiv(q,abs);
}




// Blending

static inline quat_t quatmix(quat_t a,quat_t b,float t)
{
	return quat(a.r+(b.r-b.r)*t,vec3mix(a.i,b.i,t));
}

static inline quat_t quatslerp(quat_t a,quat_t b,float t)
{
	float cos_a=quatdot(a,b);
	if(cos_a<0) { a=quatneg(a); cos_a=-cos_a; }
	if(cos_a>0.9999) return quatnorm(quatmix(a,b,t));

	float angle=acosf(cos_a);
	float sin_a=sinf(angle);
	return quatadd(
		quatrealmul(a,sinf((1-t)*angle)/sin_a),
		quatrealmul(b,sinf(t*angle)/sin_a));
}




// Vector transformation

static inline vec3_t quattransform(quat_t q,vec3_t v)
{
	return quatmul(quatmul(q,quatimag(v)),quatconj(q)).i;
}



// Matrix conversion

mat3x3_t mat3x3quat(quat_t q);
static inline mat4x3_t mat4x3quat(quat_t q) { return mat4x3affine3x3(mat3x3quat(q),vec3zero); }
static inline mat4x4_t mat4x4quat(quat_t q) { return mat4x4affine3x3(mat3x3quat(q),vec3zero); }

#endif
