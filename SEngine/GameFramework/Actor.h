#pragma once

#include <memory>
#include "d3d12.h"
#include "directxtk12\SimpleMath.h"

//#include "TextureLoader.h"
#include "SceneComponent.h"
#include "ActorData.h"

class World;
class StaticMesh;

class Actor
{
public:
	Actor();
	Actor(std::string actorName, World* world);

public:
	virtual void Initialize(std::shared_ptr<StaticMesh> mesh, const ActorData& ad);

public:
	virtual void Tick(const float& deltaTime);
	virtual void Interact();

public:
	void UpdateRotation(const int& mouseDeltaX, const int& mouseDeltaY, const float& deltaTime);
	void UpdateCameraInfo(const int& width, const int& height);
	void SetActorLocation(const DirectX::SimpleMath::Vector3& newLocation);
	//void SetActorRotation(const DirectX::SimpleMath::Matrix& newMat);
	void SetActorRotation(const DirectX::SimpleMath::Quaternion& newQuat);
	void UpdateActorLocation(const DirectX::SimpleMath::Vector3& delLocation);
	void UpdateActorRotation(const DirectX::SimpleMath::Quaternion& delQuat);
	void SetActorSpeed(const float& newSpeed);
	void SetRootComponent(std::shared_ptr<SceneComponent> newRootComponent);


public:
	void OnRegister();

public:
	SceneComponent* GetRootComponent() const { return m_rootComponent.get(); }
	World* GetWorld() const { return m_world; }
	std::string GetName() const { return m_name; }

public:
	DirectX::SimpleMath::Vector3 GetActorLocation() const;
	DirectX::SimpleMath::Vector3 GetActorFrontDir() const;
	DirectX::SimpleMath::Vector3 GetCameraLocation() const;
	DirectX::SimpleMath::Matrix GetProjMatrix() const;
	DirectX::SimpleMath::Vector3 GetActorUpDir() const;
	DirectX::SimpleMath::Vector3 GetActorRightDir() const;
	float GetActorSpeed() const;
	DirectX::SimpleMath::Matrix GetViewMatrix() const;
	DirectX::SimpleMath::Matrix GetCameraViewMatrix() const;
	ActorState GetActorState() const { return m_currentState; }

public:
	void UpdateMipState(int newForceMip0);

	void UpdateUseReflect(int newUseReflect);
	void SetUpdateConstant(bool newState);
	void UpdateAnimation(float deltaTime);
	void SetActorData(const ActorData& ad);
	void SetLocalConstant(const LocalConstant& newLocalConstant);
	void SetTextureName(const std::string& newName);
	void SetHeightScale(const float& heightScale);
	void SetPSOName(const std::string& newName);
	void SetActorState(const ActorState newState) { m_currentState = newState; }

protected:
	std::shared_ptr<SceneComponent> m_rootComponent;
	std::string m_name;

private:
	class World* m_world;
	ActorState m_currentState;
};
