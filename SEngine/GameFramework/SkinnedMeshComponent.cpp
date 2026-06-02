#include "SkinnedMeshComponent.h"
#include "Actor.h"
#include "GraphicsCommon.h"
#include "StaticMesh.h"

SkinnedMeshComponent::SkinnedMeshComponent(Actor* owner)
	:PrimitiveComponent(owner)
{
	m_mesh = std::make_shared<StaticMesh>();
}

SkinnedMeshComponent::~SkinnedMeshComponent()
{
}

void SkinnedMeshComponent::SetMesh(std::shared_ptr<StaticMesh> newMesh)
{
	m_mesh = std::move(newMesh);
}

void SkinnedMeshComponent::UpdateAnimation(const float& deltaTime)
{
	// 비활성화: 옛 World::skinnedMeshLoader에 의존하던 코드. 애니메이션 시스템 재도입 시 새 로더로 연결할 것.
	/*using namespace Graphics;
	if (bUpdateAnim && world && m_owner)
	{
		ActorState as = m_owner->GetActorState();
		if (as == ActorState::AS_attack)
		{
			m_currentFrame = 0.f;
			m_animationSize = world->skinnedMeshLoader->GetAnimationSize(m_animationNames[as], 0);
			if (m_animationSize != 0)
			{
				m_animationName = m_animationNames[as];
				m_owner->SetActorState(ActorState::AS_playMontage);
			}
		}
		as = m_owner->GetActorState();
		if (as == ActorState::AS_playMontage)
		{
			PlayMontage(deltaTime);
		}
		if (bBlendPose)
		{
			m_currentFrame += m_animationSpeed * deltaTime;
			m_skinnedLocalConstant = world->skinnedMeshLoader->BlendPose(m_currentBlendData);

		}
		else
		{
			m_currentFrame += m_animationSpeed * deltaTime;
			m_skinnedLocalConstant = world->skinnedMeshLoader->GetCurrentSLC(m_currentFrame, m_animationName, 0, bUpdateRoot);

		}
	}

	for (const auto& c : m_children)
	{
		if (SkinnedMeshComponent* comp = dynamic_cast<SkinnedMeshComponent*>(c.get()))
		{
			comp->UpdateAnimation(deltaTime);
		}
	}*/
}

void SkinnedMeshComponent::PlayMontage(const float& deltaTime)
{
	//m_currentFrame += m_animationSpeed * deltaTime;
	if (m_animationSize - m_currentFrame < 4.f)
	{
		m_currentBlendData.frame0 = m_currentFrame;
		m_currentBlendData.frame1 = 0;
		m_currentBlendData.animName0 = m_animationName;
		m_currentBlendData.animName1 = m_animationNames[ActorState::AS_idle];
		m_currentBlendData.weight = 1.f - ((m_animationSize - m_currentFrame) / 4.f);
		m_currentBlendData.clipId0 = 0;
		m_currentBlendData.clipId1 = 0;
		bBlendPose = true;
	}
	if (m_currentFrame > m_animationSize)
	{
		m_animationName = m_animationNames[ActorState::AS_idle];
		m_owner->SetActorState(ActorState::AS_idle);
		m_currentFrame = 0.f;
		bBlendPose = false;
	}
}

void SkinnedMeshComponent::SetAnimationData(const AnimData& animData)
{
	SetAnimationSpeed(animData.animationSpeed);
	std::string idle = animData.name + "_Idle";
	std::string attack = animData.name + "_AttackD";

	SetAnimationName(ActorState::AS_default, animData.name);
	SetAnimationName(ActorState::AS_idle, idle);
	SetAnimationName(ActorState::AS_attack, attack);
	m_animationName = m_animationNames[animData.actorState];

	SetPlayAnimation(animData.playAnimation);
}
