#include "Camera.h"
#include "GraphicsCommon.h"
#include <iostream>

Camera::Camera(int width, int height) :
	m_width(width),
	m_height(height)
{
}

void Camera::Initialize()
{
	using DirectX::SimpleMath::Vector3;

	m_upDir = Vector3(0, 1, 0);
	//m_eyePosition = Vector3(2, 2, -2);
	m_eyePosition = Vector3(0.f, 0.f, 0.f);
	m_forwardDir = Vector3(0.f, 0.f, 1.f);
	m_forwardDir.Normalize();

	m_globalConstant.view = DirectX::XMMatrixLookToLH(m_eyePosition,
		m_forwardDir, m_upDir);

	m_fovRadians = DirectX::XMConvertToRadians(m_fovDegrees);
	m_aspectRatio = (float)m_width / m_height;

	perspectiveMat = DirectX::XMMatrixPerspectiveFovLH(m_fovRadians,
		m_aspectRatio, m_nearZ, m_farZ);

	orthographicMat = DirectX::XMMatrixOrthographicLH(2, 2, m_nearZ, m_farZ);

	Graphics::utility->CreateConstantBuffer(sizeof(GlobalConstant), m_globalCB, &pGlobalCB);
	m_globalCB->SetName(L"Camera Global Constant Buffer");

	m_globalConstant.view = m_globalConstant.view.Transpose();
	m_globalConstant.projection = isPerspectiveMode ? perspectiveMat.Transpose() : orthographicMat.Transpose();

	memcpy(pGlobalCB, &m_globalConstant, sizeof(GlobalConstant));
}

void Camera::OnResize(int width, int height)
{
	std::cout << "Camera OnResize\n";

	m_width = width;
	m_height = height;
	m_aspectRatio = (float)m_width / m_height;

	perspectiveMat = DirectX::XMMatrixPerspectiveFovLH(m_fovRadians,
		m_aspectRatio, m_nearZ, m_farZ);

	orthographicMat = DirectX::XMMatrixOrthographicLH(2.f, 2.f, m_nearZ, m_farZ);

	m_globalConstant.projection = isPerspectiveMode ? perspectiveMat.Transpose() : orthographicMat.Transpose();

	memcpy(pGlobalCB, &m_globalConstant, sizeof(GlobalConstant));
}

void Camera::Tick(float deltaTime)
{
	memcpy(pGlobalCB, &m_globalConstant, sizeof(GlobalConstant));
}
