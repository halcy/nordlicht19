#ifndef __MATRIX_H__
#define __MATRIX_H__

#include "Vector.h"



// Definitions

typedef struct { float m[4]; } mat2x2_t;
typedef struct { float m[6]; } mat3x2_t;
typedef struct { float m[9]; } mat3x3_t;
typedef struct { float m[12]; } mat4x3_t;
typedef struct { float m[16]; } mat4x4_t;

#define mat2x2one mat2x2(1,0, 0,1)
#define mat3x2one mat3x2(1,0,0, 0,1,0)
#define mat3x3one mat3x3(1,0,0, 0,1,0, 0,0,1)
#define mat4x3one mat4x3(1,0,0,0, 0,1,0,0, 0,0,1,0)
#define mat4x4one mat4x4(1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1)



// Individual element constructors

static inline mat2x2_t mat2x2(float a11,float a12,
                              float a21,float a22)
{ return (mat2x2_t){a11,a21,a12,a22}; }

static inline mat3x2_t mat3x2(float a11,float a12,float a13,
                              float a21,float a22,float a23)
{ return (mat3x2_t){a11,a21,a12,a22,a13,a23}; }

static inline mat3x3_t mat3x3(float a11,float a12,float a13,
                              float a21,float a22,float a23,
                              float a31,float a32,float a33)
{ return (mat3x3_t){a11,a21,a31,a12,a22,a32,a13,a23,a33}; }

static inline mat4x3_t mat4x3(float a11,float a12,float a13,float a14,
                              float a21,float a22,float a23,float a24,
                              float a31,float a32,float a33,float a34)
{ return (mat4x3_t){a11,a21,a31,a12,a22,a32,a13,a23,a33,a14,a24,a34}; }

static inline mat4x4_t mat4x4(float a11,float a12,float a13,float a14,
                              float a21,float a22,float a23,float a24,
                              float a31,float a32,float a33,float a34,
                              float a41,float a42,float a43,float a44)
{ return (mat4x4_t){a11,a21,a31,a41,a12,a22,a32,a42,a13,a23,a33,a43,a14,a24,a34,a44}; }



// Individual element extractors

static inline float mat2x2_11(mat2x2_t m) { return m.m[0]; }
static inline float mat2x2_21(mat2x2_t m) { return m.m[1]; }
static inline float mat2x2_12(mat2x2_t m) { return m.m[2]; }
static inline float mat2x2_22(mat2x2_t m) { return m.m[3]; }

static inline float mat3x2_11(mat3x2_t m) { return m.m[0]; }
static inline float mat3x2_21(mat3x2_t m) { return m.m[1]; }
static inline float mat3x2_12(mat3x2_t m) { return m.m[2]; }
static inline float mat3x2_22(mat3x2_t m) { return m.m[3]; }
static inline float mat3x2_13(mat3x2_t m) { return m.m[4]; }
static inline float mat3x2_23(mat3x2_t m) { return m.m[5]; }

static inline float mat3x3_11(mat3x3_t m) { return m.m[0]; }
static inline float mat3x3_21(mat3x3_t m) { return m.m[1]; }
static inline float mat3x3_31(mat3x3_t m) { return m.m[2]; }
static inline float mat3x3_12(mat3x3_t m) { return m.m[3]; }
static inline float mat3x3_22(mat3x3_t m) { return m.m[4]; }
static inline float mat3x3_32(mat3x3_t m) { return m.m[5]; }
static inline float mat3x3_13(mat3x3_t m) { return m.m[6]; }
static inline float mat3x3_23(mat3x3_t m) { return m.m[7]; }
static inline float mat3x3_33(mat3x3_t m) { return m.m[8]; }

