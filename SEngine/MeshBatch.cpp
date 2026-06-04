#include "MeshBatch.h"

#include "StaticMesh.h"
#include "Material.h"
#include "GameFramework/PrimitiveComponent.h"

void MeshBatch::Render(ID3D12GraphicsCommandList* commandList)
{
	if (!mesh || !material || !owner) return;

	owner->SyncCB();              // CPU localConstant → GPU CB
	material->Bind(commandList);  // heap + SRV table 바인딩
	// NOTE: per-primitive CBV 바인딩은 PSO 루트 시그니처가 b1 슬롯을 가질 때만 가능하다.
	// defaultPSO는 현재 g_S1_C1_RS (b0만) 사용 → 바인딩 생략. RS를 b1까지 확장 후
	// commandList->SetGraphicsRootConstantBufferView(<slot>, owner->GetCBGPUAddress()) 추가.

	mesh->Render(commandList);    // VB/IB + DrawIndexed (StaticMesh가 이미 갖고 있음)
}
