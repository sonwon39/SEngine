#pragma once

#include "d3d12.h"
#include <string>

class StaticMesh;
class Material;
class PrimitiveComponent;

// 한 번의 mesh draw를 위한 가벼운 참조 묶음.
// 자원은 모두 외부 소유 (mesh = StaticMesh 자산, material = World 캐시, owner = 컴포넌트).
class MeshBatch
{
public:
	void Render(ID3D12GraphicsCommandList* commandList);
	void SyncCB();
	void InitGraphicsCommand(ID3D12GraphicsCommandList* commandList);

public:
	StaticMesh*   mesh = nullptr;
	const Material*     material = nullptr;
	PrimitiveComponent* owner = nullptr;  // per-primitive CB sync용
	std::string psoName;

};
