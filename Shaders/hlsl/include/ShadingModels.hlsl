#pragma once

#include "BRDF.hlsl"

float3 SimpleShading( float3 DiffuseColor, float3 SpecularColor, float Roughness, float3 L, float3 V, half3 N )
{
    float3 H = normalize(V + L);
    float NoH = saturate( dot(N, H) );
    
    // Generalized microfacet specular
    float D = D_GGX( Pow4(Roughness), NoH );
    float Vis = Vis_Implicit();
    float3 F = F_None( SpecularColor );

    return Diffuse_Lambert( DiffuseColor ) + (D * Vis) * F;
}