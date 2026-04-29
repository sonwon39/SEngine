#include "DefaultCommon.hlsli"

PSInput main(VSInput input)
{
    PSInput output;
    
    float3 pos = input.position;
    float4 svPos = mul(float4(pos, 1.f), gLocalCB.model);
    svPos = mul(svPos, gLocalCB.view);
    svPos = mul(svPos, gLocalCB.projection);
    
    output.svPosition = svPos;
    output.uv = input.uv;
    
    return output;
}