#include "Matrix.h"

mat3x3_t mat3x3rotate(float angle,vec3_t axis)
{
	float sine=sinf(angle);
	float cosine=cosf(angle);
	float one_minus_cosine=1-cosine;

	axis=vec3norm(axis);

	return mat3x3(
	cosine + one_minus_cosine*axis.x*axis.x,
	one_minus_cosine*axis.x*axis.y + axis.z*sine,
	one_minus_cosine*axis.x*axis.z - axis.y*sine,

	one_minus_cosine*axis.x*axis.y - axis.z*sine,
	cosine + one_minus_cosine*axis.y*axis.y,
	one_minus_cosine*axis.y*axis.z + axis.x*sine,

	one_minus_cosine*axis.x*axis.z + axis.y*sine,
	one_minus_cosine*axis.y*axis.z - axis.x*sine,
	cosine + one_minus_cosine*axis.z*axis.z);
}

mat3x3_t mat3x3inverselookat(vec3_t eye,vec3_t center,vec3_t up)
{
	vec3_t backward=vec3norm(vec3sub(eye,center));
	vec3_t right=vec3norm(vec3cross(up,backward));
	vec3_t actualup=vec3norm(vec3cross(backward,right));

	return mat3x3cols(right,actualup,backward);
}

mat2x2_t mat2x2mul(mat2x2_t a,mat2x2_t b)
{
	return mat2x2(a.m[0]*b.m[0]+a.m[2]*b.m[1],
	              a.m[0]*b.m[2]+a.m[2]*b.m[3],

	              a.m[1]*b.m[0]+a.m[3]*b.m[1],
	              a.m[1]*b.m[2]+a.m[3]*b.m[3]);
}

mat3x3_t mat3x3mul(mat3x3_t a,mat3x3_t b)
{
	return mat3x3(a.m[0]*b.m[0]+a.m[3]*b.m[1]+a.m[6]*b.m[2],
	              a.m[0]*b.m[3]+a.m[3]*b.m[4]+a.m[6]*b.m[5],
	              a.m[0]*b.m[6]+a.m[3]*b.m[7]+a.m[6]*b.m[8],

	              a.m[1]*b.m[0]+a.m[4]*b.m[1]+a.m[7]*b.m[2],
	              a.m[1]*b.m[3]+a.m[4]*b.m[4]+a.m[7]*b.m[5],
	              a.m[1]*b.m[6]+a.m[4]*b.m[7]+a.m[7]*b.m[8],

	              a.m[2]*b.m[0]+a.m[5]*b.m[1]+a.m[8]*b.m[2],
	              a.m[2]*b.m[3]+a.m[5]*b.m[4]+a.m[8]*b.m[5],
	              a.m[2]*b.m[6]+a.m[5]*b.m[7]+a.m[8]*b.m[8]);
}

mat4x4_t mat4x4mul(mat4x4_t a,mat4x4_t b)
{
        mat4x4_t res;
        for(int i=0;i<16;i++)
        {
                int row=i&3,column=i&12;
                float val=0;
                for(int j=0;j<4;j++) val+=a.m[row+j*4]*b.m[column+j];
                res.m[i]=val;
        }
        return res;
}

mat3x2_t mat3x2affinemul(mat3x2_t a,mat3x2_t b)
{
	return mat3x2(a.m[0]*b.m[0]+a.m[2]*b.m[1],
	              a.m[0]*b.m[2]+a.m[2]*b.m[3],
	              a.m[0]*b.m[4]+a.m[2]*b.m[5]+a.m[4],

	              a.m[1]*b.m[0]+a.m[3]*b.m[1],
	              a.m[1]*b.m[2]+a.m[3]*b.m[3],
	              a.m[1]*b.m[4]+a.m[3]*b.m[5]+a.m[5]);
}

mat3x3_t mat3x3affinemul(mat3x3_t a,mat3x3_t b)
{
	return mat3x3(a.m[0]*b.m[0]+a.m[3]*b.m[1],
	              a.m[0]*b.m[3]+a.m[3]*b.m[4],
	              a.m[0]*b.m[6]+a.m[3]*b.m[7]+a.m[6],

	              a.m[1]*b.m[0]+a.m[4]*b.m[1],
	              a.m[1]*b.m[3]+a.m[4]*b.m[4],
	              a.m[1]*b.m[6]+a.m[4]*b.m[7]+a.m[7],

	              0,0,1);
}

