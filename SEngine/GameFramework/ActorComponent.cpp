#include "ActorComponent.h"

ActorComponent::ActorComponent(Actor* owner)
	: m_owner(owner) 
{
}

ActorComponent::~ActorComponent()
{
}

void ActorComponent::Tick(const float& deltaTime)
{
}
