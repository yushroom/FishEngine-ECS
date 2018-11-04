#ifndef AppData_hlsl
#define AppData_hlsl

struct AppData
{
	float3 Position : POSITION;
	float2 TexCoord : TEXCOORD0;
	float3 Normal : NORMAL;
	float4 Tangent : TANGENT;
	float3 Color : COLOR0;
};


#endif // AppData_hlsl