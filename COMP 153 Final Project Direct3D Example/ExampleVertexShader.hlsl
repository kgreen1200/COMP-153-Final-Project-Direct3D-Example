// A constant buffer that stores the three basic column-major matrices for composing geometry.
cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
    matrix model;
    matrix view;
    matrix projection;
};

// Per-vertex data used as input to the vertex shader.
struct VertexShaderInput
{
    float3 pos : POSITION;
    float3 color : COLOR0;
    float3 normal : NORMAL0;
};

// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
    float4 pos : SV_POSITION;
    float4 position : POSITION0;
    float3 color : COLOR0;
    float3 normal : NORMAL;
};

// Simple shader to do vertex processing on the GPU.
PixelShaderInput main(VertexShaderInput input)
{
    PixelShaderInput output;
    float4 pos = float4(input.pos, 1.0f);

	// Transform the vertex position into projected space.
    pos = mul(pos, model);
    pos = mul(pos, view);
    pos = mul(pos, projection);
    output.pos = pos;
    output.position = pos;
    
    matrix modelViewMatrix = mul(view, model);
    matrix normalMatrix = transpose(modelViewMatrix);
    float4 n = mul(normalMatrix, float4(input.normal, 0.0));
    float3 normal = normalize(float3(n.x, n.y, n.z));

	// Pass the color through without modification.
    output.color = input.color;
    output.normal = normal;

    return output;
}
