#include "pch.h"
#include "TransparentEffect.h"

#include <sstream>

TransparentEffect::TransparentEffect(ID3D11Device* pDevice)
	: BaseEffect(pDevice, L"Resources/Transparent3D.fx")
{
	m_pTexture = Texture::LoadFromFile(pDevice, "Resources/fireFX_diffuse.png");
	SetDiffuseMap(m_pTexture);
}

Texture* TransparentEffect::GetTexture()
{
	return m_pTexture;
}

ID3D11InputLayout* TransparentEffect::CreateInputLayout(ID3D11Device* pDevice)
{
	// create vertex layout
	static const uint32_t numElements{ 5 };
	D3D11_INPUT_ELEMENT_DESC vertexDesc[numElements]{};

	// not adding all 5 data thingies makes the program not work,
	// so ive added them but theyre unused
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
