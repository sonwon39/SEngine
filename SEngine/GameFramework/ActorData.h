#pragma once

#include <string>
#include <directxtk12/SimpleMath.h>
#include "PBR/PBRHLSLCompat.h"
#include "PhysXMode.h"

enum ActorState {
	AS_default,
	AS_idle,
	AS_attack,
	AS_playMontage
};

struct ActorData
{
	ActorData()
	{
		useSimulate = false;
		mode = PhysXMode::PM_Default;
		updateConstants = false;
		lc.useReflect = false;
		lc.heightScale = 0.f;
		lc.forceMip0 = false;
		lc.roughness = 0.2f;
		lc.metallic = 0.8f;
		lc.collisionScale = Vector3(0.5, 0.5, 0.5);
		lc.collisionShape = PhysXShape::PS_cube;
		collisionLocation = Vector3::Zero;
	};

	std::string name;
	std::string mesh;
	std::string material;
	std::string psoName;
	//DirectX::SimpleMath::Vector3 pos;
	bool useSimulate;
	PhysXMode mode = PhysXMode::PM_Default;
	bool updateConstants;
	Vector3 collisionLocation;
	LocalConstant lc;
};

struct LightData
{
	float viewWidth;
	float viewHeight;
	float nearZ;
	float farZ;

	float intensity;
	Vector4 brightness;
	Vector4 color;
	Vector3 dir;
};


struct AnimData
{
	std::string name;
	float animationSpeed = 60.f;
	bool playAnimation = true;
	ActorState actorState = ActorState::AS_default;
};
