#pragma once

namespace COMP_153_Final_Project_Direct3D_Example
{
	// Constant buffer used to send MVP matrices to the vertex shader.
	struct ModelViewProjectionConstantBuffer
	{
		DirectX::XMFLOAT4X4 model;
		DirectX::XMFLOAT4X4 view;
		DirectX::XMFLOAT4X4 projection;
	};

	struct LightingModelConstantBuffer
	{
		DirectX::XMFLOAT4X4 viewMatrix;
		DirectX::XMFLOAT3 globalAmbientLight;
		DirectX::XMFLOAT4 light_color;
		DirectX::XMFLOAT4 light_position;

		float constantAttenuation;
		float linearAttenuation;
		float quadraticAttenuation;
	};

	// Used to send per-vertex data to the vertex shader.
	struct VertexPositionColor
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT3 color;
		DirectX::XMFLOAT3 normal;
	};
}