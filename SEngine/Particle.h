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

struct SPHParticle {
	XMFLOAT3 position;
	float density;
	XMFLOAT3 color;
	float pressure;
	XMFLOAT3 velocity;
	float radius;
	XMFLOAT3 acceleration;
};

struct ParticleLocalConstant
{
	Matrix model;
};

struct SPHParticleLocalConstant
{
	int particleCount;
	float kernelCoefficient;
	float h;
	float hd; // 1 / pow(h, d)
	float rho0;
	float k;
	float dt;
	float mu;
};
#endif
