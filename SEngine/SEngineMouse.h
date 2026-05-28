#pragma once

#include "directxtk12\SimpleMath.h"
#include "GlobalConstant.h"
#include "ConstantBuffer.h"


class SEngineMouse {
public:
	SEngineMouse();
	virtual ~SEngineMouse();

public:
	// device 생성 후 constant buffer 생성
	void Initilize();

	// constantbuffer 업데이트
	void Tick(float deltaTime);

public:
	void UpdateLButtonDownState(bool newState);
	void AddRawDelta(long x, long y)
	{
		lastX += x;
		lastY += y;
	}
	void UpdatePosition(int x, int y) { posX = x; posY = y; }
	void ConsumeRawDelta();

public:
	void Tick();
	ConstantBuffer<MouseConstant> mouseCB;

public:
	bool lButtonDown = false;
	long lastX = 0;
	long lastY = 0;

	long posX = 0;
	long posY = 0;

	// lButton 첫 클릭 여부 확인 flag
	// 첫 클릭일 시 tick에서 prevpos를 현재 위치로 업데이트
	bool lBFlag = false;
};
