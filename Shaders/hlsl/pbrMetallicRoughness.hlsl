#include <AppData.hlsl>
#include <ShaderVariables.hlsl>

struct VSToPS
{
	float4 Position : SV_POSITION;
	float3 WorldPosition : POSITION;
	float3 WorldNormal : NORMAL;
	float2 TexCoord : TEXCOORD0;
};

#if 0
// baseColorFactor number [4]  The material's base color factor.   No, default: [1,1,1,1]
// baseColorTexture    object  The base color texture. No
// metallicFactor  number  The metalness of the material.  No, default: 1
// roughnessFactor number  The roughness of the material.  No, default: 1
// metallicRoughnessTexture    object  The metallic-roughness texture. No
#endif

VSToPS VS(AppData vin)
{
	VSToPS vout;
	vout.Position = mul(float4(vin.Position, 1.0), MATRIX_MVP);
	vout.WorldPosition = mul(float4(vin.Position, 1.0), MATRIX_M).xyz;
	vout.TexCoord = vin.TexCoord;
	vout.WorldNormal = normalize(mul(float4(vin.Normal, 0.0), MATRIX_IT_M).xyz);

	return vout;
}

float4 baseColorFactor;
// float4 PBRFactor;	// Metallic, Roughness, Specular
float Metallic;
float Roughness;
float Specular;
Texture2D baseColorTexture;
SamplerState baseColorTextureSampler;
// SamplerState baseColorTextureSampler : register(s0);
// SamplerState baseColorTextureSampler
// {
// 	Filter = MIN_MAG_LINEAR;
// 	AddressU = Wrap;
// 	AddressV = Wrap;
// };

Texture2D metallicRoughnessTexture;
SamplerState metallicRoughnessSampler;

Texture2D emissiveTexture;
SamplerState emissiveSampler;

Texture2D normalTexture;
SamplerState normalSampler;

Texture2D occlusionTexture;
SamplerState occlusionSampler;

struct SurfaceData
{
	float3 L;
	float3 V;
	float3 N;
	float2 UV;
	// float Depth;

	float3 BaseColor;
	float Metallic;
	float Roughness;
	float Specular;
};


const static float ambient = 0.2f;

float4 SurfacePS(SurfaceData surfaceData);

float4 PS(VSToPS pin) : SV_Target
{
	float4 gl_FragColor;
	// gl_FragColor = texture2D(baseColorTexture, v_texcoord0);
	// gl_FragColor = baseColorTexture.Sample(baseColorTextureSampler, pin.v_texcoord0);
	// gl_FragColor *= baseColorFactor;

	SurfaceData data;
	data.UV = pin.TexCoord;
	data.L = -normalize(LightDir.xyz);
	data.V = normalize(WorldSpaceCameraPos.xyz - pin.WorldPosition);
	data.N = normalize(pin.WorldNormal);
	// data.UV = pin.v_texcoord0;
#if 0
	data.BaseColor = baseColorTexture.Sample(baseColorTextureSampler, pin.TexCoord).rgb;
	data.BaseColor *= baseColorFactor.rgb;
#else
	data.BaseColor = baseColorFactor.rgb;
#endif
#if 0
	float4 c = metallicRoughnessTexture.Sample(metallicRoughnessSampler, pin.TexCoord);
	data.Metallic = c.b;
	data.Roughness = c.g;
#else
	data.Metallic = Metallic;
	data.Roughness = Roughness;
#endif
	data.Specular = Specular;
	return SurfacePS(data);
}

#include <Common.hlsl>
#include <BRDF.hlsl>
#include <ShadingModels.hlsl>

float4 SurfacePS(SurfaceData s)
{
	float4 outColor = {0, 0, 0, 1};
	float3 L = s.L;
	float3 V = s.V;
	float3 N = s.N;
	float3 R0 = 2 * dot(V, N) * N - V;
	float3 LightColor = float3(1, 1, 1);

	float3 DiffuseColor = s.BaseColor - s.BaseColor * s.Metallic;
	float3 SpecularColor = lerp( float3(0.08, 0.08, 0.08)*s.Specular, s.BaseColor, s.Metallic);
	float NoL = saturate( dot(N, L) );
	float NoV = saturate( dot(N, V) );
	outColor.rgb = PI * LightColor * NoL * StandardShading(DiffuseColor, SpecularColor, s.Roughness, L, V, N);

#if 0
	outColor.rgb *= occlusionTexture.Sample(occlusionSampler, s.UV).r;	// ao
	outColor.rgb += emissiveTexture.Sample(emissiveSampler, s.UV).rgb;
#endif
	return outColor;
}