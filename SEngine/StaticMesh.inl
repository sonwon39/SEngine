#pragma once

#include "StaticMesh.h"
#include "GraphicsCommon.h"


template<typename V, typename I>
inline void StaticMesh::Initialize(ID3D12Device5* device, ID3D12GraphicsCommandList* commandList, Mesh<V, I>& mesh)
{
	m_indexCounts.push_back((UINT)mesh.m_indices.size());
	meshCount = 1;

	m_vertexGpu.resize(meshCount);
	m_vertexUpload.resize(meshCount);
	m_indexGpu.resize(meshCount);
	m_indexUpload.resize(meshCount);

	Graphics::utility->CreateBuffer<V>(mesh.m_vertices, m_vertexGpu[0], m_vertexUpload[0]);
	Graphics::utility->CreateBuffer<I>(mesh.m_indices, m_indexGpu[0], m_indexUpload[0]);

	D3D12_VERTEX_BUFFER_VIEW VBV;
	D3D12_INDEX_BUFFER_VIEW IBV;
	VBV.BufferLocation = m_vertexGpu[0]->GetGPUVirtualAddress();
	VBV.SizeInBytes = (UINT)(mesh.m_vertices.size() * sizeof(V));
	VBV.StrideInBytes = (UINT)(sizeof(V));

	IBV.BufferLocation = m_indexGpu[0]->GetGPUVirtualAddress();
	IBV.Format = ((sizeof(I) == sizeof(uint16_t)) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT);
	IBV.SizeInBytes = (UINT)(mesh.m_indices.size() * sizeof(I));

	m_vertexBufferViews.push_back(VBV);
	m_indexBufferViews.push_back(IBV);

}

template<typename V, typename I>
inline void StaticMesh::InitializePC(ID3D12Device5* device, ID3D12GraphicsCommandList* commandList, const std::vector<Mesh<V, I>>& meshes)
{
	meshCount = (UINT)meshes.size();

	m_vertexGpu.resize(meshCount);
	m_vertexUpload.resize(meshCount);

	for (size_t i = 0; i < meshCount; i++)
	{
		const auto& mesh = meshes[i];
		m_vertexCounts.push_back((UINT)mesh.m_vertices.size());

		Graphics::utility->CreateBuffer<V>(mesh.m_vertices, m_vertexGpu[i], m_vertexUpload[i]);

		D3D12_VERTEX_BUFFER_VIEW VBV;
		VBV.BufferLocation = m_vertexGpu[i]->GetGPUVirtualAddress();
		VBV.SizeInBytes = (UINT)(mesh.m_vertices.size() * sizeof(V));
		VBV.StrideInBytes = (UINT)(sizeof(V));
		
		m_vertexBufferViews.push_back(VBV);
	}
}

template<typename V, typename I>
inline void StaticMesh::Initialize(ID3D12Device5* device, ID3D12GraphicsCommandList* commandList, const std::vector<Mesh<V, I>>& meshes)
{
	meshCount = (UINT)meshes.size();
	
	if (sizeof(V) == sizeof(PBRVertex) && sizeof(I) == sizeof(uint16_t))
	{
		m_mesh = (std::vector<Mesh<PBRVertex, uint16_t>>*)&meshes;
	}
	m_vertexGpu.resize(meshCount);
	m_vertexUpload.resize(meshCount);
	m_indexGpu.resize(meshCount);
	m_indexUpload.resize(meshCount);

	for (size_t i = 0; i < meshCount; i++)
	{
		const auto& mesh = meshes[i];
		m_indexCounts.push_back((UINT)mesh.m_indices.size());
		m_vertexCounts.push_back((UINT)mesh.m_vertices.size());

		Graphics::utility->CreateBuffer<V>(mesh.m_vertices, m_vertexGpu[i], m_vertexUpload[i]);
		Graphics::utility->CreateBuffer<I>(mesh.m_indices, m_indexGpu[i], m_indexUpload[i]);

		D3D12_VERTEX_BUFFER_VIEW VBV;
		D3D12_INDEX_BUFFER_VIEW IBV;
		VBV.BufferLocation = m_vertexGpu[i]->GetGPUVirtualAddress();
		VBV.SizeInBytes = (UINT)(mesh.m_vertices.size() * sizeof(V));
		VBV.StrideInBytes = (UINT)(sizeof(V));

		IBV.BufferLocation = m_indexGpu[i]->GetGPUVirtualAddress();
		IBV.Format = ((sizeof(I) == sizeof(uint16_t)) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT);
		IBV.SizeInBytes = (UINT)(mesh.m_indices.size() * sizeof(I));

		m_vertexBufferViews.push_back(VBV);
		m_indexBufferViews.push_back(IBV);
		
	}
}
