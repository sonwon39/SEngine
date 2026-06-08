#include "PBRCommon.hlsli"

float4 main(PSInput input) : SV_TARGET
{
	float3 lightTemp = float3(0, 100, -200);
	
	SurfaceProperties surface;

	float3 N = normalize(input.normalW);
	float3 T = input.tangentW;
	T = normalize(T - dot(N, T) * N);
	float3 B = normalize(cross(T, N));
	float3x3 nMat= float3x3(T, B, N);

	float3 normalMap = gNormal.SampleLevel(gWrapLinearSampler, input.uv, 0.f);
	normalMap = normalMap * 2.f - 1.f;
	
	float3 color = 0.f;
	float metalic = gMetallic.Sample(gWrapLinearSampler, input.uv).r;
	float3 L = normalize(lightTemp - input.worldPos);
	surface.V = normalize(gGlobalCB.cameraPos - input.worldPos);
	surface.N = normalize(mul(normalMap, nMat));
	surface.c_diff = gAlbedo.Sample(gWrapLinearSampler, input.uv).rgb;
	surface.c_spec = gRadianceIBL.SampleLevel(gWrapLinearSampler, surface.N, 3.f * metalic).rgb;
	//surface.c_spec = gRadianceIBL.SampleLevel(gWrapLinearSampler, input.normalW, 3.f).rgb;
	surface.roughness = gRoughness.Sample(gWrapLinearSampler, input.uv).r;
	surface.NoV = dot(surface.N, surface.V);
	
	//color += DiffuseIBL(surface);
	color += SpecularIBL(surface);
	
	return float4(surface.c_spec, 1.f);
}
