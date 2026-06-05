#pragma once

#include "directxtk12\SimpleMath.h"
#include "wrl.h"
#include "d3d12.h"

#include "GlobalConstant.h"

class Camera
{
public:
	Camera(int width,int height);

public:
	void Initialize();
	void OnResize(int width, int height);
	void Tick(float deltaTime);

public:
	D3D12_GPU_VIRTUAL_ADDRESS GetGlboalConstant() { return m_globalCB->GetGPUVirtualAddress(); }

private:
	float m_fovDegrees = 70.f;
	float m_fovRadians;
	float m_aspectRatio;
	int m_width;
	int m_height;
	float m_nearZ = 0.1f;
	float m_farZ = 100.f;

	DirectX::SimpleMath::Vector3 m_upDir;
	DirectX::SimpleMath::Vector3 m_eyePosition;
	DirectX::SimpleMath::Vector3 m_forwardDir;

	//DirectX::SimpleMath::Matrix m_projectionMat;
	//DirectX::SimpleMath::Matrix m_viewMat;
	DirectX::SimpleMath::Matrix perspectiveMat;
	DirectX::SimpleMath::Matrix orthographicMat;
	bool isPerspectiveMode = true;

private:
	GlobalConstant m_globalConstant;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_globalCB;
	void* pGlobalCB;
};
