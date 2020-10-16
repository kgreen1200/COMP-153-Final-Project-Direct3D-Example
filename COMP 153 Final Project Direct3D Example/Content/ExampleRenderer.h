#pragma once

#include "..\Common\DeviceResources.h"
#include "ShaderStructures.h"
#include "..\Common\StepTimer.h"
#include <Content\ShaderStructures.h>

namespace COMP_153_Final_Project_Direct3D_Example
{
	// This sample renderer instantiates a basic rendering pipeline.
	class ExampleRenderer
	{
	public:
		ExampleRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		void CreateDeviceDependentResources();
		void CreateWindowSizeDependentResources();
		void ReleaseDeviceDependentResources();
		void Update(DX::StepTimer const& timer);
		void Render();

	private:
		void Rotate(float radians);

	private:
		// Cached pointer to device resources.
		std::shared_ptr<DX::DeviceResources> deviceResources;

		// Resources for cube geometry.
		Microsoft::WRL::ComPtr<ID3D11InputLayout>	vertexInputLayout;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		pyramidVertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		pyramidIndexBuffer;
		Microsoft::WRL::ComPtr<ID3D11VertexShader>	vertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>	pixelShader;
		uint32	pyramidIndexCount;

		// Resources for the constant buffers of the shaders.
		Microsoft::WRL::ComPtr<ID3D11Buffer>		vertexConstantBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		pixelConstantBuffer;
		ModelViewProjectionConstantBuffer	vertexConstantBufferData;
		LightingModelConstantBuffer			pixelConstantBufferData;

		// Variables used with the rendering loop.
		bool	loadingComplete;
		float	degreesPerSecond;
	};
}

