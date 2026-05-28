#ifndef GRID_H
#define GRID_H

#ifdef HLSL
#include "HlslCompat.h"
#else
using namespace DirectX::SimpleMath;
using namespace DirectX;
#endif

#define SF_GROUP_SIZE 256

struct Grid
{
	XMFLOAT3  gGridDim;
	float     deltaTime;

	float	  h;
};

#endif
