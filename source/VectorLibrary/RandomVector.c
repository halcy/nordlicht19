#include "RandomVector.h"

float RandomFloat();

vec3_t vec3cuberand()
{
	return vec3(RandomFloat()*2-1,RandomFloat()*2-1,RandomFloat()*2-1);
}

vec3_t vec3sphererand()
{
	vec3_t res;
	do { res=vec3cuberand(); } while(vec3sq(res)>1);
	return res;
}
