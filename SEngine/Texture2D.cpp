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
