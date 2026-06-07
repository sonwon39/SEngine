#include "CameraComponent.h"
#include "Engine/World.h"
#include "Engine/RenderEngine.h"
#include "GraphicsCommon.h"

CameraComponent::CameraComponent(Actor* owner)
    : SceneComponent(owner)
{

}

CameraComponent::~CameraComponent()
{
}

void CameraComponent::Initialize(const float& fovDegrees,
    const UINT& width, const UINT& height, const float& nearZ, const float& farZ)
{
    m_fovRadians = DirectX::XMConvertToRadians(fovDegrees);
    m_aspectRatio = (float)width / height;
    m_nearZ = nearZ;
    m_farZ = farZ;

	if (!m_gcbInitialized) {
		m_gcbInitialized = true;

		GlobalConstant gc;
		gc.projection = DirectX::XMMatrixPerspectiveFovLH(
			m_fovRadians, m_aspectRatio, m_nearZ, m_farZ);
		gc.view = XMMatrixLookToLH(GetLocation(), m_frontDirection, m_upDirection);

		gc.projection = gc.projection.Transpose();
		gc.view = gc.view.Transpose();
		m_gcb.Initialize(gc);
	}
	else
	{
		Matrix projection = DirectX::XMMatrixPerspectiveFovLH(
			m_fovRadians, m_aspectRatio, m_nearZ, m_farZ);
		Matrix view = XMMatrixLookToLH(GetLocation(), m_frontDirection, m_upDirection);

		m_gcb.localConstant.projection = projection.Transpose();
		m_gcb.localConstant.view = view.Transpose();
	}
	m_gcb.Update();
}

void CameraComponent::UpdateCameraInfo(const int& width, const int& height)
{
	if (!m_gcbInitialized)
		return;
    m_aspectRatio = float(width) / height;
	Matrix projection = DirectX::XMMatrixPerspectiveFovLH(
        m_fovRadians, m_aspectRatio, m_nearZ, m_farZ);

	m_gcb.localConstant.projection = projection.Transpose();
}

void CameraComponent::SyncCB()
{
	if (m_gcbInitialized)
		m_gcb.Update();
}

void CameraComponent::OnRegister()
{
	Graphics::m_renderEngine->RegistCamera(this);
}

D3D12_GPU_VIRTUAL_ADDRESS CameraComponent::GetGCBGPUAddress() const
{
	return m_gcbInitialized ? m_gcb.GetGPUAddress() : 0;
}

void CameraComponent::UpdateConstantTransform()
{
	SceneComponent::UpdateConstantTransform();
	auto loc = GetLocation();
	//std::cout << loc.x << ", " << loc.y << ", " << loc.z << "\n";
	Matrix view = XMMatrixLookToLH(loc, m_frontDirection, m_upDirection);
	m_gcb.localConstant.view = view.Transpose();
}

DirectX::SimpleMath::Matrix CameraComponent::GetProjMatrix() const
{
	if(!m_gcbInitialized)
		return DirectX::SimpleMath::Matrix();
	return m_gcb.localConstant.projection;
}

DirectX::SimpleMath::Matrix CameraComponent::GetViewMatrix() const
{
	return XMMatrixLookToLH(GetLocation(), m_frontDirection, m_upDirection);
}
