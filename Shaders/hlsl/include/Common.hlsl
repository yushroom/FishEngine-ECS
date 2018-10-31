// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

/*=============================================================================
    PlatformCommon.usf: Common shader code
=============================================================================*/

#ifndef Common_hlsl
#define Common_hlsl

// #include "/Engine/Public/Platform.ush"
// These types are used for material translator generated code, or any functions the translated code can call
#if PIXELSHADER
    #define MaterialFloat half
    #define MaterialFloat2 half2
    #define MaterialFloat3 half3
    #define MaterialFloat4 half4
    #define MaterialFloat3x3 half3x3
    #define MaterialFloat4x4 half4x4 
    #define MaterialFloat4x3 half4x3 
#else
    // Material translated vertex shader code always uses floats, 
    // Because it's used for things like world position and UVs
    #define MaterialFloat float
    #define MaterialFloat2 float2
    #define MaterialFloat3 float3
    #define MaterialFloat4 float4
    #define MaterialFloat3x3 float3x3
    #define MaterialFloat4x4 float4x4 
    #define MaterialFloat4x3 float4x3 
#endif


MaterialFloat4 Texture2DSampleLevel(Texture2D Tex, SamplerState Sampler, float2 UV, MaterialFloat Mip)
{
    return Tex.SampleLevel(Sampler, UV, Mip);
}

const static MaterialFloat PI = 3.1415926535897932f;


float Square( float x )
{
    return x*x;
}

float2 Square( float2 x )
{
    return x*x;
}

float3 Square( float3 x )
{
    return x*x;
}

float4 Square( float4 x )
{
    return x*x;
}

float Pow2( float x )
{
    return x*x;
}

float2 Pow2( float2 x )
{
    return x*x;
}

float3 Pow2( float3 x )
{
    return x*x;
}

float4 Pow2( float4 x )
{
    return x*x;
}

float Pow3( float x )
{
    return x*x*x;
}

float2 Pow3( float2 x )
{
    return x*x*x;
}

float3 Pow3( float3 x )
{
    return x*x*x;
}

float4 Pow3( float4 x )
{
    return x*x*x;
}

float Pow4( float x )
{
    float xx = x*x;
    return xx * xx;
}

float2 Pow4( float2 x )
{
    float2 xx = x*x;
    return xx * xx;
}

float3 Pow4( float3 x )
{
    float3 xx = x*x;
    return xx * xx;
}

float4 Pow4( float4 x )
{
    float4 xx = x*x;
    return xx * xx;
}

float Pow5( float x )
{
    float xx = x*x;
    return xx * xx * x;
}

float2 Pow5( float2 x )
{
    float2 xx = x*x;
    return xx * xx * x;
}

float3 Pow5( float3 x )
{
    float3 xx = x*x;
    return xx * xx * x;
}

float4 Pow5( float4 x )
{
    float4 xx = x*x;
    return xx * xx * x;
}

float Pow6( float x )
{
    float xx = x*x;
    return xx * xx * xx;
}

float2 Pow6( float2 x )
{
    float2 xx = x*x;
    return xx * xx * xx;
}

float3 Pow6( float3 x )
{
    float3 xx = x*x;
    return xx * xx * xx;
}

float4 Pow6( float4 x )
{
    float4 xx = x*x;
    return xx * xx * xx;
}

// Clamp the base, so it's never <= 0.0f (INF/NaN).
MaterialFloat ClampedPow(MaterialFloat X,MaterialFloat Y)
{
    return pow(max(abs(X),0.000001f),Y);
}
MaterialFloat2 ClampedPow(MaterialFloat2 X,MaterialFloat2 Y)
{
    return pow(max(abs(X),MaterialFloat2(0.000001f,0.000001f)),Y);
}
MaterialFloat3 ClampedPow(MaterialFloat3 X,MaterialFloat3 Y)
{
    return pow(max(abs(X),MaterialFloat3(0.000001f,0.000001f,0.000001f)),Y);
}  
MaterialFloat4 ClampedPow(MaterialFloat4 X,MaterialFloat4 Y)
{
    return pow(max(abs(X),MaterialFloat4(0.000001f,0.000001f,0.000001f,0.000001f)),Y);
} 

/** 
 * Use this function to compute the pow() in the specular computation.
 * This allows to change the implementation depending on platform or it easily can be replaced by some approxmation.
 */
MaterialFloat PhongShadingPow(MaterialFloat X, MaterialFloat Y)
{
    // The following clamping is done to prevent NaN being the result of the specular power computation.
    // Clamping has a minor performance cost.

    // In HLSL pow(a, b) is implemented as exp2(log2(a) * b).

    // For a=0 this becomes exp2(-inf * 0) = exp2(NaN) = NaN.

    // As seen in #TTP 160394 "QA Regression: PS3: Some maps have black pixelated artifacting."
    // this can cause severe image artifacts (problem was caused by specular power of 0, lightshafts propagated this to other pixels).
    // The problem appeared on PlayStation 3 but can also happen on similar PC NVidia hardware.

    // In order to avoid platform differences and rarely occuring image atrifacts we clamp the base.

    // Note: Clamping the exponent seemed to fix the issue mentioned TTP but we decided to fix the root and accept the
    // minor performance cost.

    return ClampedPow(X, Y);
}


#endif // Common_hlsl