mat4x3_t mat4x3affinemul(mat4x3_t a,mat4x3_t b)
{
	return mat4x3(a.m[0]*b.m[0]+a.m[3]*b.m[1]+a.m[6]*b.m[2],
	              a.m[0]*b.m[3]+a.m[3]*b.m[4]+a.m[6]*b.m[5],
	              a.m[0]*b.m[6]+a.m[3]*b.m[7]+a.m[6]*b.m[8],
	              a.m[0]*b.m[9]+a.m[3]*b.m[10]+a.m[6]*b.m[11]+a.m[9],

	              a.m[1]*b.m[0]+a.m[4]*b.m[1]+a.m[7]*b.m[2],
	              a.m[1]*b.m[3]+a.m[4]*b.m[4]+a.m[7]*b.m[5],
	              a.m[1]*b.m[6]+a.m[4]*b.m[7]+a.m[7]*b.m[8],
	              a.m[1]*b.m[9]+a.m[4]*b.m[10]+a.m[7]*b.m[11]+a.m[10],

	              a.m[2]*b.m[0]+a.m[5]*b.m[1]+a.m[8]*b.m[2],
	              a.m[2]*b.m[3]+a.m[5]*b.m[4]+a.m[8]*b.m[5],
	              a.m[2]*b.m[6]+a.m[5]*b.m[7]+a.m[8]*b.m[8],
	              a.m[2]*b.m[9]+a.m[5]*b.m[10]+a.m[8]*b.m[11]+a.m[11]);
}

mat4x4_t mat4x4affinemul(mat4x4_t a,mat4x4_t b)
{
	return mat4x4(a.m[0]*b.m[0]+a.m[4]*b.m[1]+a.m[8]*b.m[2],
	              a.m[0]*b.m[4]+a.m[4]*b.m[5]+a.m[8]*b.m[6],
	              a.m[0]*b.m[8]+a.m[4]*b.m[9]+a.m[8]*b.m[10],
	              a.m[0]*b.m[12]+a.m[4]*b.m[13]+a.m[8]*b.m[14]+a.m[12],

	              a.m[1]*b.m[0]+a.m[5]*b.m[1]+a.m[9]*b.m[2],
	              a.m[1]*b.m[4]+a.m[5]*b.m[5]+a.m[9]*b.m[6],
	              a.m[1]*b.m[8]+a.m[5]*b.m[9]+a.m[9]*b.m[10],
	              a.m[1]*b.m[12]+a.m[5]*b.m[13]+a.m[9]*b.m[14]+a.m[13],

	              a.m[2]*b.m[0]+a.m[6]*b.m[1]+a.m[10]*b.m[2],
	              a.m[2]*b.m[4]+a.m[6]*b.m[5]+a.m[10]*b.m[6],
	              a.m[2]*b.m[8]+a.m[6]*b.m[9]+a.m[10]*b.m[10],
	              a.m[2]*b.m[12]+a.m[6]*b.m[13]+a.m[10]*b.m[14]+a.m[14],

	              0,0,0,1);
}

mat2x2_t mat2x2inverse(mat2x2_t m)
{
	mat2x2_t res;
	float det=m.m[0]*m.m[2]-m.m[1]*m.m[2];
	// singular if det==0

	res.m[0]=m.m[0]/det;
	res.m[2]=m.m[1]/det;

	res.m[1]=m.m[2]/det;
	res.m[3]=m.m[3]/det;

	return res;
}

mat3x3_t mat3x3inverse(mat3x3_t m)
{
	mat3x3_t res;
	float det=m.m[0]*m.m[4]*m.m[8]-m.m[0]*m.m[5]*m.m[7]+
	            m.m[1]*m.m[5]*m.m[6]-m.m[1]*m.m[3]*m.m[8]+
			    m.m[2]*m.m[3]*m.m[7]-m.m[2]*m.m[4]*m.m[6];
	// singular if det==0

	res.m[0]=(m.m[4]*m.m[8]-m.m[5]*m.m[7])/det;
	res.m[3]=-(m.m[3]*m.m[8]-m.m[5]*m.m[6])/det;
	res.m[6]=(m.m[3]*m.m[7]-m.m[4]*m.m[6])/det;

	res.m[1]=-(m.m[1]*m.m[8]-m.m[2]*m.m[7])/det;
	res.m[4]=(m.m[0]*m.m[8]-m.m[2]*m.m[6])/det;
	res.m[7]=-(m.m[0]*m.m[7]-m.m[1]*m.m[6])/det;

	res.m[2]=(m.m[1]*m.m[5]-m.m[2]*m.m[4])/det;
	res.m[5]=-(m.m[0]*m.m[5]-m.m[2]*m.m[3])/det;
	res.m[8]=(m.m[0]*m.m[4]-m.m[1]*m.m[3])/det;

	return res;
}

