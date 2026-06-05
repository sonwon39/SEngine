#include "PrimitiveComponent.h"
#include "Actor.h"
#include "Engine/World.h"
//#include "PhysXEngine.h"
#include "ActorData.h"

using DirectX::SimpleMath::Vector3;
using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Quaternion;
using DirectX::XMVECTOR;

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
}

void PrimitiveComponent::SyncCB()
{
	m_cb.Update();
}

D3D12_GPU_VIRTUAL_ADDRESS PrimitiveComponent::GetCBGPUAddress()
{
	return m_cbInitialized ? m_cb.GetGPUAddress() : 0;
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
    SetTextureName(ad.textureName);

    SetLocalConstant(ad.lc);
}

// ─────────────────────────────────────────────────────────────
// SceneComponent에서 이전된 머티리얼/렌더 세터들 + 가상 오버라이드
// ─────────────────────────────────────────────────────────────

void PrimitiveComponent::SetLocalConstant(const LocalConstant& newConstant)
{
	if (m_cbInitialized)
		m_cb.localConstant = newConstant;
	else
	{
		m_cb.Initialize(newConstant);
		m_cbInitialized = true;
	}
    // 입력 model은 GPU 업로드용 전치본이므로 한 번 더 전치해서 CPU 원본 transform을 복원.
    Matrix model = newConstant.model.Transpose();

    XMVECTOR s, q, t;
    DirectX::XMMatrixDecompose(&s, &q, &t, model);
    localTransform.location = t;
    localTransform.quat = q;
    localTransform.scale = s;

    UpdateConstantTransform();
}

void PrimitiveComponent::UpdateConstantTransform()
{
    SceneComponent::UpdateConstantTransform();  // m_worldMatrix + 자식 전파

    // HLSL CB는 column-major를 기대하므로 model은 전치해서 보관하고,
    // modelInvTranspose는 전치 전 행렬의 역행렬(= 노멀 변환용)로 계산한다.
    m_cb.localConstant.modelInvTranspose = m_worldMatrix.Invert();
	m_cb.localConstant.model = m_worldMatrix.Transpose();
}

