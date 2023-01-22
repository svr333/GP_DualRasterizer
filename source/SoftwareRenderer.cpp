//External includes
#include "SDL.h"
#include "SDL_surface.h"
#include "pch.h"

//Project includes
#include "SoftwareRenderer.h"
#include <iostream>
#include "Mesh.h"
#include <cstdint>
#include <vector>

SoftwareRenderer::SoftwareRenderer(SDL_Window* pWindow, Camera* camera) :
	m_pWindow(pWindow), m_pCamera(camera)
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);

	//Create Buffers
	m_pFrontBuffer = SDL_GetWindowSurface(pWindow);
	m_pBackBuffer = SDL_CreateRGBSurface(0, m_Width, m_Height, 32, 0, 0, 0, 0);
	m_pBackBufferPixels = (uint32_t*)m_pBackBuffer->pixels;

	m_pDepthBufferPixels = new float[m_Width * m_Height];
}

SoftwareRenderer::~SoftwareRenderer()
{
	delete[] m_pDepthBufferPixels;
}

void SoftwareRenderer::Render(const std::vector<Mesh*>& meshes)
{
	// Lock BackBuffer
	SDL_LockSurface(m_pBackBuffer);

	// Clear BackBuffer
	// convert rgb to decimal
	auto decimalColor = (100 << 16) + (100 << 8) + 100;

	if (m_UseUniformColor)
	{
		decimalColor = (25 << 16) + (25 << 8) + 25;
	}
	
	SDL_FillRect(m_pBackBuffer, NULL, decimalColor);

	// Initialize Depth buffer
	std::fill_n(m_pDepthBufferPixels, m_Width * m_Height, 99999999999999.0f);

	// only render first mesh, not the fire particles
	std::vector<Mesh::Vertex_Out> verticesOut;
	VertexTransformationFunction(meshes[0], verticesOut);
	RenderMesh(meshes[0], verticesOut);

	SDL_UnlockSurface(m_pBackBuffer);
	SDL_BlitSurface(m_pBackBuffer, 0, m_pFrontBuffer, 0);
	SDL_UpdateWindowSurface(m_pWindow);
}

void SoftwareRenderer::SetTextures(Texture* pTexture, Texture* pNormal, Texture* pGloss, Texture* pSpecular)
{
	m_pTexture = pTexture;
	m_pNormal = pNormal;
	m_pGloss = pGloss;
	m_pSpecular = pSpecular;
}

void SoftwareRenderer::VertexTransformationFunction(Mesh* mesh, std::vector<Mesh::Vertex_Out>& verticesOut) const
{
	auto worldMatrix = mesh->GetWorldMatrix();
	auto verticesIn = mesh->GetVertices();

	Matrix matrix{ worldMatrix * m_pCamera->viewMatrix * m_pCamera->projectionMatrix };
	
	verticesOut.clear();
	verticesOut.reserve(verticesIn.size());

	for (size_t i{}; i < verticesIn.size(); ++i)
	{
		Mesh::Vertex_Out v{};

		v.position = matrix.TransformPoint({ verticesIn[i].position, 1.f });

		v.position.x /= v.position.w;
		v.position.y /= v.position.w;
		v.position.z /= v.position.w;

		v.position.x = ((1.f + v.position.x) / 2.f) * m_Width;
		v.position.y = ((1.f - v.position.y) / 2.f) * m_Height;

		v.color = verticesIn[i].color;
		v.uv = verticesIn[i].uv;
		v.normal = worldMatrix.TransformVector(verticesIn[i].normal);
		v.tangent = worldMatrix.TransformVector(verticesIn[i].tangent);

		verticesOut.emplace_back(v);
	}
}

bool SoftwareRenderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBackBuffer, "Rasterizer_ColorBuffer.bmp");
}

void SoftwareRenderer::ToggleDepthBufferVisualization()
{
	m_DepthBufferVisualization = !m_DepthBufferVisualization;
	auto text = m_DepthBufferVisualization ? "On" : "Off";
	std::cout << "Toggled Depth Visualization " << m_DepthBufferVisualization << "\n";
}

void SoftwareRenderer::ToggleNormalMap()
{
	m_UseNormalMap = !m_UseNormalMap;
	auto text = m_UseNormalMap ? "On" : "Off";
	std::cout << "Toggled Normal Map " << text << "\n";
}

