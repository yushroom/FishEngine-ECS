#ifdef VERTEX

$input a_position, a_texcoord0, a_normal
$output v_texcoord0, v_normal

#include <bgfx_shader.sh>

void main()
{
	gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0) );
	v_texcoord0 = a_texcoord0;
    v_normal   = mul(u_model[0], vec4(a_normal, 0.0)).xyz;
}

#else
#ifdef FRAGMENT

$input v_texcoord0, v_normal

#include <bgfx_shader.sh>

uniform vec4 lightDir;

SAMPLER2D(_MainTex, 0);

void main()
{
	gl_FragColor = texture2D(_MainTex, v_texcoord0);
    vec3 L = normalize(lightDir.xyz);
    // vec3 V = normalize(CameraPos.xyz - v_position);
    vec3 N = normalize(v_normal);
    float NdotL = dot(N, L);
    NdotL = clamp(NdotL, 0, 1);
    gl_FragColor.xyz *= NdotL;
}

#endif
#endif