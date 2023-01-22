#pragma once
#include "Camera.h"
#include "HardwareRenderer.h"
#include "SoftwareRenderer.h"
#include "Mesh.h"

struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	class Renderer final
	{
	public:
		enum class RenderMode
		{
			Software = 0,
			Hardware = 1,
			END = 2
		};

		Renderer(SDL_Window* pWindow);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void Update(const Timer* pTimer);
		void Render() const;

		RenderMode GetRenderMode() { return m_RenderMode; };
		HardwareRenderer* GetHardwareRenderer() { return m_pHardware; };
		SoftwareRenderer* GetSoftwareRenderer() { return m_pSoftware; };

		void ToggleRasterizerMode();
		void ToggleMeshRotation();
		void CycleSampleState();
		void CycleCullingMode();
		void ToggleUniformColor();

	private:
		dae::Camera* m_pCamera;
		SDL_Window* m_pWindow{};
		dae::HardwareRenderer* m_pHardware;
		dae::SoftwareRenderer* m_pSoftware;

		std::vector<Mesh*> m_pMeshes;

		RenderMode m_RenderMode = RenderMode::Hardware;
		Mesh::CullMode m_CullMode = Mesh::CullMode::Back;

		int m_Width{};
		int m_Height{};

		bool m_RotateMesh;
		bool m_UseUniformColor;
	};
}
