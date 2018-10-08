#ifndef _SHADER_VARIABLIES_HLSL_
#define _SHADER_VARIABLIES_HLSL_

cbuffer PerCameraUniforms : register(b0)
{
    float4x4 MATRIX_P;
    float4x4 MATRIX_V;
    float4x4 MATRIX_I_V;
    float4x4 MATRIX_VP;

    float4 WorldSpaceCameraPos;     // w = 1, not used
    float4 WorldSpaceCameraDir;     // w = 0, not used
};

float3 gLightPos;

cbuffer PerDrawUniforms : register(b1)
{
    float4x4 MATRIX_MVP;
    float4x4 MATRIX_MV;
    // float4x4 MATRIX_IT_MV;
    float4x4 MATRIX_M;
    float4x4 MATRIX_IT_M;
}

#endif