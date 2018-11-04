#include <ShaderVariables.hlsl>
#include <AppData.hlsl>

struct VSOutput
{
    float4 position : SV_POSITION;
    float3 color : COLOR0;
};

VSOutput VS(AppData i)
{
    VSOutput o;
    o.position = mul(float4(i.Position, 1), MATRIX_MVP);
    // o.position = mul(MATRIX_MVP, float4(i.position, 1));
    o.color = i.Color;
    return o;
}


float4 PS(VSOutput i) : SV_Target
{
    return float4(i.color, 1);
}