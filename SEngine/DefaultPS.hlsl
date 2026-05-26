#include "DefaultCommon.hlsli"

float4 main(PSInput input) : SV_TARGET
{
	if(input.uv.x < 0.5f)
		return float4(1, 0, 0, 1);
	else
		return g_hdrTexture.Sample(g_wrapLinearSampler, input.uv);

}	
