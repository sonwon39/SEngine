#include "DefaultCommon.hlsli"

float4 main(PSInput input) : SV_TARGET
{
	if(input.uv.x > 0.5f)
		return float4(1.0f, 1.0f, 1.0f, 1.0f);
	else
		return float4(1.0f, 0.0f, 0.0f, 1.0f);
		
}	