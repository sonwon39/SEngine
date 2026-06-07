#include "SEngineMouse.h"
#include "GraphicsCommon.h"
#include "Engine/World.h"

#include <iostream>
#include <algorithm>

using namespace Graphics;
using namespace DirectX::SimpleMath;

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


void SEngineMouse::Tick(float deltaTime)
{
	static int i = 0;

	POINT mousePos;
	GetCursorPos(&mousePos);
	ScreenToClient(m_world->m_mainWnd, &mousePos);

	int width = m_world->windowWidth;
	int height = m_world->windowHeight;

	float x = (mousePos.x + 0.5f) / width;
	float y = (mousePos.y + 0.5f) / height;
	float ndcX = (mousePos.x * 2.f) / width - 1.f;
	float ndcY = (-mousePos.y * 2.f) / height + 1.f;

	ndcX = std::clamp(ndcX, -1.0f, 1.0f);
	ndcY = std::clamp(ndcY, -1.0f, 1.0f);

	currPos = Vector2(mousePos.x, mousePos.y);

	currNDCPos = Vector2(ndcX, -ndcY);
	currNDCPos.Clamp(Vector2(-1,-1), Vector2(1, 1));

	mouseCB.localConstant.posX = mousePos.x;
	mouseCB.localConstant.posY = mousePos.y;

	if (lBFlag)
	{
		int index = i % m_world->colors.size();
		lBFlag = false;
		prevNDCPos = currNDCPos;
		prevPos = currPos;
		mouseCB.localConstant.color = m_world->colors[index];
		i++;
	}

	velocity = (currPos - prevPos);
	mouseCB.localConstant.velocity = (currNDCPos - prevNDCPos) * 10.f;

	mouseCB.Update();

	prevNDCPos = currNDCPos;
	prevPos = currPos;
}
