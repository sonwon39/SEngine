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
	UINT posX;
	UINT posY;

	float delX;
	float delY;

	XMFLOAT2 velocity;
	bool  lButtonDown;
	float dummy1;

	XMFLOAT3 color;
	float dummy2;
};

#endif
