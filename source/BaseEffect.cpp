#include "pch.h"
#include "BaseEffect.h"

BaseEffect::BaseEffect(ID3D11Device* pDevice, const std::wstring& assetFile)
	: m_pEffect(), m_pTechnique()
{
	HRESULT result = S_OK;
	ID3D10Blob* pErrorBlob = nullptr;
	ID3DX11Effect* pEffect;

	DWORD shaderFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
	shaderFlags |= D3DCOMPILE_DEBUG;
	shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	result = D3DX11CompileEffectFromFile(assetFile.c_str(), nullptr, nullptr, shaderFlags, 0, pDevice, &pEffect, &pErrorBlob);

	if (!FAILED(result))
	{
		m_pEffect = pEffect;

		m_pTechnique = m_pEffect->GetTechniqueByName("DefaultTechnique");

		if (!m_pTechnique->IsValid())
		{
			std::wcout << "Technique is invalid.\n";
		}

		m_pMatWorldViewProj = m_pEffect->GetVariableByName("gWorldViewProj")->AsMatrix();

		if (!m_pMatWorldViewProj->IsValid())
		{
			std::wcout << L"WorldViewProjMatrix is invalid.\n";
		}

		m_pDiffuseMap = m_pEffect->GetVariableByName("gDiffuseMap")->AsShaderResource();

		if (!m_pDiffuseMap->IsValid())
		{
			std::wcout << L"DiffuseMap is invalid.\n";
		}

		m_pRasterizerState = m_pEffect->GetVariableByName("gRasterizerState")->AsRasterizer();

		if (!m_pRasterizerState->IsValid())
		{
			std::wcout << L"RasterizerState is invalid.\n";
		}
	}

	if (pErrorBlob)
	{
		std::cout << "Error occured in Effect.cpp.\n";

		char* pErrors = (char*)pErrorBlob->GetBufferPointer();

		std::wstringstream ss;
		for (unsigned int i = 0; i < pErrorBlob->GetBufferSize(); ++i)
		{
			ss << pErrors[i];
		}

		OutputDebugStringW(ss.str().c_str());
		std::wcout << ss.str() << "\n";

		pErrorBlob->Release();
	}
}

BaseEffect::~BaseEffect()
{
	m_pDiffuseMap->Release();
	m_pMatWorldViewProj->Release();
	m_pTechnique->Release();
	m_pEffect->Release();

	delete m_pTexture;
}

ID3DX11Effect* BaseEffect::GetEffect()
{
	return m_pEffect;
}

ID3DX11EffectTechnique* BaseEffect::GetTechnique()
{
	return m_pTechnique;
}

ID3DX11EffectMatrixVariable* BaseEffect::GetWorldViewMatrix()
{
	return m_pMatWorldViewProj;
}

void BaseEffect::SetDiffuseMap(Texture* pDiffuseMap)
{
	if (m_pDiffuseMap)
	{
		m_pDiffuseMap->SetResource(pDiffuseMap->GetSRV());
	}
}

void BaseEffect::SetRasterizer(ID3D11RasterizerState* rasterizer)
{
	HRESULT hr = m_pRasterizerState->SetRasterizerState(0, rasterizer);

	if (FAILED(hr))
	{
		std::wcout << L"Failed to set rasterizer state";
	}
}
