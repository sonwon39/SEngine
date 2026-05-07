#ifndef PARTICLE_H
#define PARTICLE_H

#ifdef HLSL
#include "HlslCompat.h"
#else
using namespace DirectX::SimpleMath;
using namespace DirectX;
#endif

struct Particle
{
	XMFLOAT3 pos;
	XMFLOAT3 color;
};

struct ParticleLocalConstant
{
	Matrix model;
};

#endif
