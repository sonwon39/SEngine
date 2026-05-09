#include "ParticleCommon.hlsli"

float4 main(PSInput input) : SV_TARGET
{
	float d = distance(float2(0.5f,0.5f), input.uv);
	
	float pow = smoothstep(0, 0.5, d);
	pow = 1 - pow;
	return float4(pow *input.color, 1.f);
}