static inline float mat4x3_11(mat4x3_t m) { return m.m[0]; }
static inline float mat4x3_21(mat4x3_t m) { return m.m[1]; }
static inline float mat4x3_31(mat4x3_t m) { return m.m[2]; }
static inline float mat4x3_12(mat4x3_t m) { return m.m[3]; }
static inline float mat4x3_22(mat4x3_t m) { return m.m[4]; }
static inline float mat4x3_32(mat4x3_t m) { return m.m[5]; }
static inline float mat4x3_13(mat4x3_t m) { return m.m[6]; }
static inline float mat4x3_23(mat4x3_t m) { return m.m[7]; }
static inline float mat4x3_33(mat4x3_t m) { return m.m[8]; }
static inline float mat4x3_14(mat4x3_t m) { return m.m[9]; }
static inline float mat4x3_24(mat4x3_t m) { return m.m[10]; }
static inline float mat4x3_34(mat4x3_t m) { return m.m[11]; }

static inline float mat4x4_11(mat4x4_t m) { return m.m[0]; }
static inline float mat4x4_21(mat4x4_t m) { return m.m[1]; }
static inline float mat4x4_31(mat4x4_t m) { return m.m[2]; }
static inline float mat4x4_41(mat4x4_t m) { return m.m[3]; }
static inline float mat4x4_12(mat4x4_t m) { return m.m[4]; }
static inline float mat4x4_22(mat4x4_t m) { return m.m[5]; }
static inline float mat4x4_32(mat4x4_t m) { return m.m[6]; }
static inline float mat4x4_42(mat4x4_t m) { return m.m[7]; }
static inline float mat4x4_13(mat4x4_t m) { return m.m[8]; }
static inline float mat4x4_23(mat4x4_t m) { return m.m[9]; }
static inline float mat4x4_33(mat4x4_t m) { return m.m[10]; }
static inline float mat4x4_43(mat4x4_t m) { return m.m[11]; }
static inline float mat4x4_14(mat4x4_t m) { return m.m[12]; }
static inline float mat4x4_24(mat4x4_t m) { return m.m[13]; }
static inline float mat4x4_34(mat4x4_t m) { return m.m[14]; }
static inline float mat4x4_44(mat4x4_t m) { return m.m[15]; }




// Column vector constructors

static inline mat2x2_t mat2x2cols(vec2_t col1,vec2_t col2)
{
	return mat2x2(col1.x,col2.x,
	              col1.y,col2.y);
}

static inline mat3x2_t mat3x2cols(vec2_t col1,vec2_t col2,vec2_t col3)
{
	return mat3x2(col1.x,col2.x,col3.x,
	              col1.y,col2.y,col3.y);
}

static inline mat3x3_t mat3x3cols(vec3_t col1,vec3_t col2,vec3_t col3)
{
	return mat3x3(col1.x,col2.x,col3.x,
	              col1.y,col2.y,col3.y,
	              col1.z,col2.z,col3.z);
}

static inline mat4x3_t mat4x3cols(vec3_t col1,vec3_t col2,vec3_t col3,vec3_t col4)
{
	return mat4x3(col1.x,col2.x,col3.x,col4.x,
	              col1.y,col2.y,col3.y,col4.y,
	              col1.z,col2.z,col3.z,col4.z);
}

static inline mat4x4_t mat4x4cols(vec4_t col1,vec4_t col2,vec4_t col3,vec4_t col4)
{
	return mat4x4(col1.x,col2.x,col3.x,col4.x,
	              col1.y,col2.y,col3.y,col4.y,
	              col1.z,col2.z,col3.z,col4.z,
	              col1.w,col2.w,col3.w,col4.w);
}

static inline mat2x2_t mat2x2vec2(vec2_t x,vec2_t y) { return mat2x2cols(x,y); }
static inline mat3x2_t mat3x2vec2(vec2_t x,vec2_t y,vec2_t z) { return mat3x2cols(x,y,z); }
static inline mat3x3_t mat3x3vec3(vec3_t x,vec3_t y,vec3_t z) { return mat3x3cols(x,y,z); }
static inline mat4x3_t mat4x3vec3(vec3_t x,vec3_t y,vec3_t z,vec3_t w) { return mat4x3cols(x,y,z,w); }
static inline mat4x4_t mat4x4vec4(vec4_t x,vec4_t y,vec4_t z,vec4_t w) { return mat4x4cols(x,y,z,w); }



