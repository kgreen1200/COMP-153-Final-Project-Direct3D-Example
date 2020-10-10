// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float4 position : POSITION;
	float3 color : COLOR0;
	float3 normal : NORMAL;
};

// A pass-through function for the (interpolated) color data.
float4 main(PixelShaderInput input) : SV_TARGET
{
	float3 globalAmbientLight = float3(0.4f, 0.4f, 0.4f);
	float4 lightColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
	float4 lightPosition = float4(0.0f, 0.0f, 1.0f, 1.0f);
	float constantAttenuation = 1.0f;
	float linearAttenuation = 0.0f;
	float quadraticAttenuation = 0.0f;
	float shininess = 1.0f;

	float3 scatteredLight = globalAmbientLight * input.color;
	float3 reflectedLight = float3(0.0, 0.0, 0.0);
	float3 surfaceSpecularColor = float3(1.0, 1.0, 1.0);

	float3 I = lightColor.rgb;
	float3 L;

	float3 surfaceDiffuseColor = input.color;
	float f;

	if (lightPosition.w > 0.1) {
		float4 LTemp = lightPosition - input.position;
		L = float3(LTemp.x, LTemp.y, LTemp.z);
		float d = length(L);
		L = L / d;

		float attenuationDenominator = constantAttenuation + linearAttenuation * d + quadraticAttenuation * d * d;
		if (attenuationDenominator < 0.001) {
			attenuationDenominator = 1.0f;
		}
		f = 1.0f / attenuationDenominator;
	}
	else {
		L = normalize(float3(lightPosition.x, lightPosition.y, lightPosition.z));
		f = 1.0;
	}

	float diffuseModifier = max(0.0, dot(input.normal, L));
	float specularModifier;
	if (diffuseModifier > 0.001f) {
		float3 r = normalize(reflect(-L, input.normal));
		float4 viewerPos = float4(0.0, 0.0, 1.0, 0.0);
		float4 vTemp = viewerPos - input.position;
		float3 v = normalize(float3(vTemp.x, vTemp.y, vTemp.z));
		specularModifier = pow(max(0.0, dot(r, v)), shininess);
	}
	else {
		specularModifier = 0.0;
	}

	reflectedLight = reflectedLight + f * ((I * surfaceDiffuseColor * diffuseModifier) + (I * surfaceSpecularColor * specularModifier));
	float3 sumOfLights = scatteredLight + reflectedLight;
	float3 rgb = min(sumOfLights, float3(1.0, 1.0, 1.0));
	float4 PixelColor = float4(rgb.r, rgb.g, rgb.b, 1.0f);
	
	return float4(rgb.r, rgb.g, rgb.b, 1.0f);
}
