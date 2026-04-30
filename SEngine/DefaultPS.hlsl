#include "DefaultCommon.hlsli"

float4 main(PSInput input) : SV_TARGET
{
	return g_hdrTexture.Sample(g_wrapLinearSampler, input.uv);
}	
