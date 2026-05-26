#define HLSL
#include "GlobalConstant.h"

ConstantBuffer<MouseConstant> g_localCB : register(b0);

[numthreads(256, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
}
