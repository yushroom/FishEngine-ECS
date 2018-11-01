#ifndef ShadingModels_hlsl
#define ShadingModels_hlsl

#include "BRDF.hlsl"

float3 StandardShading( float3 DiffuseColor, float3 SpecularColor, float Roughness, float3 L, float3 V, half3 N )
{
	float3 H = normalize(V + L);
	float NoL = saturate( dot(N, L) );
	float NoV = saturate( abs( dot(N, V) ) + 1e-5 );
	float NoH = saturate( dot(N, H) );
	float VoH = saturate( dot(V, H) );

	float D = D_GGX( Roughness, NoH );
	float Vis = Vis_SmithJointApprox( Roughness, NoV, NoL);
	float3 F = F_Schlick( SpecularColor, VoH );

	return Diffuse_Lambert( DiffuseColor ) + (D * Vis) * F;
}

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

#endif // ShadingModels_hlsl