mat4x4_t mat4x4inverse(mat4x4_t m)
{
	mat4x4_t res;

	float a0=m.m[0]*m.m[5]-m.m[1]*m.m[4];
	float a1=m.m[0]*m.m[6]-m.m[2]*m.m[4];
	float a2=m.m[0]*m.m[7]-m.m[3]*m.m[4];
	float a3=m.m[1]*m.m[6]-m.m[2]*m.m[5];
	float a4=m.m[1]*m.m[7]-m.m[3]*m.m[5];
	float a5=m.m[2]*m.m[7]-m.m[3]*m.m[6];
	float b0=m.m[8]*m.m[13]-m.m[9]*m.m[12];
	float b1=m.m[8]*m.m[14]-m.m[10]*m.m[12];
	float b2=m.m[8]*m.m[15]-m.m[11]*m.m[12];
	float b3=m.m[9]*m.m[14]-m.m[10]*m.m[13];
	float b4=m.m[9]*m.m[15]-m.m[11]*m.m[13];
	float b5=m.m[10]*m.m[15]-m.m[11]*m.m[14];
	float det=a0*b5-a1*b4+a2*b3+a3*b2-a4*b1+a5*b0;
	// singular if det==0

	res.m[0]=(m.m[5]*b5-m.m[6]*b4+m.m[7]*b3)/det;
	res.m[4]=-(m.m[4]*b5-m.m[6]*b2+m.m[7]*b1)/det;
	res.m[8]=(m.m[4]*b4-m.m[5]*b2+m.m[7]*b0)/det;
	res.m[12]=-(m.m[4]*b3-m.m[5]*b1+m.m[6]*b0)/det;

	res.m[1]=-(m.m[1]*b5-m.m[2]*b4+m.m[3]*b3)/det;
	res.m[5]=(m.m[0]*b5-m.m[2]*b2+m.m[3]*b1)/det;
	res.m[9]=-(m.m[0]*b4-m.m[1]*b2+m.m[3]*b0)/det;
	res.m[13]=(m.m[0]*b3-m.m[1]*b1+m.m[2]*b0)/det;

	res.m[2]=(m.m[13]*a5-m.m[14]*a4+m.m[15]*a3)/det;
	res.m[6]=-(m.m[12]*a5-m.m[14]*a2+m.m[15]*a1)/det;
	res.m[10]=(m.m[12]*a4-m.m[13]*a2+m.m[15]*a0)/det;
	res.m[14]=-(m.m[12]*a3-m.m[13]*a1+m.m[14]*a0)/det;

	res.m[3]=-(m.m[9]*a5-m.m[10]*a4+m.m[11]*a3)/det;
	res.m[7]=(m.m[8]*a5-m.m[10]*a2+m.m[11]*a1)/det;
	res.m[11]=-(m.m[8]*a4-m.m[9]*a2+m.m[11]*a0)/det;
	res.m[15]=(m.m[8]*a3-m.m[9]*a1+m.m[10]*a0)/det;

	return res;
}

mat3x3_t mat3x3affineinverse(mat3x3_t m)
{
	mat3x3_t res;
	float det=m.m[0]*m.m[3]-m.m[1]*m.m[4];
	// singular if det==0

	res.m[0]=m.m[0]/det;
	res.m[3]=m.m[1]/det;

	res.m[1]=m.m[3]/det;
	res.m[4]=m.m[4]/det;

	res.m[2]=0;
	res.m[6]=0;

	res.m[6]=-(m.m[6]*res.m[0]+m.m[7]*res.m[3]);
	res.m[7]=-(m.m[6]*res.m[1]+m.m[7]*res.m[4]);
	res.m[8]=1;

	return res;
}

mat3x2_t mat3x2affineinverse(mat3x2_t m)
{
	mat3x2_t res;
	float det=m.m[0]*m.m[2]-m.m[1]*m.m[2];
	// singular if det==0

	res.m[0]=m.m[0]/det;
	res.m[2]=m.m[1]/det;

	res.m[1]=m.m[2]/det;
	res.m[3]=m.m[3]/det;

	res.m[4]=-(m.m[4]*res.m[0]+m.m[5]*res.m[2]);
	res.m[5]=-(m.m[4]*res.m[1]+m.m[5]*res.m[3]);

	return res;
}

