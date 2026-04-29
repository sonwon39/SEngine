#include "StaticMesh.h"
#include "Vertex.h"


StaticMesh::StaticMesh()
{

}

StaticMesh::~StaticMesh()
{
}

void StaticMesh::Render_(ID3D12GraphicsCommandList* commandList)
{
	for (size_t i = 0; i < meshCount; i++)
	{
		commandList->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		commandList->IASetVertexBuffers(0, 1, &m_vertexBufferViews[i]);
		commandList->IASetIndexBuffer(&m_indexBufferViews[i]);
		commandList->DrawIndexedInstanced(m_indexCounts[i], 1, 0, 0, 0);
	}
}

void StaticMesh::CubeMapRender(ID3D12GraphicsCommandList* commandList)
{
	for (size_t i = 0; i < meshCount; i++)
	{
		commandList->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		commandList->IASetVertexBuffers(0, 1, &m_vertexBufferViews[i]);
		commandList->IASetIndexBuffer(&m_indexBufferViews[i]);
	
		commandList->DrawIndexedInstanced(m_indexCounts[i], 1, 0, 0, 0);
	}
}

void StaticMesh::Render(ID3D12GraphicsCommandList* commandList)
{
	for (size_t i = 0; i < meshCount; i++)
	{
		commandList->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		commandList->IASetVertexBuffers(0, 1, &m_vertexBufferViews[i]);
		commandList->IASetIndexBuffer(&m_indexBufferViews[i]);
		commandList->DrawIndexedInstanced(m_indexCounts[i], 1, 0, 0, 0);
	}
}

void StaticMesh::RenderDot(ID3D12GraphicsCommandList* commandList)
{
	for (size_t i = 0; i < meshCount; i++)
	{
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
		commandList->IASetVertexBuffers(0, 1, &m_vertexBufferViews[i]);
		commandList->DrawInstanced(m_vertexCounts[i], 1, 0, 0);
	}
}

//Render Point Cloud
void StaticMesh::RenderPoints(ID3D12GraphicsCommandList* commandList)
{
	for (size_t i = 0; i < meshCount; i++)
	{
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);

		commandList->IASetVertexBuffers(0, 1, &m_vertexBufferViews[i]);

		commandList->DrawInstanced(m_vertexCounts[i], 1, 0, 0);
	}
}

