#include "InputHelper.h"
#include <iostream>

using namespace DirectX::SimpleMath;

InputHelper::InputHelper()
{
}

InputHelper::~InputHelper()
{
}

void InputHelper::Initialize()
{
	for (char c = 'A'; c <= 'Z'; c++)
	{
		m_keyDownState[c] = false;
	}
}

void InputHelper::SetKeyState(uint32_t key, bool newState)
{
	m_keyDownState[key] = newState;
}

bool InputHelper::GetKeyState(uint32_t key)
{
	auto it = m_keyDownState.find(key);
	if (it != m_keyDownState.end())
		return it->second;
	return false;
}

DirectX::SimpleMath::Vector3 InputHelper::GetInputDirection()
{
	Vector3 dir = Vector3(0.f,0.f,0.f);

	if (m_keyDownState[forwardKey])
		dir += forwardDir;

	if (m_keyDownState[backwardKey])
		dir += backwardDir;

	if (m_keyDownState[rightKey])
		dir += rightDir;

	if (m_keyDownState[leftKey])
		dir += leftDir;

	dir.Normalize();
	return dir;
}