mat4x3_t mat4x3affineinverse(mat4x3_t m)
{
	mat4x3_t res;
	float det=m.m[0]*m.m[4]*m.m[8]-m.m[0]*m.m[5]*m.m[7]+
	            m.m[1]*m.m[5]*m.m[6]-m.m[1]*m.m[3]*m.m[8]+
			    m.m[2]*m.m[3]*m.m[7]-m.m[2]*m.m[4]*m.m[6];
	// singular if det==0

	res.m[0]=(m.m[4]*m.m[8]-m.m[5]*m.m[7])/det;
	res.m[3]=-(m.m[3]*m.m[8]-m.m[5]*m.m[6])/det;
	res.m[6]=(m.m[3]*m.m[7]-m.m[4]*m.m[6])/det;

	res.m[1]=-(m.m[1]*m.m[8]-m.m[2]*m.m[7])/det;
	res.m[4]=(m.m[0]*m.m[8]-m.m[2]*m.m[6])/det;
	res.m[7]=-(m.m[0]*m.m[7]-m.m[1]*m.m[6])/det;

	res.m[2]=(m.m[1]*m.m[5]-m.m[2]*m.m[4])/det;
	res.m[5]=-(m.m[0]*m.m[5]-m.m[2]*m.m[3])/det;
	res.m[8]=(m.m[0]*m.m[4]-m.m[1]*m.m[3])/det;

	res.m[9]=-(m.m[9]*res.m[0]+m.m[10]*res.m[3]+m.m[11]*res.m[6]);
	res.m[10]=-(m.m[9]*res.m[1]+m.m[10]*res.m[4]+m.m[11]*res.m[7]);
	res.m[11]=-(m.m[9]*res.m[2]+m.m[10]*res.m[5]+m.m[11]*res.m[8]);


	return res;
}

mat4x4_t mat4x4affineinverse(mat4x4_t m)
{
	mat4x4_t res;
	float det=m.m[0]*m.m[5]*m.m[10]-m.m[0]*m.m[6]*m.m[9]+
	            m.m[1]*m.m[6]*m.m[8]-m.m[1]*m.m[4]*m.m[10]+
			    m.m[2]*m.m[4]*m.m[9]-m.m[2]*m.m[5]*m.m[8];
	// singular if det==0

	res.m[0]=(m.m[5]*m.m[10]-m.m[6]*m.m[9])/det;
	res.m[4]=-(m.m[4]*m.m[10]-m.m[6]*m.m[8])/det;
	res.m[8]=(m.m[4]*m.m[9]-m.m[5]*m.m[8])/det;

	res.m[1]=-(m.m[1]*m.m[10]-m.m[2]*m.m[9])/det;
	res.m[5]=(m.m[0]*m.m[10]-m.m[2]*m.m[8])/det;
	res.m[9]=-(m.m[0]*m.m[9]-m.m[1]*m.m[8])/det;

	res.m[2]=(m.m[1]*m.m[6]-m.m[2]*m.m[5])/det;
	res.m[6]=-(m.m[0]*m.m[6]-m.m[2]*m.m[4])/det;
	res.m[10]=(m.m[0]*m.m[5]-m.m[1]*m.m[4])/det;

	res.m[3]=0;
	res.m[7]=0;
	res.m[11]=0;

	res.m[12]=-(m.m[12]*res.m[0]+m.m[13]*res.m[4]+m.m[14]*res.m[8]);
	res.m[13]=-(m.m[12]*res.m[1]+m.m[13]*res.m[5]+m.m[14]*res.m[9]);
	res.m[14]=-(m.m[12]*res.m[2]+m.m[13]*res.m[6]+m.m[14]*res.m[10]);
	res.m[15]=1;

	return res;
}

vec2_t mat2x2transform(mat2x2_t m,vec2_t v)
{
	return vec2(
	v.x*m.m[0]+v.y*m.m[2],
	v.x*m.m[1]+v.y*m.m[3]);
}

vec2_t mat3x2transform(mat3x2_t m,vec2_t v)
{
	return vec2(
	v.x*m.m[0]+v.y*m.m[2]+m.m[4],
	v.x*m.m[1]+v.y*m.m[3]+m.m[5]);
}

vec3_t mat3x3transform(mat3x3_t m,vec3_t v)
{
	return vec3(
	v.x*m.m[0]+v.y*m.m[3]+v.z*m.m[6],
	v.x*m.m[1]+v.y*m.m[4]+v.z*m.m[7],
	v.x*m.m[2]+v.y*m.m[5]+v.z*m.m[8]);
}

vec3_t mat4x3transform(mat4x3_t m,vec3_t v)
{
	return vec3(
	v.x*m.m[0]+v.y*m.m[3]+v.z*m.m[6]+m.m[9],
	v.x*m.m[1]+v.y*m.m[4]+v.z*m.m[7]+m.m[10],
	v.x*m.m[2]+v.y*m.m[5]+v.z*m.m[8]+m.m[11]);
}

vec4_t mat4x4transform(mat4x4_t m,vec4_t v)
{
	return vec4(
	v.x*m.m[0]+v.y*m.m[4]+v.z*m.m[8]+v.w*m.m[12],
	v.x*m.m[1]+v.y*m.m[5]+v.z*m.m[9]+v.w*m.m[13],
	v.x*m.m[2]+v.y*m.m[6]+v.z*m.m[10]+v.w*m.m[14],
	v.x*m.m[3]+v.y*m.m[7]+v.z*m.m[11]+v.w*m.m[15]);
}

