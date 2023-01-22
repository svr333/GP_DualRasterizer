#pragma once
#include "Camera.h"
#include "Texture.h"
#include "Mesh.h"

struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	class HardwareRenderer final
	{
	public:
		HardwareRenderer(SDL_Window* pWindow, Camera* pCamera);
		~HardwareRenderer();

		HardwareRenderer(const HardwareRenderer&) = delete;
		HardwareRenderer(HardwareRenderer&&) noexcept = delete;
		HardwareRenderer& operator=(const HardwareRenderer&) = delete;
		HardwareRenderer& operator=(HardwareRenderer&&) noexcept = delete;

		ID3D11Device* GetDevice() const;

		void Render(const std::vector<Mesh*>& meshes);
		void CycleSampleState(const std::vector<Mesh*>& meshes);
		void ToggleFireFxMesh();
		void SetUniformColor(bool useUniformColor) { m_UseUniformColor = useUniformColor; };
		void SetCullingMode(Mesh::CullMode cullMode, const std::vector<Mesh*>& meshes);

		enum class SampleState
		{
			Point,
			Linear,
			Antisotropic,
			End
		};

	private:
		Camera* m_pCamera;
		SDL_Window* m_pWindow{};

		int m_Width{};
		int m_Height{};

		bool m_IsInitialized = false;
		SampleState m_SampleState = SampleState::Point;

		ID3D11Device* m_pDevice;
		ID3D11DeviceContext* m_pDeviceContext;

		IDXGIFactory* m_pDXGIFactory;
		IDXGISwapChain* m_pSwapChain;

		ID3D11Texture2D* m_pDepthStencilBuffer;
		ID3D11DepthStencilView* m_pDepthStencilView;
		ID3D11Texture2D* m_pRenderTargetBuffer;
		ID3D11RenderTargetView* m_pRenderTargetView;

		ID3D11SamplerState* m_pSamplerState;
		bool m_RenderFireMesh = true;
		bool m_UseUniformColor = false;
		Mesh::CullMode m_CullMode = Mesh::CullMode::Back;
		ID3D11RasterizerState* m_pRasterizer;

		HRESULT InitializeDirectX();
	};
}