// Column vector extractors

static inline vec2_t mat2x2_col1(mat2x2_t m) { return vec2(mat2x2_11(m),mat2x2_21(m)); }
static inline vec2_t mat2x2_col2(mat2x2_t m) { return vec2(mat2x2_12(m),mat2x2_22(m)); }

static inline vec2_t mat3x2_col1(mat3x2_t m) { return vec2(mat3x2_11(m),mat3x2_21(m)); }
static inline vec2_t mat3x2_col2(mat3x2_t m) { return vec2(mat3x2_12(m),mat3x2_22(m)); }
static inline vec2_t mat3x2_col3(mat3x2_t m) { return vec2(mat3x2_13(m),mat3x2_23(m)); }

static inline vec3_t mat3x3_col1(mat3x3_t m) { return vec3(mat3x3_11(m),mat3x3_21(m),mat3x3_31(m)); }
static inline vec3_t mat3x3_col2(mat3x3_t m) { return vec3(mat3x3_12(m),mat3x3_22(m),mat3x3_32(m)); }
static inline vec3_t mat3x3_col3(mat3x3_t m) { return vec3(mat3x3_13(m),mat3x3_23(m),mat3x3_33(m)); }

static inline vec3_t mat4x3_col1(mat4x3_t m) { return vec3(mat4x3_11(m),mat4x3_21(m),mat4x3_31(m)); }
static inline vec3_t mat4x3_col2(mat4x3_t m) { return vec3(mat4x3_12(m),mat4x3_22(m),mat4x3_32(m)); }
static inline vec3_t mat4x3_col3(mat4x3_t m) { return vec3(mat4x3_13(m),mat4x3_23(m),mat4x3_33(m)); }
static inline vec3_t mat4x3_col4(mat4x3_t m) { return vec3(mat4x3_14(m),mat4x3_24(m),mat4x3_34(m)); }

static inline vec4_t mat4x4_col1(mat4x4_t m) { return vec4(mat4x4_11(m),mat4x4_21(m),mat4x4_31(m),mat4x4_41(m)); }
static inline vec4_t mat4x4_col2(mat4x4_t m) { return vec4(mat4x4_12(m),mat4x4_22(m),mat4x4_32(m),mat4x4_42(m)); }
static inline vec4_t mat4x4_col3(mat4x4_t m) { return vec4(mat4x4_13(m),mat4x4_23(m),mat4x4_33(m),mat4x4_43(m)); }
static inline vec4_t mat4x4_col4(mat4x4_t m) { return vec4(mat4x4_14(m),mat4x4_24(m),mat4x4_34(m),mat4x4_44(m)); }

static inline vec2_t mat2x2_x(mat2x2_t m) { return mat2x2_col1(m); }
static inline vec2_t mat2x2_y(mat2x2_t m) { return mat2x2_col2(m); }

static inline vec2_t mat3x2_x(mat3x2_t m) { return mat3x2_col1(m); }
static inline vec2_t mat3x2_y(mat3x2_t m) { return mat3x2_col2(m); }
static inline vec2_t mat3x2_z(mat3x2_t m) { return mat3x2_col3(m); }

static inline vec3_t mat3x3_x(mat3x3_t m) { return mat3x3_col1(m); }
static inline vec3_t mat3x3_y(mat3x3_t m) { return mat3x3_col2(m); }
static inline vec3_t mat3x3_z(mat3x3_t m) { return mat3x3_col3(m); }

static inline vec3_t mat4x3_x(mat4x3_t m) { return mat4x3_col1(m); }
static inline vec3_t mat4x3_y(mat4x3_t m) { return mat4x3_col2(m); }
static inline vec3_t mat4x3_z(mat4x3_t m) { return mat4x3_col3(m); }
static inline vec3_t mat4x3_w(mat4x3_t m) { return mat4x3_col4(m); }

