#include "PBRCommon.hlsli"

PSInput main(VSInput input)
{
	PSInput output;
    
	float3 pos = input.position;
	float4 uv = float4(input.uv, 0.f, 1.f);
	uv = mul(uv, gMaterial.texTransform);
	
	pos += gMaterial.heightScale * g_height.SampleLevel(g_wrapLinearSampler, uv.xy, 0.f).x * input.normal;
	output.worldPos = pos;
	
	float4 svPos = mul(float4(pos, 1.f), gLocalCB.model);
	svPos = mul(svPos, gGlobalCB.view);
	svPos = mul(svPos, gGlobalCB.projection);
    
	float3 N = input.normal;
	float3 T = input.tangent;
	T = normalize(T - dot(T, N) * N);
	float3 B = cross(T, N);
	normalize(B);

	float3 normalMap = g_normal.SampleLevel(g_wrapLinearSampler, uv.xy, 0.f).xyz;
	
	float3x3 normalTransform = float3x3(T,B,N);
	output.normal = mul(normalMap, normalTransform);
	
	output.svPosition = svPos;
	output.uv = uv.xy;
    
	return output;
}
