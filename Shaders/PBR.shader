#ifdef VERTEX

$input a_position, a_normal, a_uv, a_tangent
$output v_position, v_normal, v_uv, v_tangent

#include <bgfx_shader.sh>

void main()
{
	gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0) );
	//v_normal = normalize( mul(u_model[0], vec4(a_normal, 0.0)).xyz );
	v_position = mul(u_model[0], vec4(a_position, 1.0)).xyz;
	v_normal   = mul(u_model[0], vec4(a_normal, 0.0)).xyz;
	v_tangent  = mul(u_model[0], vec4(a_tangent, 0.0)).xyz;
	v_uv = a_uv;
}

#else
#ifdef FRAGMENT

$input v_position, v_normal, v_uv, v_tangent

#define _AMBIENT_IBL 1

uniform vec4 lightDir;
uniform vec4 BaseColor;
uniform vec4 PBRParams;
uniform vec4 CameraPos;

const vec3 LightColor = vec3(1, 1, 1);

#include <ShadingModels.inc>
#include <Ambient.inc>

void main()
{
	float Metallic = PBRParams.x;
	float Roughness = PBRParams.y;
	float Specular = PBRParams.z;

	vec4 outColor = vec4(0, 0, 0, 1);
	vec3 L = normalize(lightDir.xyz);
	vec3 V = normalize(CameraPos.xyz - v_position);
	vec3 N = normalize(v_normal);

	vec3 DiffuseColor = BaseColor.rgb - BaseColor.rgb * Metallic;
	vec3 SpecularColor = mix( vec3(0.08 * Specular), BaseColor.rgb, Metallic);
	float NoL = saturate( dot(N, L) );
	float NoV = saturate( dot(N, V) );
	outColor.rgb = PI * LightColor.rgb * NoL * StandardShading(DiffuseColor, SpecularColor, vec3(Roughness), vec3(1), L, V, N);
	//outColor.rgb = vec3(NoL);
	//outColor.rgb = N * 0.5 + 0.5;

#ifdef _AMBIENT_IBL
	float3 R0 = 2 * dot( V, N ) * N - V;
	float a = Square( Roughness );
	float3 R = lerp( N, R0, (1 - a) * ( sqrt(1 - a) + a ) );

	float3 NonSpecularContribution = vec3(0);
	float3 SpecularContribution = vec3(0);

	float AbsoluteDiffuseMip = AmbientCubemapMipAdjust.z;
	//float3 DiffuseLookup =  TextureCubeSampleLevel(AmbientCubemap, AmbientCubemapSampler, N, AbsoluteDiffuseMip).rgb;
	float3 DiffuseLookup = textureLod(AmbientCubemap, N, AbsoluteDiffuseMip).rgb;
	NonSpecularContribution += DiffuseColor * DiffuseLookup;

	float Mip = ComputeCubemapMipFromRoughness(Roughness, AmbientCubemapMipAdjust.w);
	float3 SampleColor = textureLod(AmbientCubemap, R, Mip).rgb;
	SpecularContribution += SampleColor * EnvBRDFApprox(SpecularColor, Roughness, NoV);
	//SpecularContribution += SampleColor * EnvBRDF(SpecularColor, Roughness, NoV);

	outColor.rgb += SpecularContribution + NonSpecularContribution;
	//outColor.rgb += SpecularContribution;
	//outColor.rgb += NonSpecularContribution;
#endif

	gl_FragColor = outColor;
}

#endif
#endif