static inline vec4_t mat4x4_x(mat4x4_t m) { return mat4x4_col1(m); }
static inline vec4_t mat4x4_y(mat4x4_t m) { return mat4x4_col2(m); }
static inline vec4_t mat4x4_z(mat4x4_t m) { return mat4x4_col3(m); }
static inline vec4_t mat4x4_w(mat4x4_t m) { return mat4x4_col4(m); }



// Row vector constructors

static inline mat2x2_t mat2x2rows(vec2_t row1,vec2_t row2)
{
	return mat2x2(row1.x,row1.y,
	              row2.x,row2.y);
}

static inline mat3x2_t mat3x2rows(vec3_t row1,vec3_t row2)
{
	return mat3x2(row1.x,row1.y,row1.z,
	              row2.x,row2.y,row2.z);
}

static inline mat3x3_t mat3x3rows(vec3_t row1,vec3_t row2,vec3_t row3)
{
	return mat3x3(row1.x,row1.y,row1.z,
	              row2.x,row2.y,row2.z,
	              row3.x,row3.y,row3.z);
}

static inline mat4x3_t mat4x3rows(vec4_t row1,vec4_t row2,vec4_t row3)
{
	return mat4x3(row1.x,row1.y,row1.z,row1.w,
	              row2.x,row2.y,row2.z,row2.w,
	              row3.x,row3.y,row3.z,row3.w);
}

static inline mat4x4_t mat4x4rows(vec4_t row1,vec4_t row2,vec4_t row3,vec4_t row4)
{
	return mat4x4(row1.x,row1.y,row1.z,row1.w,
	              row2.x,row2.y,row2.z,row2.w,
	              row3.x,row3.y,row3.z,row3.w,
	              row4.x,row4.y,row4.z,row4.w);
}



// Row vector extractors

static inline vec2_t mat2x2_row1(mat2x2_t m) { return vec2(mat2x2_11(m),mat2x2_12(m)); }
static inline vec2_t mat2x2_row2(mat2x2_t m) { return vec2(mat2x2_21(m),mat2x2_22(m)); }

static inline vec3_t mat3x2_row1(mat3x2_t m) { return vec3(mat3x2_11(m),mat3x2_12(m),mat3x2_13(m)); }
static inline vec3_t mat3x2_row2(mat3x2_t m) { return vec3(mat3x2_21(m),mat3x2_22(m),mat3x2_23(m)); }

static inline vec3_t mat3x3_row1(mat3x3_t m) { return vec3(mat3x3_11(m),mat3x3_12(m),mat3x3_13(m)); }
static inline vec3_t mat3x3_row2(mat3x3_t m) { return vec3(mat3x3_21(m),mat3x3_22(m),mat3x3_23(m)); }
static inline vec3_t mat3x3_row3(mat3x3_t m) { return vec3(mat3x3_31(m),mat3x3_32(m),mat3x3_33(m)); }

static inline vec4_t mat4x3_row1(mat4x3_t m) { return vec4(mat4x3_11(m),mat4x3_12(m),mat4x3_13(m),mat4x3_14(m)); }
static inline vec4_t mat4x3_row2(mat4x3_t m) { return vec4(mat4x3_21(m),mat4x3_22(m),mat4x3_23(m),mat4x3_24(m)); }
static inline vec4_t mat4x3_row3(mat4x3_t m) { return vec4(mat4x3_31(m),mat4x3_32(m),mat4x3_33(m),mat4x3_34(m)); }

static inline vec4_t mat4x4_row1(mat4x4_t m) { return vec4(mat4x4_11(m),mat4x4_12(m),mat4x4_13(m),mat4x4_14(m)); }
static inline vec4_t mat4x4_row2(mat4x4_t m) { return vec4(mat4x4_21(m),mat4x4_22(m),mat4x4_23(m),mat4x4_24(m)); }
static inline vec4_t mat4x4_row3(mat4x4_t m) { return vec4(mat4x4_31(m),mat4x4_32(m),mat4x4_33(m),mat4x4_34(m)); }
static inline vec4_t mat4x4_row4(mat4x4_t m) { return vec4(mat4x4_41(m),mat4x4_42(m),mat4x4_43(m),mat4x4_44(m)); }





