#ifndef LOISELOCALCONSTANT_H
#define LOISELOCALCONSTANT_H

#ifdef HLSL
#include "../HLSLCompat.h"
#else
using namespace DirectX::SimpleMath;
using namespace DirectX;
#endif

#define N_GROUP_SIZE_X 32
#define N_GROUP_SIZE_Y 32

struct NoiseParticle
{
    XMFLOAT3 position;
    float radius;
    XMFLOAT3 velocity;
    float dummy;
};
#endif
