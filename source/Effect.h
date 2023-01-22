#pragma once
#include "Texture.h"
#include "BaseEffect.h"

using namespace dae;

class Effect : public BaseEffect
{
public:
	Effect(ID3D11Device* pDevice);
	~Effect();

	ID3D11InputLayout* CreateInputLayout(ID3D11Device* pDevice);

	ID3DX11EffectMatrixVariable* GetWorldMatrix() override;
	ID3DX11EffectMatrixVariable* GetViewInverseMatrix() override;

	Texture* GetTexture() override;
	Texture* GetNormal() override;
	Texture* GetGloss() override;
	Texture* GetSpecular() override;

	void SetNormalMap(Texture* pNormalMap);
	void SetSpecularMap(Texture* pSpecularMap);
	void SetGlossMap(Texture* pGlossMap);

private:
	ID3DX11EffectMatrixVariable* m_pMatWorld;
	ID3DX11EffectMatrixVariable* m_pMatViewInverse;

	ID3DX11EffectShaderResourceVariable* m_pNormalMap;
	ID3DX11EffectShaderResourceVariable* m_pSpecularMap;
	ID3DX11EffectShaderResourceVariable* m_pGlossMap;

	Texture* m_pTexture;
	Texture* m_pNormal;
	Texture* m_pSpecular;
	Texture* m_pGloss;
};
