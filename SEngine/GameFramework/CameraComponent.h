#pragma once

#include "SceneComponent.h"
#include "d3d12.h"
#include "directxtk12\SimpleMath.h"

class CameraComponent : public SceneComponent {
public:
	CameraComponent(Actor* owner);
	virtual ~CameraComponent();
	void Initialize(const float& fovDegrees, const float& width, const float& height, const float& nearZ, const float& farZ);

public:
	DirectX::SimpleMath::Matrix	GetProjMatrix() const override { return  m_projMatrix; }

	public:
	void UpdateCameraInfo(const int& width, const int& height) override;

private:
	float m_fovRadians;
	float m_nearZ;
	float m_farZ;
	float m_aspectRatio;
	
	DirectX::SimpleMath::Matrix m_projMatrix;

};
