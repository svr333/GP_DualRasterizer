#include "pch.h"
#include "HardwareRenderer.h"
#include "Utils.h"
#include "Effect.h"
#include "TransparentEffect.h"

namespace dae
{
	HardwareRenderer::HardwareRenderer(SDL_Window* pWindow, Camera* camera) 
		: m_pWindow(pWindow), m_pCamera(camera)
	{
		//Initialize
		SDL_GetWindowSize(pWindow, &m_Width, &m_Height);

		//Initialize DirectX pipeline
		const HRESULT result = InitializeDirectX();
		if (result == S_OK)
		{
			m_IsInitialized = true;
			std::cout << "DirectX is initialized and ready!\n";
		}
		else
		{
			std::cout << "DirectX initialization failed!\n";
		}
	}

	HardwareRenderer::~HardwareRenderer()
	{
		if (m_pRasterizer)
		{
			m_pRasterizer->Release();
		}

		if (m_pSamplerState)
		{
			m_pSamplerState->Release();
		}

		m_pRenderTargetView->Release();
		m_pRenderTargetBuffer->Release();
		m_pDepthStencilView->Release();
		m_pDepthStencilBuffer->Release();
		m_pSwapChain->Release();

		if (m_pDeviceContext)
		{
			m_pDeviceContext->ClearState();
			m_pDeviceContext->Flush();
			m_pDeviceContext->Release();
		}

		m_pDevice->Release();
		m_pDXGIFactory->Release();
	}

	ID3D11Device* HardwareRenderer::GetDevice() const
	{
		return m_pDevice;
	}

	void HardwareRenderer::Render(const std::vector<Mesh*>& meshes)
	{
		if (!m_IsInitialized)
		{
			return;
		}

		ColorRGB clearColor = { 0.39f, 0.59f, 0.93f };

		if (m_UseUniformColor)
		{
			clearColor = { 0.1f, 0.1f, 0.1f };
		}

		m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, &clearColor.r);
		m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		if (m_RenderFireMesh)
		{
			for (size_t i = 0; i < meshes.size(); i++)
			{
				meshes[i]->Render(m_pDeviceContext, meshes[i]->GetWorldMatrix() * m_pCamera->viewMatrix * m_pCamera->projectionMatrix);
			}
		}
		else
		{
			for (size_t i = 0; i < std::min<size_t>(meshes.size(), 1); i++)
			{
				meshes[i]->Render(m_pDeviceContext, meshes[i]->GetWorldMatrix() * m_pCamera->viewMatrix * m_pCamera->projectionMatrix);
			}
		}

