#pragma once

#include <string>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>
#include <map>
#include <unordered_map>
#include "wrl.h"
#include "d3d12.h"
#include "DescriptorHeap.h"

struct TextureInfo {
	uint64_t offset;
	uint64_t size;
};

class TextureLoader {
public:
	TextureLoader();
	TextureLoader(std::string path, ID3D12Device5* device);

	void InitHeap(UINT heapSize);
	void LoadIdx();

	void LoadTextures(ID3D12GraphicsCommandList* commandList);

	// ExecuteCommandLists -> Fence wait 가 끝난 뒤 호출자가 한 번 호출해 업로드 임시 리소스를 해제한다.
	void FlushUploadKeepAlive() { m_ddsBlobs.clear(); m_uploadHeaps.clear(); }

public:
	DescriptorHeap* GetHeap() { return &heap; }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(const int& idx) const;
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(const std::string& filename) const;
	ID3D12Resource* GetTexture(const std::string& filename) const;
	std::vector<std::string> filenames;

private:
	std::string folder;
	std::string binPath;
	std::string idxPath;

	uint32_t count;

	std::unordered_map<std::string, TextureInfo> textureMap;
	std::unordered_map<uint32_t, std::string> nameMap;
	std::map<std::string, uint32_t> idxMap;
	
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> textures;
	DescriptorHeap heap;

	// 텍스처 업로드용 keep-alive. ExecuteCommandLists 완료 시점까지 alive 유지해야 한다
	// (LoadDDSTextureFromMemoryEx 의 subresources[].pData 가 m_ddsBlobs 내부를 가리키고,
	//  GPU 복사 명령은 m_uploadHeaps 를 읽기 때문). FlushUploadKeepAlive() 로 해제.
	std::vector<std::vector<uint8_t>> m_ddsBlobs;
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> m_uploadHeaps;
	
	UINT m_heapSize = 0;
	UINT srvOffset = 0;


};
