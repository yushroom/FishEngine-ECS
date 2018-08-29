#ifdef VERTEX

$input a_position, a_normal, a_uv, a_tangent
$output v_position, v_normal, v_uv, v_tangent

#include <bgfx_shader.sh>

void main()
{
	gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0) );
	//v_normal = normalize( mul(u_model[0], vec4(a_normal, 0.0)).xyz );
	v_position = mul(u_model[0], vec4(a_position, 1.0)).xyz;
	v_normal = a_normal;
	v_uv = a_uv;
	v_tangent = a_tangent;
}

#else
#ifdef FRAGMENT

$input v_position, v_normal, v_uv, v_tangent

uniform vec4 lightDir;
uniform vec4 BaseColor;
uniform vec4 PBRParams;
uniform vec4 CameraPos;

const vec3 LightColor = vec3(1, 1, 1);

#include <ShadingModels.inc>

void main()
{
	float Metallic = PBRParams.x;
	float Roughness = PBRParams.y;
	float Specular = PBRParams.z;

	vec4 outColor = vec4(0, 0, 0, 1);
	vec3 L = normalize(lightDir.xyz);
	vec3 V = -normalize(v_position - CameraPos.xyz);
	vec3 N = normalize(v_normal);

	vec3 DiffuseColor = BaseColor.rgb - BaseColor.rgb * Metallic;
	vec3 SpecularColor = mix( vec3(0.08 * Specular), BaseColor.rgb, Metallic);
	float NoL = saturate( dot(N, L) );
	float NoV = saturate( dot(N, V) );
	outColor.rgb = PI * LightColor.rgb * NoL * StandardShading(DiffuseColor, SpecularColor, vec3(Roughness), vec3(1), L, V, N);

	gl_FragColor = outColor;
}

#endif
#endif