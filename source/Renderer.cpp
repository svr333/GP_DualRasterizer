#include "pch.h"
#include "Renderer.h"
#include "Effect.h"
#include "TransparentEffect.h"

namespace dae {

	Renderer::Renderer(SDL_Window* pWindow) :
		m_pWindow(pWindow)
	{
		SDL_GetWindowSize(pWindow, &m_Width, &m_Height);

		m_pCamera = new dae::Camera{};
		m_pCamera->Initialize(static_cast<float>(m_Width) / m_Height, 45.0f, { 0.0f, 0.0f, 0.0f });

		m_pHardware = new HardwareRenderer(pWindow, m_pCamera);
		m_pSoftware = new SoftwareRenderer(pWindow, m_pCamera);

		// Initialize meshes
		auto vehicleEffect = new Effect(m_pHardware->GetDevice());
		auto vehicleMesh = new Mesh(m_pHardware->GetDevice(), vehicleEffect, "Resources/vehicle.obj");

		m_pSoftware->SetTextures(vehicleEffect->GetTexture(), vehicleEffect->GetNormal(), vehicleEffect->GetGloss(), vehicleEffect->GetSpecular());
		m_pMeshes.push_back(vehicleMesh);

		auto transEffect = new TransparentEffect(m_pHardware->GetDevice());
		auto fireMesh = new Mesh(m_pHardware->GetDevice(), transEffect, "Resources/fireFX.obj");
		m_pMeshes.push_back(fireMesh);
	}

	Renderer::~Renderer()
	{
		delete m_pCamera;
		delete m_pHardware;
		delete m_pSoftware;

		for (size_t i = 0; i < m_pMeshes.size(); i++)
		{
			delete m_pMeshes[i];
		}
	}

	void Renderer::Update(const Timer* pTimer)
	{
		m_pCamera->Update(pTimer);

		if (m_RotateMesh)
		{
			for (size_t i = 0; i < m_pMeshes.size(); i++)
			{
				m_pMeshes[i]->Rotate(45.0f * TO_RADIANS * pTimer->GetElapsed());
			}
		}
	}

	void Renderer::Render() const
	{
		if (m_RenderMode == RenderMode::Software)
		{
			m_pSoftware->Render(m_pMeshes);
		}
		else
		{
			m_pHardware->Render(m_pMeshes);
		}
	}

	void Renderer::ToggleRasterizerMode()
	{
		m_RenderMode = (RenderMode)(((int)m_RenderMode + 1) % (int)RenderMode::END);
		auto text = (int)m_RenderMode ? "DirectX" : "Software";
		std::cout << "Toggled Rasterization Mode: " << text << "\n";
	}

	void Renderer::ToggleMeshRotation()
	{
		m_RotateMesh = !m_RotateMesh;
		auto text = m_RotateMesh ? "On" : "Off";
		std::cout << "Toggled Mesh Rotation " << text << "\n";
	}

	void Renderer::CycleSampleState()
	{
		m_pHardware->CycleSampleState(m_pMeshes);
	}

	void Renderer::CycleCullingMode()
	{
		m_CullMode = (Mesh::CullMode)(((int)m_CullMode + 1) % (int)Mesh::CullMode::END);
		
		auto text = "Back";

		switch (m_CullMode)
		{
		case Mesh::CullMode::Front:
			text = "Front";
			break;
		case Mesh::CullMode::None:
			text = "None";
			break;
		default:
			break;
		}

		m_pHardware->SetCullingMode(m_CullMode, m_pMeshes);
		m_pSoftware->SetCullingMode(m_CullMode);
		std::cout << "Toggled CullMode: " << text << "\n";
	}

	void Renderer::ToggleUniformColor()
	{
		m_UseUniformColor = !m_UseUniformColor;
		m_pHardware->SetUniformColor(m_UseUniformColor);
		m_pSoftware->SetUniformColor(m_UseUniformColor);

		auto text = m_UseUniformColor ? "Off" : "On";
		std::cout << "Toggled Uniform Color " << text << "\n";
	}
}
