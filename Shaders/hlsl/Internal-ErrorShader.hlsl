#include <ShaderVariables.hlsl>

float4 VS(float3 PosL : POSITION) : SV_POSITION
{
    return mul(float4(PosL, 1), MATRIX_MVP);
}

half4 PS() : SV_Target
{
    return half4(1, 0, 1, 1);
}