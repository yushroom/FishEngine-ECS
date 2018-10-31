#include <ShaderVariables.hlsl>

float4 VS(float3 PosL : POSITION) : SV_POSITION
{
    return mul(float4(PosL, 1), MATRIX_MVP);
}

float4 u_color;

float4 PS() : SV_Target
{
    return u_color;
}