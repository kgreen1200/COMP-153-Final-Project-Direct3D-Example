cbuffer LightingModelConstantBuffer : register(b1)
{
    matrix viewMatrix;
    float3 globalAmbientLight;
    float4 light_color;
    float4 light_position;
    
    float constantAttenuation;
    float linearAttenuation;
    float quadraticAttenuation;
}

// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
    float4 pos : SV_POSITION;
    float4 position : POSITION0;
    float3 color : COLOR0;
    float3 normal : NORMAL;
};

// A pass-through function for the (interpolated) color data.
float4 main(PixelShaderInput input) : SV_TARGET
{
	return float4(input.color, 1.0f);
}