		m_pSwapChain->Present(0, 0);
	}

	void HardwareRenderer::CycleSampleState(const std::vector<Mesh*>& meshes)
	{
		m_SampleState = SampleState(((int)m_SampleState + 1) % (int)SampleState::End);

		// release previous one so we can create a new one
		if (m_pSamplerState)
		{
			m_pSamplerState->Release();
		}

		D3D11_SAMPLER_DESC sampleDesc{};
		sampleDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		sampleDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		sampleDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

		auto text = "";

		switch (m_SampleState)
		{
			case SampleState::Point:
				sampleDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
				text = "Point";
				break;
			case SampleState::Linear:
				text = "Linear";
				sampleDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
				break;
			case SampleState::Antisotropic:
				text = "Antisotropic";
				sampleDesc.Filter = D3D11_FILTER_ANISOTROPIC;
				break;
			default:
				break;
		}
		
		HRESULT result = m_pDevice->CreateSamplerState(&sampleDesc, &m_pSamplerState);

		if (FAILED(result))
		{
			std::wcout << L"Failed to update Sampler State!\n";
			return;
		}

		std::wcout << L"Sampler state: " << text << "\n";

		for (size_t i = 0; i < meshes.size(); i++)
		{
			meshes[i]->UpdateSampleState(m_pSamplerState);
		}
	}

	void HardwareRenderer::ToggleFireFxMesh()
	{
		m_RenderFireMesh = !m_RenderFireMesh;
		auto text = m_RenderFireMesh ? "On" : "Off";
		std::cout << "Toggled FireFx Mesh " << text << "\n";
	}

	void HardwareRenderer::SetCullingMode(Mesh::CullMode cullMode, const std::vector<Mesh*>& meshes)
	{
		m_CullMode = cullMode;

		D3D11_RASTERIZER_DESC rasterizerDesc{};
		rasterizerDesc.FillMode = D3D11_FILL_SOLID;
		rasterizerDesc.FrontCounterClockwise = false;
		rasterizerDesc.DepthBias = 0;
		rasterizerDesc.SlopeScaledDepthBias = 0.0f;
		rasterizerDesc.DepthBiasClamp = 0.0f;
		rasterizerDesc.DepthClipEnable = true;
		rasterizerDesc.ScissorEnable = false;
		rasterizerDesc.MultisampleEnable = false;
		rasterizerDesc.AntialiasedLineEnable = false;

		switch (cullMode)
		{
			case Mesh::CullMode::Back:
			{
				rasterizerDesc.CullMode = D3D11_CULL_BACK;
				break;
			}
			case Mesh::CullMode::Front:
			{
				rasterizerDesc.CullMode = D3D11_CULL_FRONT;
				break;
			}
			case Mesh::CullMode::None:
			{
				rasterizerDesc.CullMode = D3D11_CULL_NONE;
				break;
			}
		}

		if (m_pRasterizer)
			m_pRasterizer->Release();

		HRESULT hr = m_pDevice->CreateRasterizerState(&rasterizerDesc, &m_pRasterizer);

		if (FAILED(hr))
		{
			std::wcout << L"m_pRasterizerState failed to load.\n";
			return;
		}

		meshes[0]->UpdateRasterizer(m_pRasterizer);
	}

	HRESULT HardwareRenderer::InitializeDirectX()
	{
		D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
		uint32_t deviceFlags = 0;

#if defined(DEBUG) || defined(_DEBUG)
		deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

		HRESULT result = D3D11CreateDevice(0, D3D_DRIVER_TYPE_HARDWARE, 0, deviceFlags, &featureLevel, 1,
			D3D11_SDK_VERSION, &m_pDevice, nullptr, &m_pDeviceContext);

		if (FAILED(result))
		{
			return result;
		}

		result = CreateDXGIFactory(__uuidof(IDXGIFactory), reinterpret_cast<void**>(&m_pDXGIFactory));

		if (FAILED(result))
		{
			return result;
		}

		// get window info
		SDL_SysWMinfo sysWMInfo{};
		SDL_VERSION(&sysWMInfo.version);
		SDL_GetWindowWMInfo(m_pWindow, &sysWMInfo);

		// initialize swap chain settings
		DXGI_SWAP_CHAIN_DESC swapChainDesc{};
		swapChainDesc.BufferDesc.Width = m_Width;
		swapChainDesc.BufferDesc.Height = m_Height;
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 1;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 60;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 1;
		swapChainDesc.Windowed = true;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		swapChainDesc.Flags = 0;
		swapChainDesc.OutputWindow = sysWMInfo.info.win.window;

		result = m_pDXGIFactory->CreateSwapChain(m_pDevice, &swapChainDesc, &m_pSwapChain);

		if (FAILED(result))
		{
			return result;
		}

		// depth buffer init
		D3D11_TEXTURE2D_DESC depthStencilDesc{};
		depthStencilDesc.Width = m_Width;
		depthStencilDesc.Height = m_Height;
		depthStencilDesc.MipLevels = 1;
		depthStencilDesc.ArraySize = 1;
		depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
		depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
		depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthStencilDesc.CPUAccessFlags = 0;
		depthStencilDesc.MiscFlags = 0;

		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
		depthStencilViewDesc.Format = depthStencilDesc.Format;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;

		result = m_pDevice->CreateTexture2D(&depthStencilDesc, nullptr, &m_pDepthStencilBuffer);

		if (FAILED(result))
		{
			return result;
		}

		result = m_pDevice->CreateDepthStencilView(m_pDepthStencilBuffer, &depthStencilViewDesc, &m_pDepthStencilView);

		if (FAILED(result))
		{
			return result;
		}

		result = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&m_pRenderTargetBuffer));

		if (FAILED(result))
		{
			return result;
		}

		result = m_pDevice->CreateRenderTargetView(m_pRenderTargetBuffer, 0, &m_pRenderTargetView);

		if (FAILED(result))
		{
			return result;
		}

		m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);

		D3D11_VIEWPORT viewPort{};
		viewPort.Width = static_cast<float>(m_Width);
		viewPort.Height = static_cast<float>(m_Height);
		viewPort.TopLeftX = 0.0f;
		viewPort.TopLeftY = 0.0f;
		viewPort.MinDepth = 0.0f;
		viewPort.MaxDepth = 1.0f;

		m_pDeviceContext->RSSetViewports(1, &viewPort);

		return result;
	}
}
