#include "StableFluidsUtility.hlsli"
#include "GlobalConstant.h"

RWTexture2D<float4> gColor : register(u0);
ConstantBuffer<MouseConstant> gMouse : register(b1);


[numthreads(SF_GROUP_SIZE, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	float brushRadius = 20.f;
	float brushSpacing = brushRadius*0.25f;
	uint width, height;
	gColor.GetDimensions(width, height);

	if (DTid.x >= width || DTid.y >= height)
		return;
	
	if (gMouse.lButtonDown)
	{
		float2 pixelPos = float2(DTid.xy);
		float2 mouseCurrPos = float2(gMouse.posX, gMouse.posY);
		float2 mousePrevPos = float2(gMouse.prevPosX, gMouse.prevPosY);
		
		float2 ab = mouseCurrPos - mousePrevPos;
		float2 ap = pixelPos - mousePrevPos;
		
		float  t  = saturate(dot(ap, ab) / max(dot(ab, ab), 1e-6));
		if(t<0 || t>=1)	return;
		float2 closest = mousePrevPos + t * ab;
		float  d = distance(pixelPos, closest);
		if (d < brushRadius)
		{
			float power = smoothstep(0, brushRadius, brushRadius-d);

			gColor[DTid.xy] += power * float4(1, 0, 0, 1);
			
		}
	}
	
}
