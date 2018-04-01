#ifndef __VECTOR_H__
#define __VECTOR_H__

#include <math.h>
#include <stdbool.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif




// Definitions

typedef struct { float x,y; } vec2_t;
typedef struct { float x,y,z; } vec3_t;
typedef struct { float x,y,z,w; } vec4_t;

#define vec2zero vec2(0,0)
#define vec3zero vec3(0,0,0)
#define vec4zero vec4(0,0,0,0)



// Constructors

inline vec2_t vec2(float x,float y) { return (vec2_t){x,y}; }
inline vec3_t vec3(float x,float y,float z) { return (vec3_t){x,y,z}; }
static inline vec4_t vec4(float x,float y,float z,float w) { return (vec4_t){x,y,z,w}; }

static inline vec2_t vec2cyl(float r,float angle)
{
	return vec2(r*cosf(angle),r*sinf(angle));
}

static inline vec4_t vec4vec3(vec3_t v) { return vec4(v.x,v.y,v.z,1); }


// Extractors

static inline vec2_t vec3_xy(vec3_t v) { return vec2(v.x,v.y); }
static inline vec2_t vec3_xz(vec3_t v) { return vec2(v.x,v.z); }
static inline vec2_t vec3_yz(vec3_t v) { return vec2(v.y,v.z); }

static inline vec3_t vec4_xyz(vec4_t v) { return vec3(v.x,v.y,v.z); }



// Unary operations

static inline vec2_t vec2neg(vec2_t v) { return vec2(-v.x,-v.y); }
static inline vec3_t vec3neg(vec3_t v) { return vec3(-v.x,-v.y,-v.z); }
static inline vec4_t vec4neg(vec4_t v) { return vec4(-v.x,-v.y,-v.z,-v.w); }



// Comparison operations

static inline bool vec2equal(vec2_t a,vec2_t b) { return a.x==b.x && a.y==b.y; }
static inline bool vec3equal(vec3_t a,vec3_t b) { return a.x==b.x && a.y==b.y && a.z==b.z; }
static inline bool vec4equal(vec4_t a,vec4_t b) { return a.x==b.x && a.y==b.y && a.z==b.z && a.w==b.w; }

static inline bool vec2almostequal(vec2_t a,vec2_t b,float epsilon) { return fabsf(a.x-b.x)<epsilon && fabsf(a.y-b.y)<epsilon; }
static inline bool vec3almostequal(vec3_t a,vec3_t b,float epsilon) { return fabsf(a.x-b.x)<epsilon && fabsf(a.y-b.y)<epsilon && fabsf(a.z-b.z)<epsilon; }
static inline bool vec4almostequal(vec4_t a,vec4_t b,float epsilon) { return fabsf(a.x-b.x)<epsilon && fabsf(a.y-b.y)<epsilon && fabsf(a.z-b.z)<epsilon && fabsf(a.w-b.w)<epsilon; }



// Arithmetic operations

static inline vec2_t vec2add(vec2_t a,vec2_t b) { return vec2(a.x+b.x,a.y+b.y); }
inline vec3_t vec3add(vec3_t a,vec3_t b) { return vec3(a.x+b.x,a.y+b.y,a.z+b.z); }
static inline vec4_t vec4add(vec4_t a,vec4_t b) { return vec4(a.x+b.x,a.y+b.y,a.z+b.z,a.w+b.w); }

static inline vec2_t vec2add3(vec2_t a,vec2_t b,vec2_t c) { return vec2(a.x+b.x+c.x,a.y+b.y+c.y); }
static inline vec3_t vec3add3(vec3_t a,vec3_t b,vec3_t c) { return vec3(a.x+b.x+c.x,a.y+b.y+c.y,a.z+b.z+c.z); }
static inline vec4_t vec4add3(vec4_t a,vec4_t b,vec4_t c) { return vec4(a.x+b.x+c.x,a.y+b.y+c.y,a.z+b.z+c.z,a.w+b.w+c.w); }

static inline vec2_t vec2sub(vec2_t a,vec2_t b) { return vec2(a.x-b.x,a.y-b.y); }
inline vec3_t vec3sub(vec3_t a,vec3_t b) { return vec3(a.x-b.x,a.y-b.y,a.z-b.z); }
static inline vec4_t vec4sub(vec4_t a,vec4_t b) { return vec4(a.x-b.x,a.y-b.y,a.z-b.z,a.w-b.w); }

