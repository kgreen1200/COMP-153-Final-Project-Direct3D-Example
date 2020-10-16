#include "pch.h"
#include "ExampleRenderer.h"

#include "..\Common\DirectXHelper.h"

using namespace COMP_153_Final_Project_Direct3D_Example;

using namespace DirectX;
using namespace Windows::Foundation;

// Loads vertex and pixel shaders from files and instantiates the pyramid geometry.
ExampleRenderer::ExampleRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources) :
	loadingComplete(false),					// becomes true when the model is ready to be rendered
	degreesPerSecond(45),					// number of degrees the pyramid should rotate per second
	pyramidIndexCount(0),					// holds the number of indices listed in the pyramidIndices object
	deviceResources(deviceResources)		// resources where we can attach shaders, create buffers, get application info, etc.
{
	CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();
}

// Initializes view parameters when the window size changes.
void ExampleRenderer::CreateWindowSizeDependentResources()
{
	Size outputSize = deviceResources->GetOutputSize();
	float aspectRatio = outputSize.Width / outputSize.Height;
	float fovAngleY = 70.0f * XM_PI / 180.0f;

	// This is a simple example of change that can be made when the app is in
	// portrait or snapped view.
	if (aspectRatio < 1.0f)
	{
		fovAngleY *= 2.0f;
	}

	// Note that the OrientationTransform3D matrix is post-multiplied here
	// in order to correctly orient the scene to match the display orientation.
	// This post-multiplication step is required for any draw calls that are
	// made to the swap chain render target. For draw calls to other targets,
	// this transform should not be applied.

	// This sample makes use of a right-handed coordinate system using row-major matrices.
	XMMATRIX perspectiveMatrix = XMMatrixPerspectiveFovRH(fovAngleY, aspectRatio, 0.01f, 100.0f);

	XMFLOAT4X4 orientation = deviceResources->GetOrientationTransform3D();

	XMMATRIX orientationMatrix = XMLoadFloat4x4(&orientation);

	XMStoreFloat4x4(
		&vertexConstantBufferData.projection,
		XMMatrixTranspose(perspectiveMatrix * orientationMatrix)
		);

	// float values for the location of the camera, storing this way makes it easier
	// to store the values to update the viewer pos in the pixel shader
	float eyeX = 0.0f;
	float eyeY = 0.7f;
	float eyeZ = 1.5f;
	float eyeW = 0.0f;

	// Eye is at (0,0.7,1.5), looking at point (0,-0.1,0) with the up-vector along the y-axis.
	static const XMVECTORF32 eye = { eyeX, eyeY, eyeZ, eyeW };
	static const XMVECTORF32 at = { 0.0f, -0.1f, 0.0f, 0.0f };
	static const XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };

	XMStoreFloat4x4(&vertexConstantBufferData.view, XMMatrixTranspose(XMMatrixLookAtRH(eye, at, up)));

	// Storing data for pixel buffer since we now have the view matrix
	XMStoreFloat4x4(&pixelConstantBufferData.viewMatrix, XMMatrixTranspose(XMMatrixLookAtRH(eye, at, up)));
	pixelConstantBufferData.globalAmbientLight = XMFLOAT3(0.1f, 0.1f, 0.1f);
	pixelConstantBufferData.light_color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	pixelConstantBufferData.light_position = XMFLOAT4(0.0f, 0.7f, 0.0f, 1.0f);
	pixelConstantBufferData.viewerPos = XMFLOAT4(eyeX, eyeY, eyeZ, eyeW);
	pixelConstantBufferData.constantAttenuation = 0.0f;
	pixelConstantBufferData.linearAttenuation = 0.0f;
	pixelConstantBufferData.quadraticAttenuation = 1.0f;
}

// Called once per frame, rotates the pyramid and calculates the model and view matrices.
void ExampleRenderer::Update(DX::StepTimer const& timer)
{
	// Convert degrees to radians, then convert seconds to rotation angle
	float radiansPerSecond = XMConvertToRadians(degreesPerSecond);
	double totalRotation = timer.GetTotalSeconds() * radiansPerSecond;
	float radians = static_cast<float>(fmod(totalRotation, XM_2PI));

	Rotate(radians);
}

// Rotate the 3D pyramid model a set amount of radians.
void ExampleRenderer::Rotate(float radians)
{
	// Prepare to pass the updated model matrix to the shader
	XMStoreFloat4x4(&vertexConstantBufferData.model, XMMatrixTranspose(XMMatrixRotationY(radians)));
}

// Renders one frame using the vertex and pixel shaders.
void ExampleRenderer::Render()
{
	// Loading is asynchronous. Only draw geometry after it's loaded.
	if (!loadingComplete)
	{
		return;
	}

	auto context = deviceResources->GetD3DDeviceContext();

	// Each vertex is one instance of the VertexPositionColor struct.
	UINT stride = sizeof(VertexPositionColor);
	UINT offset = 0;

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	context->IASetInputLayout(vertexInputLayout.Get());

	// Prepare the constant buffer to send it to the graphics device.
	context->UpdateSubresource1(
		vertexConstantBuffer.Get(),
		0,
		NULL,
		&vertexConstantBufferData,
		0,
		0,
		0
	);

	// Send the constant buffer to the graphics device.
	context->VSSetConstantBuffers1(
		0,
		1,
		vertexConstantBuffer.GetAddressOf(),
		nullptr,
		nullptr
	);

	// ps constant buffer
	context->UpdateSubresource1(pixelConstantBuffer.Get(), 0, NULL, &pixelConstantBufferData, 0, 0, 0);
	context->PSSetConstantBuffers1(0, 1, pixelConstantBuffer.GetAddressOf(), nullptr, nullptr);

	// Attach our vertex shader.
	context->VSSetShader(vertexShader.Get(), nullptr, 0);

	// Attach our pixel shader.
	context->PSSetShader(pixelShader.Get(), nullptr, 0);

	// Setting vertex buffers and index buffers for the pyramid
	context->IASetVertexBuffers(0, 1, pyramidVertexBuffer.GetAddressOf(), &stride, &offset);

	context->IASetIndexBuffer(
		pyramidIndexBuffer.Get(),
		DXGI_FORMAT_R16_UINT, // Each index is one 16-bit unsigned integer (short).
		0
	);

	// Draw the objects.
	context->DrawIndexed(pyramidIndexCount, 0, 0);
}

