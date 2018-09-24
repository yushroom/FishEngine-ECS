struct VertexIn
{
	float3 Position : POSITION;
	float3 Normal : NORMAL;
	float2 TexCoord : TEXCOORD0;
};

struct VSToPS
{
	float4 Position : SV_POSITION;
	float3 WorldPosition : POSITION;
	float3 WorldNormal : NORMAL;
	float2 TexCoord : TEXCOORD0;
};

// baseColorFactor number [4]  The material's base color factor.   No, default: [1,1,1,1]
// baseColorTexture    object  The base color texture. No
// metallicFactor  number  The metalness of the material.  No, default: 1
// roughnessFactor number  The roughness of the material.  No, default: 1
// metallicRoughnessTexture    object  The metallic-roughness texture. No

cbuffer PerFrame : register(b0)
{
	// float4x4 u_modelMat;
	float4x4 u_modelViewProj;
	float4x4 u_model[32];
	float4 CameraPos;
}

VSToPS VS(VertexIn vin)
{
	VSToPS vout;
	vout.Position = mul(float4(vin.Position, 1.0), u_modelViewProj);
	vout.WorldPosition = mul(float4(vin.Position, 1.0), u_model[0]).xyz;
	vout.TexCoord = vin.TexCoord;
	vout.WorldNormal = mul(float4(vin.Normal, 0.0), u_model[0]).xyz;

	return vout;
}

cbuffer PerFrame : register(b1)
{
	float4 baseColorFactor;
	float4 lightDir;
	Texture2D baseColorTexture;
	SamplerState baseColorTextureSampler;
	float4 PBRFactor;	// Metallic, Roughness, Specular
}

struct SurfaceData
{
	float3 L;
	float3 V;
	float3 N;
	// float2 UV;
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
	data.L = normalize(lightDir.xyz);
	data.V = normalize(CameraPos.xyz - pin.WorldPosition);
	data.N = normalize(pin.WorldNormal);
	// data.UV = pin.v_texcoord0;
	data.BaseColor = baseColorTexture.Sample(baseColorTextureSampler, pin.TexCoord).rgb;
	data.BaseColor *= baseColorFactor.rgb;
	data.Metallic = PBRFactor.x;
	data.Roughness = PBRFactor.y;
	data.Specular = PBRFactor.z;
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
	outColor.rgb = PI * LightColor * NoL * SimpleShading(DiffuseColor, SpecularColor, s.Roughness, L, V, N);
	return outColor;
}