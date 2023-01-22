#include "pch.h"
#include "Mesh.h"
#include "Effect.h"
#include "Utils.h"

Mesh::Mesh(ID3D11Device* pDevice, BaseEffect* pEffect, const std::string& filePath)
	: m_pEffect(pEffect)
{
	Utils::ParseOBJ(filePath, m_Vertices, m_Indices);

	m_pVertexLayout = m_pEffect->CreateInputLayout(pDevice);

	// create vertex buffer
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(Vertex_In) * (uint32_t)m_Vertices.size();
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData = { 0 };
	initData.pSysMem = m_Vertices.data();
	
	auto result = pDevice->CreateBuffer(&bd, &initData, &m_pVertexBuffer);

	if (FAILED(result))
	{
		throw;
	}

	// create index buffer
	bd = {};
	m_AmountIndices = static_cast<uint32_t>(m_Indices.size());
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(uint32_t) * m_AmountIndices;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	
	initData = { 0 };
	initData.pSysMem = m_Indices.data();
	
	result = pDevice->CreateBuffer(&bd, &initData, &m_pIndexBuffer);

	if (FAILED(result))
	{
		throw;
	}
}

void Mesh::Render(ID3D11DeviceContext* pDeviceContext, const Matrix& worldViewProjMatrix, const Matrix& worldMatrix, const Matrix& invViewMatrix)
{
	// Set primitive topology
	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Set input layout
	pDeviceContext->IASetInputLayout(m_pVertexLayout);

	// set vertex buffer
	UINT stride = sizeof(Vertex_In);
	UINT offset = 0;
	pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

	// set index buffer
	pDeviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// set input layout
	pDeviceContext->IASetInputLayout(m_pVertexLayout);


	// set matrices
	m_pEffect->GetWorldViewMatrix()->SetMatrix((float*)(&worldViewProjMatrix));

	if (m_pEffect->GetWorldMatrix())
	{
		m_pEffect->GetWorldMatrix()->SetMatrix((float*)&worldMatrix);
	}
	
	if (m_pEffect->GetViewInverseMatrix())
	{
		m_pEffect->GetViewInverseMatrix()->SetMatrix((float*)&invViewMatrix);
	}

	// render a triangle
	D3DX11_TECHNIQUE_DESC techDesc;
	m_pEffect->GetTechnique()->GetDesc(&techDesc);

	for (UINT i = 0; i < techDesc.Passes; i++)
	{
		m_pEffect->GetTechnique()->GetPassByIndex(0)->Apply(0, pDeviceContext);
		pDeviceContext->DrawIndexed(m_AmountIndices, 0, 0);
	}
}

void Mesh::Render(ID3D11DeviceContext* pDeviceContext, const Matrix& worldViewProjMatrix)
{
	Render(pDeviceContext, worldViewProjMatrix, {}, {});
}

Matrix Mesh::GetWorldMatrix()
{
	return m_MatWorld;
}

std::vector<Mesh::Vertex_In> Mesh::GetVertices()
{
	return m_Vertices;
}

std::vector<uint32_t> Mesh::GetIndices()
{
	return m_Indices;
}

void Mesh::Rotate(float newAngle)
{
	
	m_MatWorld = Matrix::CreateRotationY(newAngle) * m_MatWorld;
}

void Mesh::UpdateRasterizer(ID3D11RasterizerState* rasterizer)
{
	m_pEffect->SetRasterizer(rasterizer);
}

void Mesh::UpdateSampleState(ID3D11SamplerState* pSampleState)
{
	if (!pSampleState)
	{
		return;
	}

	HRESULT result = m_pEffect->GetEffect()->GetVariableByName("samPoint")->AsSampler()->SetSampler(0, pSampleState);

	if (FAILED(result))
	{
		std::wcout << L"Couldn't update sampler state\n";
		throw;
	}
}

Mesh::~Mesh()
{
	m_pVertexLayout->Release();
	m_pIndexBuffer->Release();
	m_pVertexBuffer->Release();

	delete m_pEffect;
}
