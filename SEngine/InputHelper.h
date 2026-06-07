#pragma once

#include <map>
#include "directxtk12\SimpleMath.h"

class InputHelper {
public:
	InputHelper();
	virtual ~InputHelper();

public:
	void Initialize();

	void SetKeyState(uint32_t key, bool newState);
	bool GetKeyState(uint32_t key);

	DirectX::SimpleMath::Vector3 GetInputDirection();

private:
	std::map<uint32_t, bool> m_keyDownState;

private:
	uint32_t forwardKey = 'W';
	uint32_t backwardKey = 'S';
	uint32_t rightKey = 'D';
	uint32_t leftKey = 'A';
	uint32_t upKey = 'E';
	uint32_t downKey = 'Q';

	DirectX::SimpleMath::Vector3 forwardDir = DirectX::SimpleMath::Vector3(0, 0, 1);
	DirectX::SimpleMath::Vector3 backwardDir = DirectX::SimpleMath::Vector3(0, 0, -1);
	DirectX::SimpleMath::Vector3 rightDir = DirectX::SimpleMath::Vector3(1, 0, 0);
	DirectX::SimpleMath::Vector3 leftDir = DirectX::SimpleMath::Vector3(-1, 0, 0);
};
