#include "PBRCommon.hlsli"

float4 main(PSInput input) : SV_TARGET
{
	float3 lightTemp = float3(0, 100, -200);
	float3 L = normalize(lightTemp - input.worldPos);
	float3 V = normalize(gGlobalCB.cameraPos - input.worldPos);
	float3 N = input.normal;
	float3 R = reflect(-L, N);
	float diffuse = clamp(dot(L, N), 0.f, 1.f);
	float3 albedo = g_albedo.Sample(g_wrapLinearSampler, input.uv).xyz;
	float specular = pow(clamp(dot(V, R), 0.f, 1.f), 20.f);
	
	albedo *= diffuse;
	
	return float4(albedo + specular, 1.f);
}
