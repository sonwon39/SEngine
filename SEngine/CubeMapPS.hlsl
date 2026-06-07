#include "CubeMapCommon.hlsli"

float4 main(PSInput input) : SV_TARGET
{
	return g_cubeMap.Sample(g_wrapLinearSampler, input.uv);
}
