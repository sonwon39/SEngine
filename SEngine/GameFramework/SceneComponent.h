#pragma once

#include <vector>
#include <memory>

#include "ActorComponent.h"
#include "directxtk12/SimpleMath.h"
#include "Transform.h"
#include "PBR/PBRHLSLCompat.h"
#include "PhysXMode.h"

// transform
class SceneComponent : public ActorComponent {
public:
	SceneComponent(Actor* owner);
	virtual ~SceneComponent();

	void Attach(std::shared_ptr<SceneComponent> sceneComp);

public:
	void SetSpeed(const float& newSpeed);
	void SetRotateSpeed(const float& newSpeed);
	void UpdateWorldTransform(const Transform& tr);
	void SetLocalConstant(const LocalConstant& newConstant);
	void UpdateRotation(const int& mouseDeltaX, const int& mouseDeltaY, const float& deltaTime);
	void SetLocation(const DirectX::SimpleMath::Vector3& newLocation);
	void SetRotation(const DirectX::SimpleMath::Quaternion& newQuat);
	void SetLocalTransform(const Matrix& newMatrix);
	void SetLocalTransform(const Transform& newTransform);
	// physx에서 계산한 localTransform * worldTransform으로 부터 localTransform 추출
	void SetLocalTransformByLdotW(const Transform& LoW);
	void SetFrontDirection(const DirectX::SimpleMath::Vector3& newDir) { m_frontDirection = newDir; }
	void SetUpDirection(const DirectX::SimpleMath::Vector3& newDir) { m_upDirection = newDir; }
	void SetRightDirection(const DirectX::SimpleMath::Vector3& newDir) { m_rightDirection = newDir; }
	void SetCubeMapMipLevel(const int& newCubeMapMipLevel);
	void SetHeightScale(const float& newHeightScale);
	void SetRoughness(const float& newRoughness);
	void SetMetallic(const float& newMetalic);
	void SetCollisionScale(const Vector3& newScale);
	void SetCollisionShape(const PhysXShape& newShape);
	void AddLocation(const DirectX::SimpleMath::Vector3& delLocation);
	void AddRotation(const DirectX::SimpleMath::Quaternion& delQ);

public:
	float GetSpeed() const { return m_speed; }
	float GetRotateSpeed() const { return m_rotateSpeed; }
	virtual DirectX::SimpleMath::Vector3 GetCollisionScale() const;
	DirectX::SimpleMath::Quaternion GetCollisionRotation() const;
	LocalConstant GetLocalConstant() { return localConstant; }

	DirectX::SimpleMath::Vector3 GetFrontDirection() const { return m_frontDirection; }
	DirectX::SimpleMath::Vector3 GetBaseFrontDirection() const { return m_baseFrontDirection; }
	DirectX::SimpleMath::Vector3 GetBaseUpDirection() const { return m_baseUpDirection; }
	DirectX::SimpleMath::Vector3 GetUpDirection() const { return m_upDirection; }
	DirectX::SimpleMath::Vector3 GetRightDirection()const { return m_rightDirection; }

	DirectX::SimpleMath::Vector3 GetLocalLocation() const { return localTransform.location; }
	DirectX::SimpleMath::Quaternion GetLocalRotation() const { return localTransform.quat; }

	DirectX::SimpleMath::Vector3 GetLocation() const { return localConstant.model.Transpose().Translation(); }
	DirectX::SimpleMath::Vector3 GetCollisionLocation() const;
	DirectX::SimpleMath::Vector3 GetCollisionOffsetLocation() const;
	DirectX::SimpleMath::Quaternion GetCollisionOffsetRotation() const;
	DirectX::SimpleMath::Quaternion GetRotation() const;
	DirectX::SimpleMath::Matrix GetViewMatrix() const;
	DirectX::SimpleMath::Matrix GetCameraViewMatrix() const;
	void GetChildrenComponents(std::vector<std::shared_ptr<SceneComponent>>& children) const;

	DirectX::SimpleMath::Vector3 GetCameraLocation() const;
	virtual	DirectX::SimpleMath::Matrix GetProjMatrix() const;

	virtual void UpdateConstantTransform();

public:
	// world->actor 호출
	virtual void OnRegister();

	virtual void UpdateConstantLocation();
	void UpdateConstantRotation();
	void UpdateMipState(int newForceMip0);
	void UpdateUseReflect(int newUseReflect);
	void UpdateTexTransform(const DirectX::SimpleMath::Matrix& texTransform);
	virtual void UpdateCameraInfo(const int& width, const int& height);

protected:
	//DirectX::SimpleMath::Vector3 m_location;
	//DirectX::SimpleMath::Matrix m_rotation;
	Transform worldTransform;
	Transform localTransform;

	DirectX::SimpleMath::Vector3 m_baseUpDirection;
	DirectX::SimpleMath::Vector3 m_baseFrontDirection;
	DirectX::SimpleMath::Vector3 m_frontDirection;
	DirectX::SimpleMath::Vector3 m_upDirection;
	DirectX::SimpleMath::Vector3 m_rightDirection;

protected:
	float m_speed = 5.f;
	float m_rotateSpeed = 60.f;
	LocalConstant localConstant;
	float xAngle = 0.f;
	float yAngle = 0.f;

	float maxYAngle = 89.f;
	float minYAngle = -89.f;


protected:
	SceneComponent* m_parent = nullptr;
	std::vector<std::shared_ptr<SceneComponent>> m_children;
};
