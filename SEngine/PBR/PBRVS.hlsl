#include "PBRCommon.hlsli"

PSInput main(VSInput input)
{
	PSInput output;
    
	float3 pos = input.position;
	output.worldPos = pos;
	
	float4 uv = float4(input.uv, 0.f, 1.f);
	uv = mul(uv, gMaterial.texTransform);
	
	pos += gMaterial.heightScale * gHeight.SampleLevel(gWrapLinearSampler, uv.xy, 0.f).x * input.normal;
	
	
	float4 svPos = mul(float4(pos, 1.f), gLocalCB.model);
	svPos = mul(svPos, gGlobalCB.view);
	svPos = mul(svPos, gGlobalCB.projection);
    
	output.normalW = normalize(input.normal);
	output.tangentW = normalize(input.tangent);
	
	output.svPosition = svPos;
	output.uv = uv.xy;
    
	return output;
}
