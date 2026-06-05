#include "MeshBatch.h"

#include "StaticMesh.h"
#include "Material.h"
#include "GameFramework/PrimitiveComponent.h"

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
