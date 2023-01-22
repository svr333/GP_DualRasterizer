#include "pch.h"
#include "Effect.h"

#include <sstream>

Effect::Effect(ID3D11Device* pDevice)
	: BaseEffect(pDevice, L"Resources/PosCol3D.fx")
{
	//--------------------------------
	// Matrices
	//--------------------------------
	m_pMatWorld = m_pEffect->GetVariableByName("gWorld")->AsMatrix();

	if (!m_pMatWorld->IsValid())
	{
		std::wcout << L"m_pMatWorldVar is not valid!\n";
	}
	m_pMatViewInverse = m_pEffect->GetVariableByName("gViewInverse")->AsMatrix();

	if (!m_pMatViewInverse->IsValid())
	{
		std::wcout << L"m_pMatViewInverseVar is not valid!\n";
	}

	//--------------------------------
	// Texture2D
	//--------------------------------
	m_pNormalMap = m_pEffect->GetVariableByName("gNormalMap")->AsShaderResource();

	if (!m_pNormalMap->IsValid())
	{
		std::wcout << L"NormalMap is invalid.\n";
	}

	m_pSpecularMap = m_pEffect->GetVariableByName("gSpecularMap")->AsShaderResource();

	if (!m_pSpecularMap->IsValid())
	{
		std::wcout << L"SpecularMap is invalid.\n";
	}

	m_pGlossMap = m_pEffect->GetVariableByName("gGlossinessMap")->AsShaderResource();

	if (!m_pGlossMap->IsValid())
	{
		std::wcout << L"GlossinessMap is invalid.\n";
	}

	// set textures
	m_pTexture = Texture::LoadFromFile(pDevice, "Resources/vehicle_diffuse.png");
	SetDiffuseMap(m_pTexture);

	m_pNormal = Texture::LoadFromFile(pDevice, "Resources/vehicle_normal.png");
	SetNormalMap(m_pNormal);

	m_pSpecular = Texture::LoadFromFile(pDevice, "Resources/vehicle_specular.png");
	SetSpecularMap(m_pSpecular);

	m_pGloss = Texture::LoadFromFile(pDevice, "Resources/vehicle_gloss.png");
	SetGlossMap(m_pGloss);
}

Effect::~Effect()
{
	m_pSpecularMap->Release();
	m_pNormalMap->Release();
	m_pGlossMap->Release();
	m_pMatViewInverse->Release();
	m_pMatWorld->Release();

	delete m_pGloss;
	delete m_pSpecular;
	delete m_pNormal;
	delete m_pTexture;
}

Texture* Effect::GetTexture()
{
	return m_pTexture;
}

Texture* Effect::GetNormal()
{
	return m_pNormal;
}

Texture* Effect::GetGloss()
{
	return m_pGloss;
}

Texture* Effect::GetSpecular()
{
	return m_pSpecular;
}

ID3D11InputLayout* Effect::CreateInputLayout(ID3D11Device* pDevice)
{
	// create vertex layout
	static const uint32_t numElements{ 5 };
	D3D11_INPUT_ELEMENT_DESC vertexDesc[numElements]{};

	vertexDesc[0].SemanticName = "POSITION";
	vertexDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[0].AlignedByteOffset = 0;
	vertexDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[1].SemanticName = "COLOR";
	vertexDesc[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[1].AlignedByteOffset = 12;
	vertexDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[2].SemanticName = "TEXCOORD";
	vertexDesc[2].Format = DXGI_FORMAT_R32G32_FLOAT;
	vertexDesc[2].AlignedByteOffset = 24;
	vertexDesc[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[3].SemanticName = "NORMAL";
	vertexDesc[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[3].AlignedByteOffset = 32;
	vertexDesc[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[4].SemanticName = "TANGENT";
	vertexDesc[4].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[4].AlignedByteOffset = 44;
	vertexDesc[4].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	ID3D11InputLayout* pVertexLayout;

	// create input layout
	D3DX11_PASS_DESC passDesc;
	GetTechnique()->GetPassByIndex(0)->GetDesc(&passDesc);
	HRESULT result = pDevice->CreateInputLayout(vertexDesc, numElements, passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &pVertexLayout);

	if (FAILED(result))
	{
		throw;
	}

	return pVertexLayout;
}

ID3DX11EffectMatrixVariable* Effect::GetWorldMatrix()
{
	return m_pMatWorld;
}

ID3DX11EffectMatrixVariable* Effect::GetViewInverseMatrix()
{
	return m_pMatViewInverse;
}

void Effect::SetNormalMap(dae::Texture* pNormalMap)
{
	if (m_pNormalMap)
	{
		m_pNormalMap->SetResource(pNormalMap->GetSRV());
	}
}

void Effect::SetSpecularMap(dae::Texture* pSpecularMap)
{
	if (m_pSpecularMap)
	{
		m_pSpecularMap->SetResource(pSpecularMap->GetSRV());
	}
}

void Effect::SetGlossMap(dae::Texture* pGlossMap)
{
	if (m_pGlossMap)
	{
		m_pGlossMap->SetResource(pGlossMap->GetSRV());
	}
}
