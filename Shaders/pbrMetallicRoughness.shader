#ifdef VERTEX

$input a_position, a_texcoord0, a_normal
$output v_texcoord0, v_normal

// baseColorFactor number [4]  The material's base color factor.   No, default: [1,1,1,1]
// baseColorTexture    object  The base color texture. No
// metallicFactor  number  The metalness of the material.  No, default: 1
// roughnessFactor number  The roughness of the material.  No, default: 1
// metallicRoughnessTexture    object  The metallic-roughness texture. No

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

uniform vec4 baseColorFactor;
SAMPLER2D(baseColorTexture, 0);
uniform vec4 lightDir;

const float ambient = 0.2f;

void main()
{
	gl_FragColor = texture2D(baseColorTexture, v_texcoord0);
    gl_FragColor *= baseColorFactor;
    vec3 L = normalize(lightDir.xyz);
    // vec3 V = normalize(CameraPos.xyz - v_position);
    vec3 N = normalize(v_normal);
    float NdotL = dot(N, L);
    NdotL = saturate(NdotL);
    gl_FragColor.xyz *= saturate(NdotL + ambient);
}

#endif
#endif