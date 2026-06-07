#include "CubeMapCommon.hlsli"

PSInput main( VSInput input )
{
	PSInput output;

	float3 pos = mul(float4(input.position, 1.f), gGlobalCB.view).xyz;
	output.svPosition = float4(pos, pos.z);
	output.uv = input.uv;

	return output;
}