static inline vec2_t vec2mul(vec2_t v,float s) { return vec2(v.x*s,v.y*s); }
inline vec3_t vec3mul(vec3_t v,float s) { return vec3(v.x*s,v.y*s,v.z*s); }
static inline vec4_t vec4mul(vec4_t v,float s) { return vec4(v.x*s,v.y*s,v.z*s,v.w*s); }

static inline vec2_t vec2div(vec2_t v,float s) { return vec2(v.x/s,v.y/s); }
static inline vec3_t vec3div(vec3_t v,float s) { return vec3(v.x/s,v.y/s,v.z/s); }
static inline vec4_t vec4div(vec4_t v,float s) { return vec4(v.x/s,v.y/s,v.z/s,v.w/s); }

static inline vec2_t vec2mac(vec2_t a,vec2_t b,float c) { return vec2(a.x+b.x*c,a.y+b.y*c); }
static inline vec3_t vec3mac(vec3_t a,vec3_t b,float c) { return vec3(a.x+b.x*c,a.y+b.y*c,a.z+b.z*c); }
static inline vec4_t vec4mac(vec4_t a,vec4_t b,float c) { return vec4(a.x+b.x*c,a.y+b.y*c,a.z+b.z*c,a.w+b.w*c); }




// Norms

static inline float vec2dot(vec2_t a,vec2_t b) { return a.x*b.x+a.y*b.y; }
static inline float vec3dot(vec3_t a,vec3_t b) { return a.x*b.x+a.y*b.y+a.z*b.z; }
static inline float vec4dot(vec4_t a,vec4_t b) { return a.x*b.x+a.y*b.y+a.z*b.z+a.w*b.w; }

static inline float vec2sq(vec2_t v) { return vec2dot(v,v); }
static inline float vec3sq(vec3_t v) { return vec3dot(v,v); }
static inline float vec4sq(vec4_t v) { return vec4dot(v,v); }

static inline float vec2abs(vec2_t v) { return sqrtf(vec2sq(v)); }
static inline float vec3abs(vec3_t v) { return sqrtf(vec3sq(v)); }
static inline float vec4abs(vec4_t v) { return sqrtf(vec4sq(v)); }

static inline float vec2distsq(vec2_t a,vec2_t b) { return vec2sq(vec2sub(a,b)); }
static inline float vec3distsq(vec3_t a,vec3_t b) { return vec3sq(vec3sub(a,b)); }
static inline float vec4distsq(vec4_t a,vec4_t b) { return vec4sq(vec4sub(a,b)); }

static inline float vec2dist(vec2_t a,vec2_t b) { return vec2abs(vec2sub(a,b)); }
static inline float vec3dist(vec3_t a,vec3_t b) { return vec3abs(vec3sub(a,b)); }
static inline float vec4dist(vec4_t a,vec4_t b) { return vec4abs(vec4sub(a,b)); }

#ifdef USE_FIXEDPOINT64
static inline int64_t vec2dot64(vec2_t a,vec2_t b) { return imul64(a.x,b.x)+imul64(a.y,b.y); }
static inline int64_t vec3dot64(vec3_t a,vec3_t b) { return imul64(a.x,b.x)+imul64(a.y,b.y)+imul64(a.z,b.z); }
static inline int64_t vec4dot64(vec4_t a,vec4_t b) { return imul64(a.x,b.x)+imul64(a.y,b.y)+imul64(a.z,b.z)+imul64(a.w,b.w); }

static inline int64_t vec2sq64(vec2_t v) { return vec2dot64(v,v); }
static inline int64_t vec3sq64(vec3_t v) { return vec3dot64(v,v); }
static inline int64_t vec4sq64(vec4_t v) { return vec4dot64(v,v); }

static inline int64_t vec2abs64(vec2_t v) { return isqrt64(vec2sq64(v)); }
static inline int64_t vec3abs64(vec3_t v) { return isqrt64(vec3sq64(v)); }
static inline int64_t vec4abs64(vec4_t v) { return isqrt64(vec4sq64(v)); }

static inline int64_t vec2distsq64(vec2_t a,vec2_t b) { return vec2sq64(vec2sub(a,b)); }
static inline int64_t vec3distsq64(vec3_t a,vec3_t b) { return vec3sq64(vec3sub(a,b)); }
static inline int64_t vec4distsq64(vec4_t a,vec4_t b) { return vec4sq64(vec4sub(a,b)); }

