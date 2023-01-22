#pragma once
#include "Texture.h"

using namespace dae;

class BaseEffect
{
public:
	BaseEffect(ID3D11Device* pDevice, const std::wstring& assetFile);
	virtual ~BaseEffect();
	ID3DX11Effect* GetEffect();
	ID3DX11EffectTechnique* GetTechnique();

	ID3DX11EffectMatrixVariable* GetWorldViewMatrix();
	virtual ID3DX11EffectMatrixVariable* GetWorldMatrix() { return nullptr; };
	virtual ID3DX11EffectMatrixVariable* GetViewInverseMatrix() { return nullptr; };

	virtual Texture* GetTexture() { return nullptr; };
	virtual Texture* GetNormal() { return nullptr; };
	virtual Texture* GetGloss() { return nullptr; };
	virtual Texture* GetSpecular() { return nullptr; };

	virtual ID3D11InputLayout* CreateInputLayout(ID3D11Device* pDevice) = 0;
	void SetDiffuseMap(Texture* pDiffuseMap);
	void SetRasterizer(ID3D11RasterizerState* rasterizer);

protected:
	ID3DX11Effect* m_pEffect;
	ID3DX11EffectTechnique* m_pTechnique;
	ID3DX11EffectShaderResourceVariable* m_pDiffuseMap;
	ID3DX11EffectRasterizerVariable* m_pRasterizerState;

	Texture* m_pTexture;

	ID3DX11EffectMatrixVariable* m_pMatWorldViewProj;
};
