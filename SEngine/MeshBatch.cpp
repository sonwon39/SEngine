#include "MeshBatch.h"
#include <iostream>

#include "StaticMesh.h"
#include "Material.h"
#include "GameFramework/PrimitiveComponent.h"
#include "Renderer.h"
#include "PipelineState.h"
#include "RootSignature.h"
#include "Engine\RenderEngine.h"

using namespace Renderer;

void MeshBatch::Render(ID3D12GraphicsCommandList* commandList)
{
	if (!mesh || !material || !owner) return;

	commandList->SetGraphicsRootConstantBufferView(2, owner->GetCBGPUAddress());
	material->Bind(commandList, 0);  // heap + SRV table 바인딩
	mesh->Render(commandList);    // VB/IB + DrawIndexed (StaticMesh가 이미 갖고 있음)
}

void MeshBatch::SyncCB()
{
	if (!owner) return;
	if (owner->GetUpdateConstant())
	{
		owner->SyncCB();
	}
}

void MeshBatch::InitGraphicsCommand(ID3D12GraphicsCommandList* commandList)
{ 
	if (m_renderEngine->GetCurrPSOName() == psoName)
		return;

	GraphicsPSO pso;
	if (!GetGraphicsPSO(psoName, pso))
	{
		std::cout << "Failed to find pso\n";
		return;
	}
	m_renderEngine->SetCurrPSOName(psoName);
	commandList->SetPipelineState(pso.GetPSO());
	commandList->SetGraphicsRootSignature(pso.GetRootSignature()->GetSignature());
}
