#define HLSL
#include "NoiseLocalConstant.h"

float2 hash(float2 p)
{
	p = p + float2(34.1321, 45.659);
	p = sin(float2(dot(p, float2(42.12314, 35.78911)),
			dot(p, float2(93.22345, 18.39531))));
	return -1.f + 2.f * frac(p * 53249.143116);
}

float perlinNoise(float2 p)
{
	float2 o = floor(p);
	float2 f = frac(p);
	float2 u = smoothstep(0., 1., f);

	return lerp(
			lerp(dot(hash(o + float2(0., 0.)), f - float2(0., 0.)), dot(hash(o + float2(1., 0.)), f - float2(1., 0.)), u.x),
			lerp(dot(hash(o + float2(0., 1.)), f - float2(0., 1.)), dot(hash(o + float2(1., 1.)), f - float2(1., 1.)), u.x),
		u.y);
}

float2 getCurl(float2 p, float2 dx)
{
	float2 left = p - float2(dx.x, 0.f);
	float2 right = p + float2(dx.x, 0.f);
	float2 up = p + float2(0.f, dx.y);
	float2 down = p - float2(0.f, dx.y);
	

	float ln = perlinNoise(left);
	float rn = perlinNoise(right);
	float un = perlinNoise(up);
	float dn = perlinNoise(down);

	return float2((un - dn)/ dx.y, -(rn - ln) / dx.x);

}
struct GSInput
{
	float3 pos : POSITION;
	float radius : PSIZE;
};

struct PSInput
{
	float4 pos : SV_POSITION;
	float radius : PSIZE;
};