void SoftwareRenderer::CycleLightingMode()
{
	m_LightingMode = LightingMode(((int)m_LightingMode + 1) % (int)LightingMode::End);

	auto text = m_LightingMode == LightingMode::ObservedArea ? "Observed Area" : m_LightingMode == LightingMode::Diffuse ? "Diffuse" : m_LightingMode == LightingMode::Specular ? "Specular" : "Combined";
	std::cout << "Toggled Lighting Mode To: " << text << "\n";
}

void dae::SoftwareRenderer::ToggleBoundingBoxVisualization()
{
	m_BoundingBoxVisualization = !m_BoundingBoxVisualization;
	auto text = m_BoundingBoxVisualization ? "On" : "Off";
	std::cout << "Toggled Bounding Box Visualization " << text << "\n";
}

// Private functions
void SoftwareRenderer::RenderTriangle(const Mesh::Vertex_Out& v0, const Mesh::Vertex_Out& v1, const Mesh::Vertex_Out& v2) const
{
	// Frustum culling x & y
	if (v0.position.x < 0 || v1.position.x < 0 || v2.position.x < 0 ||
		v0.position.x > m_Width || v1.position.x > m_Width || v2.position.x > m_Width ||
		v0.position.y < 0 || v1.position.y < 0 || v2.position.y < 0 ||
		v0.position.y > m_Height || v1.position.y > m_Height || v2.position.y > m_Height)
	{
		return;
	}

	Vector2 edge0 = { v2.position.GetXY() - v1.position.GetXY() };
	Vector2 edge1 = { v0.position.GetXY() - v2.position.GetXY() };
	Vector2 edge2 = { v1.position.GetXY() - v0.position.GetXY() };

	float area = Vector2::Cross(edge0, edge1);

	if (area < 1.0f)
	{
		return;
	}

	auto top = std::max<float>(std::max<float>(v0.position.y, v1.position.y), v2.position.y);
	auto bottom = std::min<float>(std::min<float>(v0.position.y, v1.position.y), v2.position.y);
	auto left = std::min<float>(std::min<float>(v0.position.x, v1.position.x), v2.position.x);
	auto right = std::max<float>(std::max<float>(v0.position.x, v1.position.x), v2.position.x);

	for (int px{ static_cast<int>(left) }; px < static_cast<int>(right); ++px)
	{
		for (int py{ static_cast<int>(bottom) }; py < static_cast<int>(top); ++py)
		{
			if (m_BoundingBoxVisualization)
			{
				m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
					static_cast<uint8_t>(255),
					static_cast<uint8_t>(255),
					static_cast<uint8_t>(255));
				continue;
			}

			Vector2 pixel{ static_cast<float>(px), static_cast<float>(py) };

			Vector2 p0ToPixel = pixel - v0.position.GetXY();
			auto w2 = Vector2::Cross(edge2, p0ToPixel) / area;

			if (w2 < 0.0f)
			{
				continue;
			}

			Vector2 p1ToPixel = pixel - v1.position.GetXY();
			auto w0 = Vector2::Cross(edge0, p1ToPixel) / area;

			if (w0 < 0.0f)
			{
				continue;
			}

			Vector2 p2ToPixel = pixel - v2.position.GetXY();
			auto w1 = Vector2::Cross(edge1, p2ToPixel) / area;

			if (w1 < 0.0f)
			{
				continue;
			}

			// Deoth Buffer
			float depthBuffer = 1.f / (w0 / v0.position.z + w1 / v1.position.z + w2 / v2.position.z);

			// frustum culling z + depth test
			if (depthBuffer < 0 || depthBuffer > 1 ||
				depthBuffer > m_pDepthBufferPixels[px + py * m_Width])
			{
				continue;
			}

			m_pDepthBufferPixels[px + py * m_Width] = depthBuffer;

			// actual depth
			w0 /= v0.position.w;
			w1 /= v1.position.w;
			w2 /= v2.position.w;

			auto depth = 1.0f / (w0 + w1 + w2);

			ColorRGB finalColor{};

			if (m_DepthBufferVisualization)
			{
				// Remap so it isnt too bright 
				depthBuffer = (depthBuffer - 0.985f) / (1.0f - 0.985f);

				depthBuffer = Clamp(depthBuffer, 0.f, 1.f);
				finalColor = { depthBuffer, depthBuffer, depthBuffer };
			}
			else
			{
				Mesh::Vertex_Out shadingVertex{};
				shadingVertex.position.x = (float)px;
				shadingVertex.position.y = (float)py;
				shadingVertex.color = (w0 * v0.color + w1 * v1.color + w2 * v2.color) * depth;
				shadingVertex.uv = (w0 * v0.uv + w1 * v1.uv + w2 * v2.uv) * depth;
				shadingVertex.normal = ((w0 * v0.normal + w1 * v1.normal + w2 * v2.normal) * depth).Normalized();
				shadingVertex.tangent = ((w0 * v0.tangent + w1 * v1.tangent + w2 * v2.tangent) * depth).Normalized();

				finalColor = PixelShading(shadingVertex);
			}

			finalColor.MaxToOne();

			m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
				static_cast<uint8_t>(finalColor.r * 255),
				static_cast<uint8_t>(finalColor.g * 255),
				static_cast<uint8_t>(finalColor.b * 255));
		}
	}
}

