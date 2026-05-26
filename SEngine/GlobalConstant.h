#ifndef GLOBALCONSTNAT_H
#define GLOBALCONSTNAT_H

#ifdef HLSL
#include "HlslCompat.h"
#else
using namespace DirectX::SimpleMath;
using namespace DirectX;
#endif

struct GlobalConstant
{
	Matrix view;
	Matrix projection;
};

struct MouseConstant
{
	float posX;
	float posY;
	float delX;
	float delY;
};

#endif
