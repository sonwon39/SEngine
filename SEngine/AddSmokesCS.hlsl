#include "StableFluidsUtility.hlsli"
#include "GlobalConstant.h"

RWTexture2D<float4> gDensity : register(u0);
RWTexture2D<float4> gVelocity : register(u1);

ConstantBuffer<MouseConstant> gMouse : register(b1);


[numthreads(SF_GROUP_SIZE, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	float brushRadius = 50.f;
	uint width, height;
	gDensity.GetDimensions(width, height);

	if (DTid.x >= width || DTid.y >= height)
		return;
	
	if (gMouse.lButtonDown)
	{
		float deltaTime = gLocalCB.deltaTime;
		float sigma = brushRadius / 2.5f;
		
		float2 pixelPos = float2(DTid.xy);
		float2 mouseCurrPos = float2(gMouse.posX, gMouse.posY);
		float2 mousePrevPos = float2(gMouse.prevPosX, gMouse.prevPosY);
		
		float2 velocity = (mouseCurrPos - mousePrevPos) / deltaTime;

		float d = distance(pixelPos, mouseCurrPos);
		float power = exp(-(d * d) / (sigma * sigma));

		gDensity[DTid.xy].xyz += power * float3(1, 0, 0);
		gVelocity[DTid.xy].xy += power * velocity * 10.f;
		
	}
	
}
