#include "Actor.h"
#include "StaticMeshComponent.h"
#include "StaticMesh.h"
#include "CameraComponent.h"
#include "CollisionComponent.h"
#include "SkinnedMeshComponent.h"
#include "GraphicsCommon.h"

using namespace Graphics;
using DirectX::SimpleMath::Vector3;


Actor::Actor()
	: m_currentState(AS_default) // 버그 수정: 기존엔 상태를 초기화하지 않아 GetActorState()가 미정의 값을 반환했음
{
}

Actor::Actor(std::string actorName)
	: m_name(actorName), m_currentState(AS_default)
{
}

void Actor::Initialize(std::shared_ptr<StaticMesh> mesh, const ActorData& ad)
{
	std::shared_ptr<StaticMeshComponent> root = std::make_shared<StaticMeshComponent>(this);
	root->SetMesh(mesh);
	root->SetActorData(ad);
	SetRootComponent(root);
}

void Actor::Tick(const float& deltaTime)
{
}

void Actor::UpdateRotation(const int& mouseDeltaX, const int& mouseDeltaY, const float& deltaTime)
{
	if (m_rootComponent) m_rootComponent->UpdateRotation(mouseDeltaX, mouseDeltaY, deltaTime);
}

void Actor::UpdateCameraInfo(const int& width, const int& height)
{
	if (m_rootComponent) m_rootComponent->UpdateCameraInfo(width, height);
}

void Actor::SetActorLocation(const DirectX::SimpleMath::Vector3& newLocation)
{
	if (m_rootComponent) m_rootComponent->SetLocation(newLocation);
}

void Actor::SetActorRotation(const DirectX::SimpleMath::Quaternion& newQuat)
{
	if (m_rootComponent) m_rootComponent->SetRotation(newQuat);
}

void Actor::UpdateActorLocation(const DirectX::SimpleMath::Vector3& delLocation)
{
	if (m_rootComponent) m_rootComponent->AddLocation(delLocation);
}

void Actor::UpdateActorRotation(const DirectX::SimpleMath::Quaternion& delQuat)
{
	if (m_rootComponent) m_rootComponent->AddRotation(delQuat);
}

void Actor::SetActorSpeed(const float& newSpeed)
{
	if (m_rootComponent) m_rootComponent->SetSpeed(newSpeed);
}

void Actor::SetRootComponent(std::shared_ptr<SceneComponent> newRootComponent)
{
	m_rootComponent = std::move(newRootComponent);
	// 루트의 구체 타입을 한 번만 확인해 캐싱한다(이후 Setter들은 dynamic_cast 없이 포인터 사용).
	m_rootPrimitive = dynamic_cast<PrimitiveComponent*>(m_rootComponent.get());
	m_rootSkinned = dynamic_cast<SkinnedMeshComponent*>(m_rootComponent.get());
}

void Actor::OnRegister()
{
	if (m_rootComponent) m_rootComponent->OnRegister();
}

DirectX::SimpleMath::Vector3 Actor::GetActorLocation() const
{
	return m_rootComponent ? m_rootComponent->GetLocation() : DirectX::SimpleMath::Vector3(0.f,0.f,0.f);
}

DirectX::SimpleMath::Vector3 Actor::GetActorFrontDir() const
{
	return m_rootComponent ? m_rootComponent->GetFrontDirection() : DirectX::SimpleMath::Vector3::Zero;
}

DirectX::SimpleMath::Vector3 Actor::GetCameraLocation() const
{
	return m_rootComponent ? m_rootComponent->GetCameraLocation() : DirectX::SimpleMath::Vector3::Zero;
}

DirectX::SimpleMath::Matrix Actor::GetProjMatrix() const
{
	return m_rootComponent ? m_rootComponent->GetProjMatrix() : DirectX::SimpleMath::Matrix();
}

DirectX::SimpleMath::Vector3 Actor::GetActorUpDir() const
{
	return m_rootComponent ? m_rootComponent->GetUpDirection() : DirectX::SimpleMath::Vector3::Zero;
}

DirectX::SimpleMath::Vector3 Actor::GetActorRightDir() const
{
	return m_rootComponent ? m_rootComponent->GetRightDirection() : DirectX::SimpleMath::Vector3::Zero;
}

float Actor::GetActorSpeed() const
{
	return m_rootComponent ? m_rootComponent->GetSpeed() : 0.f;
}

DirectX::SimpleMath::Matrix Actor::GetViewMatrix() const
{
	return m_rootComponent ? m_rootComponent->GetViewMatrix() : DirectX::SimpleMath::Matrix();
}

DirectX::SimpleMath::Matrix Actor::GetCameraViewMatrix() const
{
	return m_rootComponent ? m_rootComponent->GetCameraViewMatrix() : DirectX::SimpleMath::Matrix();
}

void Actor::UpdateMipState(int newForceMip0)
{
	if (m_rootComponent) m_rootComponent->UpdateMipState(newForceMip0);
}

void Actor::UpdateUseReflect(int newUseReflect)
{
	if (m_rootComponent) m_rootComponent->UpdateUseReflect(newUseReflect);
}

void Actor::SetUpdateConstant(bool newState)
{
	if (m_rootPrimitive)
		m_rootPrimitive->SetUpdateConstant(newState);
}

void Actor::UpdateAnimation(float deltaTime)
{
	if (m_rootSkinned)
		m_rootSkinned->UpdateAnimation(deltaTime);
}

void Actor::SetLocalConstant(const LocalConstant& newLocalConstant)
{
	if (m_rootComponent) m_rootComponent->SetLocalConstant(newLocalConstant);
}

void Actor::SetTextureName(const std::string& newName)
{
	if (m_rootPrimitive)
		m_rootPrimitive->SetTextureName(newName);
}
void Actor::SetPSOName(const std::string& newName)
{
	if (m_rootPrimitive)
		m_rootPrimitive->SetPSOName(newName);
}

void Actor::SetHeightScale(const float& heightScale)
{
	// SetHeightScale은 SceneComponent에 정의돼 있어 캐스팅 없이 루트로 바로 위임한다.
	if (m_rootComponent)
		m_rootComponent->SetHeightScale(heightScale);
}
