#define HLSL
#include "../GlobalConstant.h"
#include "PBRHLSLCompat.h"

Texture2D						 gAlbedo			: register(t0);
Texture2D						 gAmbient			: register(t1);
Texture2D						 gHeight			: register(t2);
Texture2D						 gMetallic			: register(t3);
Texture2D						 gNormal			: register(t4);
Texture2D						 gRoughness			: register(t5);
								 
Texture2D						 gBRDF				: register(t6);
TextureCube						 gIrradianceIBL		: register(t7);
TextureCube						 gIBL				: register(t8);
TextureCube						 gRadianceIBL     : register(t9);

ConstantBuffer<GlobalConstant>	 gGlobalCB			: register(b0);
ConstantBuffer<LocalConstant>	 gLocalCB			: register(b1);
ConstantBuffer<MaterialConstant> gMaterial			: register(b2);

SamplerState 					 gWrapLinearSampler : register(s0);

struct VSInput
{
	float3 position	: POSITION;
	float3 normal	: NORMAL;
	float3 tangent  : TANGENT;
	float2 uv		: TEXCOORD;
};

struct PSInput
{
	float4 svPosition   : SV_Position;
	float3 worldPos		: Position;
	float3 normalW		: NORMAL;
	float3 tangentW		: TANGENT;
	float2 uv 			: TEXCOORD;
};
struct SurfaceProperties
{
    float3 N;
    float3 V;
    float3 c_diff;
    float3 c_spec;
    float roughness;
    float NoV;
};

float Pow5(float x)
{
    float xSq = x * x;
    return xSq * xSq * x;
}

float3 Fresnel_Schlick(float3 F0, float3 F90, float cos)
{
	return lerp( F0 , F90,  Pow5(1- cos));
}

float3 DiffuseIBL(SurfaceProperties surface)
{
	return surface.c_diff * gIrradianceIBL.SampleLevel(gWrapLinearSampler, surface.N, 0.f).rgb;
}

float3 SpecularIBL(SurfaceProperties surface)
{
	float NoV = dot(surface.N, surface.V);
	float2 EnvBRDF = gBRDF.SampleLevel(gWrapLinearSampler,float2(surface.roughness, surface.NoV) , 0.f).rg;
	return (surface.c_spec * EnvBRDF.x + EnvBRDF.y);
}
