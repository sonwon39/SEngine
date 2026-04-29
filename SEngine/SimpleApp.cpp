#include "SimpleApp.h"
#include "RootSignature.h"
#include "PipelineState.h"
#include "GeometryGenerator.h"

#include "Directxtk12/DDSTextureLoader.h"
#include "directxtk12/ResourceUploadBatch.h"


using Microsoft::WRL::ComPtr;
using namespace GraphicsUtils;
using namespace Graphics;
using namespace Renderer;

Core::SimpleApp::SimpleApp()
	:BaseApp()
{
	m_guiWidth = 100;
}

Core::SimpleApp::SimpleApp(const int width, const int height, const int guiWidth)
	:BaseApp(width, height)
{
	m_guiWidth = guiWidth;
}

Core::SimpleApp::~SimpleApp()
{
}

int Core::SimpleApp::Run()
{
	MSG msg = { };
	m_timer.Reset();
	while (true) {
		if (PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE)) {
			if (msg.message == WM_QUIT)
			{
				break;
			}

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			//PIXBeginEvent(0, L"Frame Update");

			m_timer.Tick();
			deltaTime = (float)m_timer.GetDeltaTime();

			Update(deltaTime);

		}
	}

	std::cout << "Run 함수 종료\n";
	//m_renderEngine->Quit();
	return (int)msg.wParam;
}

bool Core::SimpleApp::InitDirectX()
{
	UINT dxgiFactoryFlags = 0;

	// Enable the debug layer
#if defined(DEBUG) || defined(_DEBUG)
	{
		ComPtr<ID3D12Debug> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();
			// Enable additional debug layers.
			dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
		}
	}
#endif

	// Create Factory and Device 
	ComPtr<ID3D12Device5> device;
	ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&m_dxgiFactory)));

	HRESULT hardwareResult = D3D12CreateDevice(
		nullptr,
		D3D_FEATURE_LEVEL_11_0,
		IID_PPV_ARGS(&device));

	if (FAILED(hardwareResult))
	{
		ComPtr<IDXGIAdapter> pWarpAdapter;
		m_dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&pWarpAdapter));
		D3D12CreateDevice(
			pWarpAdapter.Get(),
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(&device));;
	}

	ThrowIfFailed(device.As(&m_device));

	Graphics::InitializeCommonState(m_device);
	Renderer::Initialize(m_device);

	m_renderEngine = std::make_shared<RenderEngine>(m_device.Get());
	m_renderEngine->Initialize(m_width, m_height, m_guiWidth, m_dxgiFactory.Get(), m_mainWnd);

	return true;
}

bool Core::SimpleApp::InitGUI()
{
	return true;
}

// BOOKMARK
// main thread
void Core::SimpleApp::Update(float deltaTime)
{
	m_renderEngine->Tick(deltaTime);

}

void Core::SimpleApp::OnResize()
{

}

