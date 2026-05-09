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

	float4 points[] =
	{
		center + float4(r, -r, 0, 0),
		center + float4(-r, -r, 0, 0),
		center + float4(-r, r, 0, 0),
		center + float4(r, r, 0, 0)
	};
	float2 uvs[] =
	{
		float2(1, 1),
		float2(0, 1),
		float2(0, 0),
		float2(1, 0)
	};
	uint indices[] =
	{
		0,1,2,0,2,3
	};

	for (uint i = 0; i < 4; i++)
	{
		points[i] = mul(points[i], g_globalConstant.view);
		points[i] = mul(points[i], g_globalConstant.projection);
	}
	for (uint j = 0; j < 6; j++)
	{
		uint idx = indices[j];
		
		element.pos = points[idx];
		element.uv = uvs[idx];
		output.Append(element);

		if ((j + 1)  % 3 == 0)
		{
			output.RestartStrip();
		}
	}	
}
