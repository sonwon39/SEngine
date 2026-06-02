#include "CameraComponent.h"
#include "World.h"

CameraComponent::CameraComponent(Actor* owner)
    : SceneComponent(owner)
{

}

CameraComponent::~CameraComponent()
{
}

void CameraComponent::Initialize(const float& fovDegrees,
    const float& width, const float& height, const float& nearZ, const float& farZ)
{
    m_fovRadians = DirectX::XMConvertToRadians(fovDegrees);
    m_aspectRatio = width / height;
    m_nearZ = nearZ;
    m_farZ = farZ;

    m_projMatrix = DirectX::XMMatrixPerspectiveFovLH(
        m_fovRadians, m_aspectRatio, m_nearZ, m_farZ);
}

void CameraComponent::UpdateCameraInfo(const int& width, const int& height)
{
    m_aspectRatio = float(width) / height;
    m_projMatrix = DirectX::XMMatrixPerspectiveFovLH(
        m_fovRadians, m_aspectRatio, m_nearZ, m_farZ);
}
//
//World* CameraComponent::GetWorld() const
//{
//    return m_owner ? m_owner->GetWorld() : nullptr;
//}