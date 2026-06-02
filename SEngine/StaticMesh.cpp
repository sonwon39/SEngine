#include "StaticMesh.h"
#include "Vertex.h"


StaticMesh::StaticMesh()
{

}

StaticMesh::~StaticMesh()
{
}

void StaticMesh::Render(ID3D12GraphicsCommandList* commandList)
{
	for (size_t i = 0; i < meshCount; i++)
	{
		commandList->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		commandList->IASetVertexBuffers(0, 1, m_vertexBuffers[i].GetView());
		commandList->IASetIndexBuffer(m_indexBuffers[i].GetView());
		UINT indexCount = (UINT)m_indexBuffers[i].dataCount;
		commandList->DrawIndexedInstanced(indexCount, 1, 0, 0, 0);
	}
}

