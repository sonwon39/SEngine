#include "PrimitiveComponent.h"
#include "Actor.h"
#include "Engine/World.h"
//#include "PhysXEngine.h"
#include "ActorData.h"

PrimitiveComponent::PrimitiveComponent(Actor* owner)
    : SceneComponent(owner), m_visible(true)
{

}

PrimitiveComponent::~PrimitiveComponent()
{
}


std::string PrimitiveComponent::GetName() const
{
	return m_owner ? m_owner->GetName() : "";
}

physx::PxTransform PrimitiveComponent::GetPxTransform() const
{
    // TODO : quat 변경
    DirectX::SimpleMath::Vector3 loc = GetCollisionLocation();
    DirectX::SimpleMath::Quaternion q = GetCollisionRotation();
    return physx::PxTransform(
        physx::PxVec3(loc.x, loc.y, loc.z),
        physx::PxQuat(q.x,q.y,q.z,q.w)
    );
}

void PrimitiveComponent::OnRegister()
{
    // 비활성화: 현재 World에는 RegisterPrimitive가 없음. PhysX 재도입 시 아래 등록 로직을 복구할 것.
    /*  SceneComponent::OnRegister();

    World* world = GetWorld();
    if (world)
    {
        world->RegisterPrimitive(this, m_usePhysX);
    }*/
}

void PrimitiveComponent::SyncFromPhysX(const physx::PxTransform& transform)
{
    DirectX::SimpleMath::Vector3 loc(transform.p.x, transform.p.y, transform.p.z);
    DirectX::SimpleMath::Quaternion rot(transform.q.x, transform.q.y, transform.q.z, transform.q.w);
    
    Vector3 collLoc = GetCollisionOffsetLocation();
    Quaternion collRot = GetCollisionOffsetRotation();

    Matrix collMat =
        Matrix::CreateFromQuaternion(collRot) *
        Matrix::CreateTranslation(collLoc);

    Matrix ret =
        Matrix::CreateFromQuaternion(rot) *
        Matrix::CreateTranslation(loc);
    collMat = collMat.Invert();
    Matrix finalMat = collMat * ret;
    Transform t;
    finalMat.Decompose(t.scale, t.quat, t.location);

    SetLocalTransformByLdotW(t);
}


void PrimitiveComponent::SetActorData(const ActorData& ad)
{
    SetPhysX(ad.useSimulate);
    SetPhysXMode(ad.mode);
    SetPSOName(ad.psoName);
    SetUpdateConstant(ad.updateConstants);
    SetTextureName(ad.material);
    
    UpdateUseReflect(ad.lc.useReflect);
    SetHeightScale(ad.lc.heightScale);
    SetRoughness(ad.lc.roughness);
    SetMetallic(ad.lc.metallic);
    SetCollisionScale(ad.lc.collisionScale);
    SetCollisionShape((PhysXShape)ad.lc.collisionShape);
}
