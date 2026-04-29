#pragma once

#include "d3d12.h"
#include "directx/d3dx12.h"
#include "wrl.h"
#include <vector>

#include "GraphicsCommon.h"
#include "Mesh.h"
//#include "TextureGPUResource.h"
#include "Vertex.h"

class TextureLoader;

class StaticMesh {

public:

	StaticMesh();
	virtual ~StaticMesh();

	template<typename V, typename I>
	void Initialize(ID3D12Device5* device, ID3D12GraphicsCommandList* commandList, Mesh<V,I>& mesh);

	// PointCloud 초기화
	template<typename V, typename I>
	void InitializePC(ID3D12Device5* device, ID3D12GraphicsCommandList* commandList, const std::vector<Mesh<V, I>>& meshes);

	template<typename V, typename I>
	void Initialize(ID3D12Device5* device, ID3D12GraphicsCommandList* commandList, const std::vector<Mesh<V, I>>& meshes);

	void Render_(ID3D12GraphicsCommandList* commandList);
	void CubeMapRender(ID3D12GraphicsCommandList* commandList);
	void Render(ID3D12GraphicsCommandList* commandList);

	void RenderDot(ID3D12GraphicsCommandList* commandList);

	//Render Point Cloud
	void RenderPoints(ID3D12GraphicsCommandList* commandList);
	void UpdateMipState(int newForceMip0);

public:
	D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView(int index = 0) const { return m_vertexBufferViews[index]; }
	D3D12_INDEX_BUFFER_VIEW GetIndexBufferView(int index = 0)const { return m_indexBufferViews[index]; }
	UINT GetIndexCount(int index = 0)const { return m_indexCounts[index]; }

protected:
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> m_vertexGpu;
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> m_vertexUpload;
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> m_indexGpu;
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> m_indexUpload;

	std::vector<D3D12_VERTEX_BUFFER_VIEW> m_vertexBufferViews;
	std::vector<D3D12_INDEX_BUFFER_VIEW> m_indexBufferViews;

	//std::vector<TextureGPUResource> GPUResources;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_texturesHeap;

	std::vector<UINT> m_indexCounts;
	std::vector<UINT> m_vertexCounts;
	UINT meshCount = 0;

	std::vector<Mesh<PBRVertex, uint16_t>>* m_mesh;
};
#include "StaticMesh.inl"