#include "ALight.h"
#include "GraphicsCommon.h"
#include "Engine\World.h"
#include "GameFramework\StaticMeshComponent.h"
#include "GameFramework\LightComponent.h"

using namespace Graphics;

ALight::ALight()
{
}

ALight::~ALight()
{
}
void ALight::Initialize()
{
    if (!m_world)
        return;
    auto mesh = m_world->GetMesh("sphere");
    Light l;
    l.position = Vector3(-10.f, 10.f, 0.f);
	l.intensity = 100.f;
    l.color = Vector3(1.f, 1.f, 1.f);
    l.enabled = 1;

    LocalConstant lc;
    lc.model = DirectX::XMMatrixTranslation(l.position.x, l.position.y, l.position.z);
    lc.model = lc.model.Transpose();

    std::shared_ptr<StaticMeshComponent> root = std::make_shared<StaticMeshComponent>(this);
    root->SetMesh(mesh);
    root->SetUpdateConstant(true);
    root->SetLocalConstant(lc);
    root->SetTextureName("PavingStones145_2K-PNG_Albedo");
    root->SetPSOName("defaultPSO");

    std::shared_ptr<LightComponent> light = std::make_shared<LightComponent>(this);
    light->Initialize(l);

    root->Attach(light);
     SetRootComponent(root);
}

void ALight::Tick(const float& deltaTime)
{

}
