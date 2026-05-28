#include "Texture2D.h"
#include "GraphicsCommon.h"

using namespace Graphics;

Texture2D::Texture2D()
{
}

Texture2D::~Texture2D()
{
}

void Texture2D::Initialize(int width, int height, DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags, D3D12_RESOURCE_STATES state, UINT miplevels, std::wstring name)
{
	m_width = width;
	m_height = height;

	utility->CreateTextureBuffer(buffer, width, height, format, flags, state, miplevels, name);
	m_currentState = state;
}

D3D12_RESOURCE_BARRIER Texture2D::Transition(D3D12_RESOURCE_STATES newState)
{
	
	if (m_currentState == newState && newState == D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
	{
		return CD3DX12_RESOURCE_BARRIER::UAV(buffer.Get());
	}
	D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(buffer.Get(), m_currentState, newState);
	m_currentState = newState;

	return barrier;
}