// Upgrade constructors

static inline mat3x2_t mat3x2affine2x2(mat2x2_t m,vec2_t col3)
{
	vec2_t col1=mat2x2_col1(m),col2=mat2x2_col1(m);
	return mat3x2cols(col1,col2,col3);
}

static inline mat3x3_t mat3x3affine2x2(mat2x2_t m,vec2_t col3)
{
	vec2_t col1=mat2x2_col1(m),col2=mat2x2_col2(m);
	return mat3x3(col1.x,col2.x,col3.x,
	              col1.y,col2.y,col3.y,
	                   0,     0,     1);
}

static inline mat3x3_t mat3x3affine3x2(mat3x2_t m)
{
	vec2_t col1=mat3x2_col1(m),col2=mat3x2_col2(m),col3=mat3x2_col3(m);
	return mat3x3(col1.x,col2.x,col3.x,
	              col1.y,col2.y,col3.y,
	                   0,     0,     1);
}

static inline mat4x3_t mat4x3affine3x3(mat3x3_t m,vec3_t col4)
{
	vec3_t col1=mat3x3_col1(m),col2=mat3x3_col2(m),col3=mat3x3_col3(m);
	return mat4x3cols(col1,col2,col3,col4);
}

static inline mat4x4_t mat4x4affine3x3(mat3x3_t m,vec3_t col4)
{
	vec3_t col1=mat3x3_col1(m),col2=mat3x3_col2(m),col3=mat3x3_col3(m);
	return mat4x4(col1.x,col2.x,col3.x,col4.x,
	              col1.y,col2.y,col3.y,col4.y,
	              col1.z,col2.z,col3.z,col4.z,
	                   0,     0,     0,     1);
}

static inline mat4x4_t mat4x4affine4x3(mat4x3_t m)
{
	vec3_t col1=mat4x3_col1(m),col2=mat4x3_col2(m),col3=mat4x3_col3(m),col4=mat4x3_col4(m);
	return mat4x4(col1.x,col2.x,col3.x,col4.x,
	              col1.y,col2.y,col3.y,col4.y,
	              col1.z,col2.z,col3.z,col4.z,
	                   0,     0,     0,     1);
}



// Downgrade extractors

static inline mat2x2_t mat3x2_mat2x2(mat3x2_t m) { return mat2x2cols(mat3x2_col1(m),mat3x2_col2(m)); }
static inline mat2x2_t mat3x3_mat2x2(mat3x3_t m) { return mat2x2cols(vec3_xy(mat3x3_col1(m)),vec3_xy(mat3x3_col2(m))); }
static inline mat3x2_t mat3x3_mat3x2(mat3x3_t m) { return mat3x2cols(vec3_xy(mat3x3_col1(m)),vec3_xy(mat3x3_col2(m)),vec3_xy(mat3x3_col3(m))); }
static inline mat3x3_t mat4x3_mat3x3(mat4x3_t m) { return mat3x3cols(mat4x3_col1(m),mat4x3_col2(m),mat4x3_col3(m)); }
static inline mat3x3_t mat4x4_mat3x3(mat4x4_t m) { return mat3x3cols(vec4_xyz(mat4x4_col1(m)),vec4_xyz(mat4x4_col2(m)),vec4_xyz(mat4x4_col3(m))); }
static inline mat4x3_t mat4x4_mat4x3(mat4x4_t m) { return mat4x3cols(vec4_xyz(mat4x4_col1(m)),vec4_xyz(mat4x4_col2(m)),vec4_xyz(mat4x4_col3(m)),vec4_xyz(mat4x4_col4(m))); }




// Translation constructors

static inline mat3x2_t mat3x2translate(vec2_t v)
{
	return mat3x2(1,0,v.x,
	              0,1,v.y);
}

static inline mat4x3_t mat4x3translate(vec3_t v)
{
	return mat4x3(1,0,0,v.x,
	              0,1,0,v.y,
	              0,0,1,v.z);
}
static inline mat4x4_t mat4x4translate(vec3_t v) { return mat4x4affine4x3(mat4x3translate(v)); }



