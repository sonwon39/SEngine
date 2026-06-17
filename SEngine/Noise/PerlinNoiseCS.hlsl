#include "NoiseCommon.hlsli"

RWTexture2D<float4> g_noise : register(u0);

[numthreads(N_GROUP_SIZE_X, N_GROUP_SIZE_Y, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	uint width;
	uint height;
	g_noise.GetDimensions(width, height);
	if( DTid.x >= width || DTid. y >= height)
		return;

	float2 range = float2(16.xx);
	float2 uv = DTid.xy / float2(width, height) * range;
	float noise = perlinNoise(uv) * 0.5 + 0.5;
	noise = pow(noise, 2.2f);
	g_noise[DTid.xy] = float4(float3(noise.xxx), 1.);
}
