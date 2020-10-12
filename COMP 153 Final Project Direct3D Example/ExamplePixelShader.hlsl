cbuffer LightingModelConstantBuffer : register(b0)
{
    matrix viewMatrix;
    float3 globalAmbientLight;
    float4 light_color;
    float4 light_position;
    
    float constantAttenuation;
    float linearAttenuation;
    float quadraticAttenuation;
}

struct PixelShaderInput
{
    float4 pos : SV_POSITION; // needed for declaration but useless for us
    float4 position : POSITION0;
    float3 color : COLOR0;
    float3 normal : NORMAL;
};

float4 main(PixelShaderInput input) : SV_TARGET
{
    float4 pixel_color;
    
    //float3 globalAmbientLight = float3(1.0f, 0.0f, 0.0f);
    
    float shininess = 1.0f;
    
    float3 scatteredLight = globalAmbientLight * input.color.rgb;
    float3 reflectedLight = float3(0.0f, 0.0f, 0.0f);
    float3 surfaceSpecularColor = float3(1.0f, 1.0f, 1.0f);
    
    float3 I = light_color.rgb;
    float3 L;
    float4 lightPos_eyeCoords = mul(viewMatrix, light_position);
    
    float3 surfaceDiffuseColor = input.color.rgb;
    float f;
    
    if (light_position.w > 0.f)
    {
        float4 LTemp = lightPos_eyeCoords - input.position;
        L = float3(LTemp.x, LTemp.y, LTemp.z);
        float d = length(L);
        L = L / d;
        
        float attenuationDenominator = constantAttenuation + linearAttenuation * d + quadraticAttenuation * d * d;
        if (attenuationDenominator < 0.001f)
        {
            attenuationDenominator = 1.0f;
        }
        f = 1.0f / attenuationDenominator;
    }
    else
    {
        L = normalize(float3(lightPos_eyeCoords.x, lightPos_eyeCoords.y, lightPos_eyeCoords.z));
        f = 1.0f;
    }
    
    float diffuseModifier = max(0.0f, dot(normalize(input.normal), L));
    float specularModifier;
    if (diffuseModifier > 0.0001f)
    {
        float3 r = normalize(reflect(-L, input.normal));
        float4 viewerPos = float4(0.0f, 0.7f, 1.5f, 0.0f);
        float4 vTemp = viewerPos - input.position;
        float3 v = normalize(float3(vTemp.x, vTemp.y, vTemp.z));
        specularModifier = pow(max(0.0f, dot(r, v)), shininess);

    }
    else
    {
        specularModifier = 0.0f;
    }
    
    reflectedLight += f * ((I * surfaceDiffuseColor * diffuseModifier) + (I * surfaceSpecularColor * specularModifier));
    
    float3 sumOfLights = scatteredLight + reflectedLight;
    float3 rgb = min(sumOfLights, float3(1.0f, 1.0f, 1.0f));
    pixel_color = float4(rgb.r, rgb.g, rgb.b, 1.0f);
    
    //return float4(globalAmbientLight.r, globalAmbientLight.g, globalAmbientLight.b, 1.0f);
    return pixel_color;

}