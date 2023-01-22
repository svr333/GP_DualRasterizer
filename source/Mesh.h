#pragma once
#include "Math.h"
#include <vector>
#include "Texture.h"
#include "BaseEffect.h"

using namespace dae;

class Effect;

class Mesh
{
public:
	struct Vertex_In
	{
		Vector3 position;
		ColorRGB color;
		Vector2 uv;
		Vector3 normal;
		Vector3 tangent;
	};

	struct Vertex_Out
	{
		Vector4 position;
		ColorRGB color;
		Vector2 uv;
		Vector3 normal;
		Vector3 tangent;
	};

	enum class PrimitiveTopology
	{
		TriangleList,
		TriangleStrip
	};

	enum class CullMode
	{
		Back = 0,
		Front = 1,
		None = 2,
		END = 3
	};

	Mesh(ID3D11Device* pDevice, BaseEffect* pEffect, const std::string& filePath);
	void Render(ID3D11DeviceContext* pDeviceContext, const Matrix& worldViewProjMatrix, const Matrix& worldMatrix, const Matrix& invViewMatrix);
	void Render(ID3D11DeviceContext* pDeviceContext, const Matrix& worldViewProjMatrix);

	Matrix GetWorldMatrix();
	std::vector<Vertex_In> GetVertices();
	std::vector<uint32_t> GetIndices();
	PrimitiveTopology GetPrimitiveTopology() { return m_Topology; };

	void Rotate(float newAngle);
	void UpdateRasterizer(ID3D11RasterizerState* rasterizer);
	void UpdateSampleState(ID3D11SamplerState* pSampleState);
	~Mesh();

private:
	PrimitiveTopology m_Topology = PrimitiveTopology::TriangleList;
	std::vector<Vertex_In> m_Vertices;
	std::vector<uint32_t> m_Indices;

	Matrix m_MatWorld{ Vector3::UnitX, Vector3::UnitY, Vector3::UnitZ, { 0.0f, 0.0f, 50.0f } };

	BaseEffect* m_pEffect;
	ID3D11InputLayout* m_pVertexLayout;
	ID3D11Buffer* m_pVertexBuffer;
	ID3D11Buffer* m_pIndexBuffer;

	int m_AmountIndices;
};
