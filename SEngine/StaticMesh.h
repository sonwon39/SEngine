#pragma once

#include "d3d12.h"
#include "directx/d3dx12.h"
#include "wrl.h"
#include <vector>

#include "GraphicsCommon.h"
#include "Mesh.h"
//#include "TextureGPUResource.h"
#include "Vertex.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"

class TextureLoader;

class StaticMesh {

public:

	StaticMesh();
	virtual ~StaticMesh();

	template<typename V, typename I>
	void Initialize(ID3D12Device5* device, ID3D12GraphicsCommandList* commandList, Mesh<V,I>& mesh);

	template<typename V, typename I>
	void Initialize(ID3D12Device5* device, ID3D12GraphicsCommandList* commandList, const std::vector<Mesh<V, I>>& meshes);

	void Render(ID3D12GraphicsCommandList* commandList);


protected:
	std::vector<VertexBuffer> m_vertexBuffers;
	std::vector<IndexBuffer> m_indexBuffers;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_texturesHeap;
	UINT meshCount = 0;
};
#include "StaticMesh.inl"
