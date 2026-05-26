#include "SEngineMouse.h"
#include "GraphicsCommon.h"
#include "World.h"

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
	ConsumeRawDelta();

	mouseCB.Update();
}
