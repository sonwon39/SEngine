#include "StableFluidsUtility.hlsli"
#include "../GlobalConstant.h"

RWTexture2D<float4> gDensity : register(u0);
RWTexture2D<float4> gVelocity : register(u1);

ConstantBuffer<MouseConstant> gMouse : register(b1);

float smootherstep(float x, float edge0 = 0.0f, float edge1 = 1.0f)
{
    x = clamp((x - edge0) / (edge1 - edge0), 0, 1);
    return x * x * x * (3 * x * (2 * x - 5) + 10.0f);
}

[numthreads(SF_GROUP_SIZE_X, SF_GROUP_SIZE_Y, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    float brushRadius = 50.f;
    uint width, height;
    gDensity.GetDimensions(width, height);

    if (DTid.x >= width || DTid.y >= height)
        return;

    gDensity[DTid.xy] = max(0.0, gDensity[DTid.xy] - 0.001);

    if (gMouse.lButtonDown)
    {
        float deltaTime = gLocalCB.deltaTime;
        float sigma = brushRadius / 2.5f;

        float2 pixelPos = DTid.xy;

        float2 mouseCurrPos = float2(gMouse.posX, gMouse.posY);
        float2 velocity = gMouse.velocity;

        float d = distance(pixelPos, mouseCurrPos) / brushRadius;
        float scale = smootherstep(1.0 - d);

        // scale = exp(-(d * d) / (sigma * sigma));

        float3 color = gMouse.color;
        gDensity[DTid.xy].xyz += scale * color;
        gVelocity[DTid.xy].xy += scale * velocity;
    }
}
