#pragma once
#include "Texture.h"
#include "BaseEffect.h"

using namespace dae;

class TransparentEffect : public BaseEffect
{
public:
	TransparentEffect(ID3D11Device* pDevice);

	Texture* GetTexture() override;

	ID3D11InputLayout* CreateInputLayout(ID3D11Device* pDevice) override;
};
