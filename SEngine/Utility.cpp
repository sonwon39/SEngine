#include "Utility.h"
#include <comdef.h>
#include <fstream>
#include <iostream>

DxException::DxException(HRESULT hr, const std::wstring& functionName, const std::wstring& filename, int lineNumber) :
	ErrorCode(hr),
	FunctionName(functionName),
	Filename(filename),
	LineNumber(lineNumber)
{
}

std::wstring DxException::ToString()const
{
	// Get the string description of the error code.
	_com_error err(ErrorCode);
	std::wstring msg = err.ErrorMessage();

	return FunctionName + L" failed in " + Filename + L"; line " + std::to_wstring(LineNumber) + L"; error: " + msg;
}

namespace GraphicsUtils {

	using Microsoft::WRL::ComPtr;

	Utility::Utility()
		:m_device(nullptr),
		m_commandList(nullptr)
	{
	}

	Utility::Utility(ID3D12Device5* pDevice, ID3D12GraphicsCommandList* pCommandList)
		:m_device(pDevice),
		m_commandList(pCommandList)
	{
	}

	void Utility::LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format)
	{

	}

	void Utility::CreateDescriptorHeap(UINT numDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE type, ComPtr<ID3D12DescriptorHeap>& heap, UINT nodeMask, D3D12_DESCRIPTOR_HEAP_FLAGS flag)
	{
		if (m_device == nullptr) return;

		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		heapDesc.Flags = flag;
		heapDesc.NodeMask = nodeMask;
		heapDesc.NumDescriptors = numDescriptors;
		heapDesc.Type = type;

		ThrowIfFailed(m_device->CreateDescriptorHeap(
			&heapDesc,
			IID_PPV_ARGS(heap.ReleaseAndGetAddressOf())
		));
	}

	void Utility::CreateConstantBuffer(UINT bufferSize, Microsoft::WRL::ComPtr<ID3D12Resource>& buffer, void** pConstant)
	{
		ThrowIfFailed(m_device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(bufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(buffer.ReleaseAndGetAddressOf())
		));

		CD3DX12_RANGE range(0, 0);
		ThrowIfFailed(buffer->Map(0, &range, pConstant));
	}

	D3D12_SHADER_RESOURCE_VIEW_DESC Utility::CreateSRVDesc(ID3D12Resource* resource)
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = resource->GetDesc().Format;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Texture2D.MipLevels = resource->GetDesc().MipLevels;
		srvDesc.Texture2D.ResourceMinLODClamp = 0.f;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;

		return srvDesc;
	}

	void Utility::CreateTextureBuffer(Microsoft::WRL::ComPtr<ID3D12Resource>& buffer, UINT width, UINT height,
		DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags, D3D12_RESOURCE_STATES state, UINT mipLevels, std::wstring name)
	{
		D3D12_RESOURCE_DESC rDesc = {};
		rDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		rDesc.DepthOrArraySize = 1;
		rDesc.Width = width;
		rDesc.Height = height;
		rDesc.Format = format;
		rDesc.SampleDesc = { 1,0 };
		rDesc.Flags = flags;
		rDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		rDesc.MipLevels = mipLevels;

		D3D12_CLEAR_VALUE clearValue;
		clearValue.Format = format;
		if (format == DXGI_FORMAT_R32_TYPELESS)
			clearValue.Format = DXGI_FORMAT_D32_FLOAT;

		if (clearValue.Format == DXGI_FORMAT_D24_UNORM_S8_UINT ||
			clearValue.Format == DXGI_FORMAT_D32_FLOAT)
		{
			clearValue.DepthStencil.Depth = 1.f;
			clearValue.DepthStencil.Stencil = 0;
		}
		else
		{
			clearValue.Color[0] = 0.f;
			clearValue.Color[1] = 0.f;
			clearValue.Color[2] = 0.f;
			clearValue.Color[3] = 1.f;
		}

		ThrowIfFailed( m_device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&rDesc,
			state,
			&clearValue,
			IID_PPV_ARGS(buffer.ReleaseAndGetAddressOf())
		));
		buffer->SetName(name.c_str());

	}
	inline void Utility::CreateResourceView(Microsoft::WRL::ComPtr<ID3D12Resource>& buffer, DXGI_FORMAT format, bool bUseMsaa, D3D12_CPU_DESCRIPTOR_HANDLE& handle, const DescriptorType& type)
	{
		if (type == DescriptorType::RTV) {
			D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
			ZeroMemory(&rtvDesc, sizeof(rtvDesc));
			if (bUseMsaa) {
				rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
			}
			else {
				rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
			}
			rtvDesc.Format = format;
			rtvDesc.Texture2D.MipSlice = 0;

			m_device->CreateRenderTargetView(buffer.Get(), &rtvDesc, handle);
		}
		else if (type == DescriptorType::UAV) {
			if (bUseMsaa) {
				std::cout << "UAV는 MSAA로 만들 수 없습니다" << std::endl;
				return;
			}
			D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc;
			ZeroMemory(&uavDesc, sizeof(uavDesc));
			uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
			uavDesc.Format = format;
			uavDesc.Texture2D.MipSlice = 0;
			m_device->CreateUnorderedAccessView(buffer.Get(), nullptr, &uavDesc, handle);
		}
		else if (type == DescriptorType::SRV) {

			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			if (bUseMsaa) {
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
			}
			else {
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			}
			srvDesc.Format = format;
			srvDesc.Texture2D.MipLevels = 1;
			m_device->CreateShaderResourceView(buffer.Get(), &srvDesc, handle);
		}
		else if (type == DescriptorType::DSV)
		{
			D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
			dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
			dsvDesc.Texture2D.MipSlice = 0;
			dsvDesc.Format = format;

			m_device->CreateDepthStencilView(buffer.Get(), &dsvDesc, handle);
		}
	}


	ByteArray ReadFileHelper(const wstring& fileName)
	{
		ByteArray NullFile = make_shared<vector<byte> >(vector<byte>());

		struct _stat64 fileStat;
		int fileExists = _wstat64(fileName.c_str(), &fileStat);
		if (fileExists == -1)
			return NullFile;

		ifstream file(fileName, ios::in | ios::binary);
		if (!file)
			return NullFile;

		ByteArray byteArray = make_shared<vector<byte> >(fileStat.st_size);
		file.read((char*)byteArray->data(), byteArray->size());
		file.close();

		return byteArray;
	}
}
