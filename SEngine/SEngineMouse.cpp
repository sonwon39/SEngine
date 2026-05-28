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

void SEngineMouse::Tick()
{
	POINT mousePos;
	GetCursorPos(&mousePos);
	ScreenToClient(m_world->m_mainWnd, &mousePos);
		
	mouseCB.localConstant.posX = mousePos.x;
	mouseCB.localConstant.posY = mousePos.y;
	if (lBFlag)
	{
		lBFlag = false;
		mouseCB.localConstant.prevPosX = mousePos.x;
		mouseCB.localConstant.prevPosY = mousePos.y;
	}
	ConsumeRawDelta();

	//std::cout << "Mouse Tick  " << mousePos.x << ' ' << mousePos.y << '\n';
	mouseCB.Update();

	mouseCB.localConstant.prevPosX = mousePos.x;
	mouseCB.localConstant.prevPosY = mousePos.y;
}