static inline int64_t vec2dist64(vec2_t a,vec2_t b) { return vec2abs64(vec2sub(a,b)); }
static inline int64_t vec3dist64(vec3_t a,vec3_t b) { return vec3abs64(vec3sub(a,b)); }
static inline int64_t vec4dist64(vec4_t a,vec4_t b) { return vec4abs64(vec4sub(a,b)); }
#endif

static inline vec2_t vec2norm(vec2_t v)
{
	float abs=vec2abs(v);
	if(abs==0) return vec2zero;
	else return vec2div(v,abs);
}

static inline vec3_t vec3norm(vec3_t v)
{
	float abs=vec3abs(v);
	if(abs==0) return vec3zero;
	else return vec3div(v,abs);
}

static inline vec4_t vec4norm(vec4_t v)
{
	float abs=vec4abs(v);
	if(abs==0) return vec4zero;
	else return vec4div(v,abs);
}

static inline vec2_t vec2setlength(vec2_t v,float length) { return vec2mul(vec2norm(v),length); }
static inline vec3_t vec3setlength(vec3_t v,float length) { return vec3mul(vec3norm(v),length); }
static inline vec4_t vec4setlength(vec4_t v,float length) { return vec4mul(vec4norm(v),length); }



// Homogenous conversion functions

static inline vec4_t vec4homogenize(vec4_t v) { return vec4(v.x/v.w,v.y/v.w,v.z/v.w,1); }
static inline vec3_t vec4homogenize3(vec4_t v) { return vec3(v.x/v.w,v.y/v.w,v.z/v.w); }
static inline vec4_t vec3unhomogenize(vec3_t v) { return vec4(v.x,v.y,v.z,1); }



// Blending

static inline vec2_t vec2midpoint(vec2_t a,vec2_t b) { return vec2((a.x+b.x)/2,(a.y+b.y)/2); }
static inline vec3_t vec3midpoint(vec3_t a,vec3_t b) { return vec3((a.x+b.x)/2,(a.y+b.y)/2,(a.z+b.z)/2); }
static inline vec4_t vec4midpoint(vec4_t a,vec4_t b) { return vec4((a.x+b.x)/2,(a.y+b.y)/2,(a.z+b.z)/2,(a.w+b.w)/2); }

static inline vec2_t vec2mix(vec2_t a,vec2_t b,float x) { return vec2add(a,vec2mul(vec2sub(b,a),x)); }
static inline vec3_t vec3mix(vec3_t a,vec3_t b,float x) { return vec3add(a,vec3mul(vec3sub(b,a),x)); }
static inline vec4_t vec4mix(vec4_t a,vec4_t b,float x) { return vec4add(a,vec4mul(vec4sub(b,a),x)); }



// 2D specifics

static inline vec2_t vec2perp(vec2_t v) { return vec2(-v.y,v.x); }

static inline float vec2pdot(vec2_t a,vec2_t b) { return vec2dot(a,vec2perp(b)); }
#ifdef USE_FIXEDPOINT64
static inline float vec2pdot64(vec2_t a,vec2_t b) { return vec2dot64(a,vec2perp(b)); }
#endif

static inline vec2_t vec2rot(vec2_t v,float angle)
{
	return vec2(
	v.x*cosf(angle)-v.y*sinf(angle),
	v.x*sinf(angle)+v.y*cosf(angle));
}



// 3D specifics

vec3_t vec3cross(vec3_t a,vec3_t b);



// Plane functions

static inline vec3_t vec4_planenormal(vec4_t p) { return vec4_xyz(p); }

static inline vec3_t vec4_vec3plane(vec4_t p) { return vec4homogenize3(p); }

static inline vec4_t vec4planenorm(vec4_t p)
{
	float abs=vec3abs(vec4_planenormal(p));
	if(abs==0) return vec4zero;
	else return vec4div(p,abs);
}

static inline float vec3planepointdistance(vec3_t p,vec3_t v) { return (vec3dot(p,v)+1)/vec3abs(p); }
static inline float vec4planepointdistance(vec4_t p,vec3_t v) { return vec4dot(p,vec4vec3(v))/vec3abs(vec4_planenormal(p)); }

#endif