// Scaling constructors

static inline mat2x2_t mat2x2scale(float x,float y)
{
	return mat2x2(x,0,
	              0,y);
}
static inline mat3x2_t mat3x2scale(float x,float y) { return mat3x2affine2x2(mat2x2scale(x,y),vec2zero); }

static inline mat3x3_t mat3x3scale(float x,float y,float z)
{
	return mat3x3(x,0,0,
	              0,y,0,
	              0,0,z);
}
static inline mat4x3_t mat4x3scale(float x,float y,float z) { return mat4x3affine3x3(mat3x3scale(x,y,z),vec3zero); }
static inline mat4x4_t mat4x4scale(float x,float y,float z) { return mat4x4affine3x3(mat3x3scale(x,y,z),vec3zero); }



// Rotation constructors

static inline mat2x2_t mat2x2rotate(float a)
{
	return mat2x2(cosf(a),-sinf(a),
	              sinf(a), cosf(a));
}
static inline mat3x2_t mat3x2rotate(float a) { return mat3x2affine2x2(mat2x2rotate(a),vec2zero); }

static inline mat3x3_t mat3x3rotatex(float a)
{
	return mat3x3(1,      0,       0,
	              0,cosf(a),-sinf(a),
	              0,sinf(a), cosf(a));
}
static inline mat4x3_t mat4x3rotatex(float a) { return mat4x3affine3x3(mat3x3rotatex(a),vec3zero); }
static inline mat4x4_t mat4x4rotatex(float a) { return mat4x4affine3x3(mat3x3rotatex(a),vec3zero); }

static inline mat3x3_t mat3x3rotatey(float a)
{
	return mat3x3( cosf(a), 0,sinf(a),
	                     0, 1,      0,
	              -sinf(a), 0,cosf(a));
}
static inline mat4x3_t mat4x3rotatey(float a) { return mat4x3affine3x3(mat3x3rotatey(a),vec3zero); }
static inline mat4x4_t mat4x4rotatey(float a) { return mat4x4affine3x3(mat3x3rotatey(a),vec3zero); }

static inline mat3x3_t mat3x3rotatez(float a)
{
	return mat3x3(cosf(a),-sinf(a),0,
	              sinf(a), cosf(a),0,
	                    0,       0,1);
}
static inline mat4x3_t mat4x3rotatez(float a) { return mat4x3affine3x3(mat3x3rotatez(a),vec3zero); }
static inline mat4x4_t mat4x4rotatez(float a) { return mat4x4affine3x3(mat3x3rotatez(a),vec3zero); }

mat3x3_t mat3x3rotate(float angle,vec3_t axis);
static inline mat4x3_t mat4x3rotate(float angle,vec3_t axis) { return mat4x3affine3x3(mat3x3rotate(angle,axis),vec3zero); }
static inline mat4x4_t mat4x4rotate(float angle,vec3_t axis) { return mat4x4affine3x3(mat3x3rotate(angle,axis),vec3zero); }



// Lookat constructors

mat3x3_t mat3x3inverselookat(vec3_t eye,vec3_t center,vec3_t up);
static inline mat4x3_t mat4x3inverselookat(vec3_t eye,vec3_t center,vec3_t up) { return mat4x3affine3x3(mat3x3inverselookat(eye,center,up),eye); }
static inline mat4x4_t mat4x4inverselookat(vec3_t eye,vec3_t center,vec3_t up) { return mat4x4affine3x3(mat3x3inverselookat(eye,center,up),eye); }

static inline mat3x3_t mat3x3transpose(mat3x3_t m);
vec3_t mat3x3transform(mat3x3_t m,vec3_t v);

