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
    XMFLOAT2 velocity;

    bool lButtonDown;
    XMFLOAT3 color;
};

#endif
