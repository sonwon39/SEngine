// 1. dds, fallback 파일 생성
// 2. bin, idx 파일 생성

#include <DirectXTex.h>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <iostream>
#include <string>

using namespace DirectX;
namespace fs = std::filesystem;

struct TextureInfo {
	std::string name;
	uint64_t offset = 0;
	uint64_t size = 0;
};

void PackDDSTextures(const std::wstring& inputFolder, const std::wstring& buildFolder)
{

	std::vector<TextureInfo> infos;
	std::wstring outputBin = buildFolder + L"textures.bin";
	std::wstring outputIdx = buildFolder + L"textures.idx";

	std::ofstream bin(outputBin, std::ios::binary);
	std::ofstream idx(outputIdx, std::ios::binary);

	uint64_t offset = 0;

	for (auto& entry : fs::directory_iterator(inputFolder))
	{
		if (entry.path().extension() == ".dds")
		{
			std::ifstream file(entry.path(), std::ios::binary | std::ios::ate);
			TextureInfo info;
			auto size = file.tellg();

			info.name = entry.path().stem().string();
			info.offset = offset;
			info.size = size;

			infos.push_back(info);
			file.seekg(0);
			std::vector<char> texture(size);
			file.read(texture.data(), size);

			bin.write(texture.data(), size);

			offset += size;
			std::string sizeStr;
			if (size >= 1e12)
				sizeStr = std::to_string(size / (int)1e12) + " T";
			else if (size >= 1e9)
				sizeStr = std::to_string(size / (int)1e12) + " G";
			else if (size >= 1e6)
				sizeStr = std::to_string(size / (int)1e6) + " M";
			else if (size >= 1e3)
				sizeStr = std::to_string(size / (int)1e3) + " K";
			else
				sizeStr = std::to_string(size);

			std::cout << "packed : " << info.name << " ( size : " << sizeStr << "B )\n";
		}
	}

	uint32_t count = static_cast<uint32_t>(infos.size());;

	idx.write(reinterpret_cast<const char*>(&count), sizeof(count));

	for (auto& info : infos)
	{
		uint32_t nameLen = static_cast<uint32_t>(info.name.size());
		idx.write(reinterpret_cast<const char*>(&nameLen), sizeof(nameLen));
		idx.write(info.name.data(), nameLen);
		idx.write(reinterpret_cast<const char*>(&info.offset), sizeof(info.offset));
		idx.write(reinterpret_cast<const char*>(&info.size), sizeof(info.size));
	}

	std::cout << "Done. Packed " << infos.size() << " DDS textures.\n";
}
void ConvertWICToDDS(const std::wstring& textureFolder, const std::wstring& texconvPath, const std::wstring& outputDir)
{
	for (auto& entry : fs::directory_iterator(textureFolder))
	{
		if (!entry.is_regular_file())
			continue;

		std::wstring ext = entry.path().extension().wstring();
		for (auto& c : ext) c = towlower(c);

		if (ext != L".jpg" && ext != L".jpeg" && ext != L".png")
			continue;


		std::wstring filename = entry.path().filename().wstring();
		std::wstring fullPath = entry.path().wstring();
		std::wstring outputFullPath = outputDir + entry.path().stem().wstring() + L".dds";

		if (fs::exists(outputFullPath)) {

			std::wcout << outputFullPath << " 파일이 존재합니다" << '\n';
			continue;
		}
		bool isAlbedo = (filename.find(L"albedo") != std::wstring::npos);

		std::wstring format = isAlbedo ? L"BC7_UNORM_SRGB" : L"BC3_UNORM";

		std::wstring command =
			texconvPath + L" -f " + format +
			L" -m 0 -o \"" + outputDir + L"\" \"" + fullPath + L"\"";

		std::wcout << L"Running: " << command << std::endl;

		int result = _wsystem(command.c_str());
		if (result != 0)
			std::wcout << L"Failed: " << filename << std::endl;
		else
			std::wcout << L"Converted: " << filename << std::endl;
	}

}

bool GenerateDDS(const std::wstring& ddsPath, const std::wstring& fileName, const std::wstring& fallbackPath, bool isAlbedo)
{
	ScratchImage image;
	HRESULT hr = LoadFromDDSFile(ddsPath.c_str(), DDS_FLAGS_NONE, nullptr, image);
	if (FAILED(hr)) {
		std::wcout << L"Failed to LoadFromDDSFile" << fileName << '\n';
		return false;
	}

	// fallback 생성 후 저장
	const TexMetadata& meta = image.GetMetadata();
	size_t mipCount = image.GetMetadata().mipLevels;

	const Image* lastMip = image.GetImage(mipCount - 1, 0, 0);

	ScratchImage fallback;
	hr = fallback.Initialize2D(meta.format, lastMip->width, lastMip->height, 1, 1);
	memcpy(fallback.GetImages()->pixels, lastMip->pixels, lastMip->slicePitch);

	hr = SaveToDDSFile(fallback.GetImages(), fallback.GetImageCount(), fallback.GetMetadata(), DDS_FLAGS_NONE, fallbackPath.c_str());
	if (FAILED(hr)) {
		std::wcout << L"Failed to SaveToDDSFile" << fileName << '\n';
		return false;
	}

	return true;
}

void GenerateFallback(const std::wstring& ddsFolder, const std::wstring& fallbackFolder)
{
	for (auto& f : fs::directory_iterator(ddsFolder))
	{
		if (f.path().extension() == ".dds")
		{
			std::wstring fileName = f.path().stem().wstring();

			bool isAlbedo = (fileName.find(L"albedo") != std::string::npos);
			std::wstring fallbackPath = fallbackFolder + fileName + L".dds";
			if (GenerateDDS(f.path().wstring(), fileName, fallbackPath, isAlbedo))
			{
				std::wcout << L"Genderated Fallback - " << fileName << '\n';
			}
		}
	}

}

int main()
{
	HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	if (FAILED(hr))
	{
		std::wcout << L"Failed to initialize COM: hr = " << std::hex << hr << std::endl;
		return -1;
	}
	std::wstring texconvPath = L"texconv.exe";
	std::wstring assetsFolder = L"../SEngine/Assets/";
	std::wstring texturesFolder = assetsFolder + L"Textures/";
	std::wstring buildFolder = assetsFolder + L"Build/";
	std::wstring fallbackBuildFolder = buildFolder+ L"Fallback/";
	std::wstring ddsFolder = texturesFolder + L"DDS/";
	std::wstring fallbackFolder = texturesFolder+ L"Fallback/";
	std::wstring ddsExtension = L".dds";

	fs::create_directories(buildFolder);
	fs::create_directories(fallbackBuildFolder);

	fs::create_directories(ddsFolder);
	fs::create_directories(fallbackFolder);

	std::cout << "- ConvertWICToDDS\n\n";

	ConvertWICToDDS(texturesFolder, texconvPath, ddsFolder);

	std::cout << "\n- GenerateFallback\n\n";
	GenerateFallback(ddsFolder, fallbackFolder);

	std::cout << "\n- Packing 시작\n\n";
	// dds packing
	PackDDSTextures(ddsFolder, buildFolder);
	std::cout << '\n';
	// fallback packing
	PackDDSTextures(fallbackFolder, fallbackBuildFolder);
}