static inline mat3x3_t mat3x3lookat(vec3_t eye,vec3_t center,vec3_t up)  { return mat3x3transpose(mat3x3inverselookat(eye,center,up)); }
static inline mat4x3_t mat4x3lookat(vec3_t eye,vec3_t center,vec3_t up) { mat3x3_t m=mat3x3lookat(eye,center,up); return mat4x3affine3x3(m,mat3x3transform(m,eye)); }
static inline mat4x4_t mat4x4lookat(vec3_t eye,vec3_t center,vec3_t up) { mat3x3_t m=mat3x3lookat(eye,center,up); return mat4x4affine3x3(m,mat3x3transform(m,eye)); }




// Perspective constructors

static inline mat4x4_t mat4x4perspectiveinternal(float fx,float fy,float znear,float zfar)
{
	return mat4x4(fx, 0,                        0,                        0,
	               0,fy,                        0,                        0,
				   0, 0,(zfar+znear)/(znear-zfar),2*zfar*znear/(znear-zfar),
				   0, 0,                       -1,                        0);
}

static inline mat4x4_t mat4x4horizontalperspective(float fovx,float aspect,float znear,float zfar)
{
	float f=1/tan(fovx*M_PI/180/2);
	return mat4x4perspectiveinternal(f,f*aspect,znear,zfar);
}

static inline mat4x4_t mat4x4verticalperspective(float fovy,float aspect,float znear,float zfar)
{
	float f=1/tan(fovy*M_PI/180/2);
	return mat4x4perspectiveinternal(f/aspect,f,znear,zfar);
}

static inline mat4x4_t mat4x4minperspective(float fov,float aspect,float znear,float zfar)
{
	if(aspect<1) return mat4x4horizontalperspective(fov,aspect,znear,zfar);
	else return mat4x4verticalperspective(fov,aspect,znear,zfar);
}

static inline mat4x4_t mat4x4maxperspective(float fov,float aspect,float znear,float zfar)
{
	if(aspect>1) return mat4x4horizontalperspective(fov,aspect,znear,zfar);
	else return mat4x4verticalperspective(fov,aspect,znear,zfar);
}

static inline mat4x4_t mat4x4diagonalperspective(float fov,float aspect,float znear,float zfar)
{
	float f=1/tan(fov*M_PI/180/2);
	return mat4x4perspectiveinternal(f*sqrtf(1/(aspect*aspect)+1),f*sqrtf(aspect*aspect+1),znear,zfar);
}




// Prespective extractors

static inline vec4_t mat4x4_leftplane(mat4x4_t m) { return vec4add(mat4x4_row4(m),mat4x4_row1(m)); }
static inline vec4_t mat4x4_rightplane(mat4x4_t m) { return vec4sub(mat4x4_row4(m),mat4x4_row1(m)); }
static inline vec4_t mat4x4_bottomplane(mat4x4_t m) { return vec4add(mat4x4_row4(m),mat4x4_row2(m)); }
static inline vec4_t mat4x4_topplane(mat4x4_t m) { return vec4sub(mat4x4_row4(m),mat4x4_row2(m)); }
static inline vec4_t mat4x4_nearplane(mat4x4_t m) { return vec4add(mat4x4_row4(m),mat4x4_row3(m)); }
static inline vec4_t mat4x4_farplane(mat4x4_t m) { return vec4sub(mat4x4_row4(m),mat4x4_row3(m)); }

mat4x3_t mat4x3affineinverse(mat4x3_t m);
static inline vec3_t mat4x4_cameraposition(mat4x4_t m) { return mat4x3_col4(mat4x3affineinverse(mat4x3rows(mat4x4_row1(m),mat4x4_row2(m),mat4x4_row4(m)))); }



// Comparison operations

static inline bool mat2x2equal(mat2x2_t a,mat2x2_t b) { for(int i=0;i<sizeof(a.m)/sizeof(a.m[0]);i++) if(a.m[i]!=b.m[i]) return false; return true; }
static inline bool mat3x2equal(mat3x2_t a,mat3x2_t b) { for(int i=0;i<sizeof(a.m)/sizeof(a.m[0]);i++) if(a.m[i]!=b.m[i]) return false; return true; }
static inline bool mat3x3equal(mat3x3_t a,mat3x3_t b) { for(int i=0;i<sizeof(a.m)/sizeof(a.m[0]);i++) if(a.m[i]!=b.m[i]) return false; return true; }
static inline bool mat4x3equal(mat4x3_t a,mat4x3_t b) { for(int i=0;i<sizeof(a.m)/sizeof(a.m[0]);i++) if(a.m[i]!=b.m[i]) return false; return true; }
static inline bool mat4x4equal(mat4x4_t a,mat4x4_t b) { for(int i=0;i<sizeof(a.m)/sizeof(a.m[0]);i++) if(a.m[i]!=b.m[i]) return false; return true; }

