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
    float3 normal : NORMAL;
};

VSOutput VS(VSInput i)
{
    VSOutput o;
    o.position = mul(float4(i.position, 1), MATRIX_MVP);
    // o.position = mul(MATRIX_MVP, float4(i.position, 1));
    o.normal = normalize(mul(float4(i.normal, 0), MATRIX_IT_M).xyz);
    return o;
}


float4 PS(VSOutput i) : SV_Target
{
    return float4(normalize(i.normal)*0.5+0.5, 1);
}