void SoftwareRenderer::RenderMesh(Mesh* mesh, std::vector<Mesh::Vertex_Out>& vertices) const
{
	auto indices = mesh->GetIndices();

	if (mesh->GetPrimitiveTopology() == Mesh::PrimitiveTopology::TriangleList)
	{
		for (size_t i = 0; i < indices.size() - 2; i += 3)
		{
			RenderTriangle(vertices[indices[i]], vertices[indices[i + 1]], vertices[indices[i + 2]]);
		}
	}
	else if (mesh->GetPrimitiveTopology() == Mesh::PrimitiveTopology::TriangleStrip)
	{
		for (size_t i = 0; i < indices.size() - 2; ++i)
		{
			// try optimize without if statement, either 2 for loops or just adding/substracting the result of the modulo directly
			if (i % 2)
			{
				RenderTriangle(vertices[indices[i]], vertices[indices[i + 2]], vertices[indices[i + 1]]);
			}
			else
			{
				RenderTriangle(vertices[indices[i]], vertices[indices[i + 1]], vertices[indices[i + 2]]);
			}
		}
	}
}

ColorRGB SoftwareRenderer::PixelShading(const Mesh::Vertex_Out& v) const
{
	Vector3 lightDirection = { .577f, -.577f, .577f };
	Vector3 normal{ v.normal };

	// Create viewDirection
	float x = (2 * (v.position.x + 0.5f / float(m_Width)) - 1) * m_pCamera->aspectRatio * m_pCamera->fov;
	float y = (1 - (2 * (v.position.y + 0.5f / float(m_Height)))) * m_pCamera->fov;

	Vector3 viewDirection = (x * m_pCamera->right + y * m_pCamera->up + m_pCamera->forward).Normalized();

	if (m_UseNormalMap)
	{
		// Create tangent space transformation matrix
		Vector3 binormal = Vector3::Cross(v.normal, v.tangent);
		Matrix tangentSpaceAxis{ v.tangent, binormal, v.normal, Vector3::Zero };

		// sample and remap color to [-1, 1]
		ColorRGB sampledColor = m_pNormal->Sample(v.uv);
		sampledColor = (2.f * sampledColor) - ColorRGB{ 1.f, 1.f, 1.f };

		normal = tangentSpaceAxis.TransformVector(sampledColor.r, sampledColor.g, sampledColor.b);
	}

	float dot = normal * -lightDirection;

	if (dot < 0.f)
	{
		return {};
	}

	ColorRGB finalColor{};
	auto lightIntensity = 7.0f;
	auto shine = 25.0f;

	float dot2 = -1;

	switch (m_LightingMode)
	{
		case SoftwareRenderer::LightingMode::ObservedArea:
			finalColor = { dot, dot, dot };
			break;
		case SoftwareRenderer::LightingMode::Diffuse:
			finalColor = m_pTexture->Sample(v.uv) * dot * lightIntensity / M_PI;
			break;
		case SoftwareRenderer::LightingMode::Specular:
			finalColor = Phong(m_pSpecular->Sample(v.uv), shine * m_pGloss->Sample(v.uv).r, -lightDirection, viewDirection, normal) * dot;
			break;
		case SoftwareRenderer::LightingMode::Combined:
		default:
			finalColor = m_pTexture->Sample(v.uv) * dot * lightIntensity / M_PI;
			finalColor += Phong(m_pSpecular->Sample(v.uv), shine * m_pGloss->Sample(v.uv).r, -lightDirection, viewDirection, normal) * dot;
			break;
	}

	finalColor.MaxToOne();
	return finalColor;
}

ColorRGB SoftwareRenderer::Phong(ColorRGB specular, float gloss, Vector3 lightDir, Vector3 viewDir, Vector3 normal) const
{
	auto dot = (lightDir - (normal * (2.f * (normal * lightDir)))) * viewDir;

	if (dot < 0.f)
	{
		return {};
	}

	return specular * powf(dot, gloss);
}
