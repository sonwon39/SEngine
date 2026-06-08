#include "AMovingPlatform.h"
#include "GraphicsCommon.h"
#include "Engine\World.h"
#include "GameFramework\StaticMeshComponent.h"
#include "GameFramework\CameraComponent.h"

using namespace Graphics;

AMovingPlatform::AMovingPlatform()
{
    m_velocity = 10.f;
}

AMovingPlatform::~AMovingPlatform()
{
}

void AMovingPlatform::Initialize()
{
    if (!m_world)
        return;
    auto mesh = m_world->GetMesh("cube");

    LocalConstant lc;
    lc.model = DirectX::XMMatrixTranslation(2.f, 1.f, 3.f);
    lc.model = lc.model.Transpose();

    std::shared_ptr<StaticMeshComponent> root = std::make_shared<StaticMeshComponent>(this);
    root->SetMesh(mesh);
    root->SetUpdateConstant(true);
    root->SetLocalConstant(lc);
    root->SetTextureName("PavingStones145_2K-PNG_Albedo");
    root->SetPSOName("defaultPSO");

    std::shared_ptr<CameraComponent> camera = std::make_shared<CameraComponent>(this);

    root->Attach(camera);

    camera->SetLocation(Vector3(0.f, 1.f, -2.f));
    camera->UpdateRotation(0.f, 30.f);
    camera->Initialize(70.f, m_world->windowWidth, m_world->windowHeight, 0.01f, 200.f);
    SetRootComponent(root);
}

void AMovingPlatform::Tick(const float& deltaTime)
{
    if (!m_world || !(m_world->m_inputHelper))
        return;

    /*Vector3 dir = m_world->m_inputHelper->GetInputDirection();
    Vector3 dx = dir * m_velocity *deltaTime;
    Actor::UpdateActorLocation(dx);*/
}
