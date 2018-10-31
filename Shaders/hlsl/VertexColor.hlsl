#include <ShaderVariables.hlsl>

struct VSInput
{
    float3 position : POSITION;
    float2 uv : TEXXOORD0;
    float3 normal : NORMAL;
    float4 tangent : TANGENT;
    float3 color : COLOR0;
};

struct VSOutput
{
    float4 position : SV_POSITION;
    float3 color : COLOR0;
};

VSOutput VS(VSInput i)
{
    VSOutput o;
    o.position = mul(float4(i.position, 1), MATRIX_MVP);
    // o.position = mul(MATRIX_MVP, float4(i.position, 1));
    o.color = i.color;
    return o;
}


float4 PS(VSOutput i) : SV_Target
{
    return float4(i.color, 1);
}