// #define SM5_PROFILE 1
#include <Common.hlsl>
#include <BRDF.hlsl>
#include <ShadingModels.hlsl>
#include <ShaderVariables.hlsl>

struct VertexIn
{
	float3 PosL : POSITION;
	float3 Normal : NORMAL;
};

struct VertexOut
{
	float4 PosH : SV_POSITION;
	float3 PosW : POSITION;
	float3 Normal : NORMAL;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	float4 posH = float4(vin.PosL, 1.0f);
	vout.PosH = mul(posH, MATRIX_MVP);
	vout.PosW = mul(posH, MATRIX_M).xyz;
	vout.Normal = normalize(mul(vin.Normal, (float3x3)MATRIX_IT_M));

	return vout;
}

float3 DiffuseColor;
float3 SpecularColor;
float Roughness;

float4 PS(VertexOut pin) : SV_Target
{
	// float3 SimpleShading( float3 DiffuseColor, float3 SpecularColor, float Roughness, float3 L, float3 V, half3 N )
	// float3 DiffuseColor = float3(1, 0.5, 1);
	// float3 SpecularColor = float3(0.5, 1, 1);
	// float Roughness = 0.5f;
	float3 L = normalize(pin.PosW - LightPos.xyz);
	float3 V = normalize(pin.PosW - WorldSpaceCameraPos.xyz);
	float3 N = normalize(pin.Normal);
	float4 color = float4(1, 1, 1, 1);
	color.xyz = SimpleShading(DiffuseColor, SpecularColor, Roughness, L, V, N);
	return color;
}