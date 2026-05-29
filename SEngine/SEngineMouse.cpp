#include "SEngineMouse.h"
#include "GraphicsCommon.h"
#include "World.h"

#include <iostream>

using namespace Graphics;

SEngineMouse::SEngineMouse()
{
}

SEngineMouse::~SEngineMouse()
{
}

void SEngineMouse::Initilize()
{
	mouseCB.Initialize({});
}

void SEngineMouse::UpdateLButtonDownState(bool newState)
{
	lButtonDown = newState;
	mouseCB.localConstant.lButtonDown = newState;

	if (newState)
	{
		lBFlag = true;
	}
	//std::cout << "UpdateLButtonDownState " << newState << '\n';
}

void SEngineMouse::ConsumeRawDelta()
{
	mouseCB.localConstant.delX = lastX;
	mouseCB.localConstant.delY = lastY;

	lastX = 0;
	lastY = 0;
}

void SEngineMouse::Tick(float deltaTime)
{
	POINT mousePos;
	GetCursorPos(&mousePos);
	ScreenToClient(m_world->m_mainWnd, &mousePos);

	int width = m_world->windowWidth;
	int height = m_world->windowHeight;

	float x = (mousePos.x + 0.5f) / width;
	float y = (mousePos.y + 0.5f) / height;

	currPos = Vector2(x, y);
	currPos.Clamp(Vector2::Zero, Vector2(1, 1));

	mouseCB.localConstant.posX = mousePos.x;
	mouseCB.localConstant.posY = mousePos.y;

	if (lBFlag)
	{
		lBFlag = false;
		prevPos = currPos;
	}

	Vector2 velocity = (currPos - prevPos) / deltaTime;
	mouseCB.localConstant.velocity = velocity * 10.f;

	mouseCB.Update();

	prevPos = currPos;
}
