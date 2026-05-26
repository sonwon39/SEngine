#pragma once

#include "d3d12.h"
#include <memory>
#include "SEngineMouse.h"

class World {
public:
	World();
	virtual ~World() {};

public:
	void Initialize(ID3D12Device5* device);
	void Tick(float deltaTime);

public:
	UINT m_cbvSrvDescriptorSize = 0;
	UINT m_rtvDescriptorSize = 0;
	UINT m_dsvDescriptorSize = 0;

public:
	HWND m_mainWnd;
	std::shared_ptr<SEngineMouse> mouse;

private:
	ID3D12Device5* m_device;

};