void ExampleRenderer::CreateDeviceDependentResources()
{
	// Load shaders asynchronously.
	auto loadVSTask = DX::ReadDataAsync(L"ExampleVertexShader.cso");
	auto loadPSTask = DX::ReadDataAsync(L"ExamplePixelShader.cso");

	// After the vertex shader file is loaded, create the shader and input layout.
	auto createVSTask = loadVSTask.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			deviceResources->GetD3DDevice()->CreateVertexShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&vertexShader
				)
			);

		// Description that lays out how the vertex buffer is laid out. In this case its {Position, Color, Normal}.
		static const D3D11_INPUT_ELEMENT_DESC vertexDesc [] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		// sets input layout to that which is listed above. notice how it matches the Vertex Shader input in ExampleVertexShader.hlsl
		DX::ThrowIfFailed(
			deviceResources->GetD3DDevice()->CreateInputLayout(vertexDesc, ARRAYSIZE(vertexDesc), &fileData[0], fileData.size(), &vertexInputLayout)
		);
	});

	// After the pixel shader file is loaded, create the shader and constant buffer.
	auto createPSTask = loadPSTask.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			deviceResources->GetD3DDevice()->CreatePixelShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&pixelShader
				)
			);

		CD3D11_BUFFER_DESC constantBufferDesc(sizeof(ModelViewProjectionConstantBuffer) , D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(
			deviceResources->GetD3DDevice()->CreateBuffer(
				&constantBufferDesc,
				nullptr,
				&vertexConstantBuffer
				)
			);

		// the lighting model constant buffer is not divisible by 16 in its size, so we tack on 8 bytes to make it fit.
		CD3D11_BUFFER_DESC lightingConstantBufferDesc(sizeof(LightingModelConstantBuffer) + 8, D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(
			deviceResources->GetD3DDevice()->CreateBuffer(
				&lightingConstantBufferDesc,
				nullptr,
				&pixelConstantBuffer
			)
		);
	});

	// Once both shaders are loaded, create the mesh.
	auto createPyramidTask = (createPSTask && createVSTask).then([this] () { // basically says once the shaders are loaded in, then do this chunk of code

		// Load mesh vertices. Each vertex has a position and a color.
		static const VertexPositionColor pyramidVertices[] = 
		{
			{XMFLOAT3(-0.5f,  0.0f, -0.5f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT3(-1.0f,  1.0f, -1.0f)}, // 0
			{XMFLOAT3( 0.5f,  0.0f, -0.5f), XMFLOAT3(1.0f, 0.5f, 0.0f), XMFLOAT3( 1.0f,  1.0f, -1.0f)}, // 1
			{XMFLOAT3( 0.5f,  0.0f,  0.5f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT3( 1.0f,  1.0f,  1.0f)}, // 2
			{XMFLOAT3(-0.5f,  0.0f,  0.5f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT3(-1.0f,  1.0f,  1.0f)}, // 3
			{XMFLOAT3( 0.0f,  0.5f,  0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3( 0.0f,  1.0f,  0.0f)}, // 4
		};

		D3D11_SUBRESOURCE_DATA vertexBufferData = {0};
		vertexBufferData.pSysMem = pyramidVertices;
		vertexBufferData.SysMemPitch = 0;
		vertexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(pyramidVertices), D3D11_BIND_VERTEX_BUFFER);
		DX::ThrowIfFailed(
			deviceResources->GetD3DDevice()->CreateBuffer(
				&vertexBufferDesc,
				&vertexBufferData,
				&pyramidVertexBuffer
				)
			);

		// Load mesh indices. Each trio of indices represents
		// a triangle to be rendered on the screen.
		// For example: 0,2,1 means that the vertices with indexes
		// 0, 2 and 1 from the vertex buffer compose the 
		// first triangle of this mesh.
		static const unsigned short pyramidIndices [] =
		{
			4, 0, 1,
			4, 1, 2,
			4, 2, 3,
			4, 3, 0,

			3, 0, 1,
			2, 3, 1
		};

		pyramidIndexCount = ARRAYSIZE(pyramidIndices);

		D3D11_SUBRESOURCE_DATA indexBufferData = {0};
		indexBufferData.pSysMem = pyramidIndices;
		indexBufferData.SysMemPitch = 0;
		indexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC indexBufferDesc(sizeof(pyramidIndices), D3D11_BIND_INDEX_BUFFER);
		DX::ThrowIfFailed(
			deviceResources->GetD3DDevice()->CreateBuffer(
				&indexBufferDesc,
				&indexBufferData,
				&pyramidIndexBuffer
				)
			);
	});

	// Once the pyramid is loaded, the object is ready to be rendered.
	createPyramidTask.then([this] () {
		loadingComplete = true;
	});
}

// Resets and frees up memory objects once the renderer is not longer in use.
void ExampleRenderer::ReleaseDeviceDependentResources()
{
	loadingComplete = false;
	vertexShader.Reset();
	vertexInputLayout.Reset();
	pixelShader.Reset();
	vertexConstantBuffer.Reset();
	pixelConstantBuffer.Reset();
	pyramidVertexBuffer.Reset();
	pyramidIndexBuffer.Reset();
}