static inline bool mat2x2almostequal(mat2x2_t a,mat2x2_t b,float epsilon) { for(int i=0;i<sizeof(a.m)/sizeof(a.m[0]);i++) if(fabsf(a.m[i]-b.m[i])<epsilon) return false; return true; }
static inline bool mat3x2almostequal(mat3x2_t a,mat3x2_t b,float epsilon) { for(int i=0;i<sizeof(a.m)/sizeof(a.m[0]);i++) if(fabsf(a.m[i]-b.m[i])<epsilon) return false; return true; }
static inline bool mat3x3almostequal(mat3x3_t a,mat3x3_t b,float epsilon) { for(int i=0;i<sizeof(a.m)/sizeof(a.m[0]);i++) if(fabsf(a.m[i]-b.m[i])<epsilon) return false; return true; }
static inline bool mat4x3almostequal(mat4x3_t a,mat4x3_t b,float epsilon) { for(int i=0;i<sizeof(a.m)/sizeof(a.m[0]);i++) if(fabsf(a.m[i]-b.m[i])<epsilon) return false; return true; }
static inline bool mat4x4almostequal(mat4x4_t a,mat4x4_t b,float epsilon) { for(int i=0;i<sizeof(a.m)/sizeof(a.m[0]);i++) if(fabsf(a.m[i]-b.m[i])<epsilon) return false; return true; }



// Multiplication

mat2x2_t mat2x2mul(mat2x2_t a,mat2x2_t b);
mat3x3_t mat3x3mul(mat3x3_t a,mat3x3_t b);
mat4x4_t mat4x4mul(mat4x4_t a,mat4x4_t b);

mat3x2_t mat3x2affinemul(mat3x2_t a,mat3x2_t b);
mat3x3_t mat3x3affinemul(mat3x3_t a,mat3x3_t b);
mat4x3_t mat4x3affinemul(mat4x3_t a,mat4x3_t b);
mat4x4_t mat4x4affinemul(mat4x4_t a,mat4x4_t b);




// Transpose

static inline mat2x2_t mat2x2transpose(mat2x2_t m) { return mat2x2cols(mat2x2_row1(m),mat2x2_row2(m)); }
static inline mat3x3_t mat3x3transpose(mat3x3_t m) { return mat3x3cols(mat3x3_row1(m),mat3x3_row2(m),mat3x3_row3(m)); }
static inline mat4x4_t mat4x4transpose(mat4x4_t m) { return mat4x4cols(mat4x4_row1(m),mat4x4_row2(m),mat4x4_row3(m),mat4x4_row4(m)); }



// Inverse

mat2x2_t mat2x2inverse(mat2x2_t m);
mat3x3_t mat3x3inverse(mat3x3_t m);
mat4x4_t mat4x4inverse(mat4x4_t m);

mat3x2_t mat3x2affineinverse(mat3x2_t m);
mat4x3_t mat4x3affineinverse(mat4x3_t m);
mat3x3_t mat3x3affineinverse(mat3x3_t m);
mat4x4_t mat4x4affineinverse(mat4x4_t m);



// Vector transformation

vec2_t mat2x2transform(mat2x2_t m,vec2_t v);
vec2_t mat3x2transform(mat3x2_t m,vec2_t v);
vec3_t mat3x3transform(mat3x3_t m,vec3_t v);
vec3_t mat4x3transform(mat4x3_t m,vec3_t v);
vec4_t mat4x4transform(mat4x4_t m,vec4_t v);

#endif

