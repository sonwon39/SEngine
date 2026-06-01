#ifndef GRID_H
#define GRID_H

#ifdef HLSL
#include "../HLSLCompat.h"
#else
using namespace DirectX::SimpleMath;
using namespace DirectX;
#endif

#define SF_GROUP_SIZE_X 32
#define SF_GROUP_SIZE_Y 32

struct Grid
{
	XMFLOAT3  gGridDim;
	float     deltaTime;

	float	  h;
};

#endif
