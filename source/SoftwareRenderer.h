#pragma once

#include "Mesh.h"
#include "Camera.h"

struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	class SoftwareRenderer final
	{
	public:
		SoftwareRenderer(SDL_Window* pWindow, Camera* pCamera);
		~SoftwareRenderer();

		SoftwareRenderer(const SoftwareRenderer&) = delete;
		SoftwareRenderer(SoftwareRenderer&&) noexcept = delete;
		SoftwareRenderer& operator=(const SoftwareRenderer&) = delete;
		SoftwareRenderer& operator=(SoftwareRenderer&&) noexcept = delete;

		void Render(const std::vector<Mesh*>& meshes);
		void SetTextures(Texture* pTexture, Texture* pNormal, Texture* pGloss, Texture* pSpecular);

		void SetUniformColor(bool useUniformColor) { m_UseUniformColor = useUniformColor; };
		void SetCullingMode(Mesh::CullMode cullMode) { m_CullMode = cullMode; };

		bool SaveBufferToImage() const;
		void ToggleDepthBufferVisualization();
		void ToggleNormalMap();
		void CycleLightingMode();
		void ToggleBoundingBoxVisualization();

	private:
		Camera* m_pCamera;
		SDL_Window* m_pWindow{};
		int m_Width{};
		int m_Height{};

		SDL_Surface* m_pFrontBuffer{ nullptr };
		SDL_Surface* m_pBackBuffer{ nullptr };
		uint32_t* m_pBackBufferPixels{};
		float* m_pDepthBufferPixels{};

		enum class LightingMode
		{
			ObservedArea,
			Diffuse,
			Specular,
			Combined,
			End
		};

		LightingMode m_LightingMode{ LightingMode::Combined };
		bool m_BoundingBoxVisualization = false;
		bool m_DepthBufferVisualization = false;
		bool m_RotateMesh = false;
		float m_MeshRotation = 0.0f;
		bool m_UseNormalMap = true;
		bool m_UseUniformColor;
		Mesh::CullMode m_CullMode;

		Texture* m_pTexture = nullptr;
		Texture* m_pNormal = nullptr;
		Texture* m_pGloss = nullptr;
		Texture* m_pSpecular = nullptr;

		void VertexTransformationFunction(Mesh* mesh, std::vector<Mesh::Vertex_Out>& verticesOut) const;

		void RenderTriangle(const Mesh::Vertex_Out& v0, const Mesh::Vertex_Out& v1, const Mesh::Vertex_Out& v2) const;
		void RenderMesh(Mesh* mesh, std::vector<Mesh::Vertex_Out>& vertices) const;

		ColorRGB PixelShading(const Mesh::Vertex_Out& v) const;
		ColorRGB Phong(ColorRGB specular, float gloss, Vector3 lightDir, Vector3 viewDir, Vector3 normal) const;
	};
}
