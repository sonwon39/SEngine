#include "ParticleCommon.hlsli"

StructuredBuffer<SPHParticle> particles : register(t0);

[maxvertexcount(6)]
void main(
	point GSInput input[1],
	inout TriangleStream<PSInput> output
)
{
	PSInput element;
	float r = input[0].radius;
	element.color = input[0].color;

	float4 center = float4(input[0].pos, 1.f);
	
	float4 p0 = center + float4(r, -r, 0, 0);
	float4 p1 = center + float4(-r, -r, 0, 0);
	float4 p2 = center + float4(-r, r, 0, 0);
	float4 p3 = center + float4(r, r, 0, 0);

	element.pos = p0;
	element.uv = float2(1, 1);
	output.Append(element);
	element.pos = p1;
	element.uv = float2(0, 1);
	output.Append(element);
	element.pos = p2;
	element.uv = float2(0, 0);
	output.Append(element);

	output.RestartStrip();
	
	element.pos = p0;
	element.uv = float2(1, 1);
	output.Append(element);
	element.pos = p2;
	element.uv = float2(0, 0);
	output.Append(element);
	element.pos = p3;
	element.uv = float2(1, 0);
	output.Append(element);

	output.RestartStrip();
	
}
