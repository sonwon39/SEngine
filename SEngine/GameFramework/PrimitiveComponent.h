#pragma once
#include "SceneComponent.h"
#include "d3d12.h"
#include <string>

//#include "TextureLoader.h"
//#include "Delegate.h"
#include "physx\PxPhysicsAPI.h"
#include "PhysXMode.h"

class PrimitiveComponent;
struct ActorData;

//DECLARE_DELEGATE_OneParam(ComponentBeginOverlapSignature, PrimitiveComponent*)
//DECLARE_DELEGATE_OneParam(ComponentEndOverlapSignature, PrimitiveComponent*)


class PrimitiveComponent : public SceneComponent {
public:
	PrimitiveComponent(Actor* owner);
	virtual ~PrimitiveComponent();

public:
	void SetVisible(bool visible) { m_visible = visible; }
	void SetPhysX(bool usePhysX) { m_usePhysX = usePhysX; }
	void SetPhysXMode(PhysXMode newMode) { m_physXMode = newMode; }
	void SetTextureName(const std::string& newName) { m_textureName = newName; }
	void SetPSOName(const std::string& newName) { m_psoName = newName; }
	
	virtual void SetActorData(const ActorData& ad);
	bool IsVisible() const { return m_visible; }
	bool IsKinematic() const { return m_physXMode == PhysXMode::PM_Kinematic; }

public:
	//ComponentBeginOverlapSignature OnComponentBeginOverlap;
	//ComponentEndOverlapSignature OnComponentEndOverlap;

public:
	class World* GetWorld() const;
	std::string GetName() const;
	std::string GetTextureName() const { return m_textureName; };
	std::string GetPSOName() const { return m_psoName; };
	PhysXMode GetPhysXMode() const { return m_physXMode; }
	physx::PxTransform GetPxTransform() const;

public:
	void OnRegister() override;

	// 물리 시뮬레이션 이후 transform 동기화
	void SyncFromPhysX(const physx::PxTransform& transform);
	void SetUpdateConstant(bool newState) { m_updateConstant = newState; }
	bool IsUpdateConstant() { return m_updateConstant; }

protected:
	bool m_visible;
	bool m_usePhysX = false;
	PhysXMode m_physXMode;
	bool m_updateConstant = false;
	std::string m_textureName;
	std::string m_psoName;
};
