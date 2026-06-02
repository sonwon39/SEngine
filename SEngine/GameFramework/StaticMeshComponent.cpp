#include "StaticMeshComponent.h"
#include "Actor.h"
#include "StaticMesh.h"

StaticMeshComponent::StaticMeshComponent(Actor* owner)
	:PrimitiveComponent(owner)
{
	m_mesh = std::shared_ptr<StaticMesh>();
}

StaticMeshComponent::~StaticMeshComponent()
{
}

void StaticMeshComponent::SetMesh(std::shared_ptr<StaticMesh> newMesh)
{
	m_mesh = std::move(newMesh);
}

