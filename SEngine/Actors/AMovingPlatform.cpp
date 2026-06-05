#include "AMovingPlatform.h"
#include "GraphicsCommon.h"
#include "Engine\World.h"
#include "GameFramework\StaticMeshComponent.h"

using namespace Graphics;

AMovingPlatform::AMovingPlatform()
{
	m_velocity = Vector3(-1.f, 0.f, 0.f);
}

AMovingPlatform::~AMovingPlatform()
{
}

void AMovingPlatform::Initialize()
{
	if (!m_world) return;
	auto mesh = m_world->GetMesh("cube");

	LocalConstant lc;
	lc.model = DirectX::XMMatrixTranslation(2.f, 0.f, 3.f);
	lc.model = lc.model.Transpose();


	std::shared_ptr<StaticMeshComponent> root = std::make_shared<StaticMeshComponent>(this);
	root->SetMesh(mesh);
	root->SetUpdateConstant(true);
	root->SetLocalConstant(lc);
	root->SetTextureName("PavingStones145_2K-PNG_Albedo");

	SetRootComponent(root);
}

void AMovingPlatform::Tick(const float& deltaTime)
{
	Vector3 dx = m_velocity * deltaTime;
	Actor::UpdateActorLocation(dx);
}
