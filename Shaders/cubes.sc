#ifdef VERTEX

$input a_position, a_normal
$output v_normal

#include <bgfx_shader.sh>

void main()
{
	gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0) );
	//v_normal = normalize( mul(u_model[0], vec4(a_normal, 0.0)).xyz );
	v_normal = normalize(a_normal);
}

#else
#ifdef FRAGMENT

$input v_normal

uniform vec4 lightDir;

void main()
{
	float NdotL = -dot(normalize(lightDir.xyz), normalize(v_normal));
	NdotL = clamp(NdotL, 0, 1);
	gl_FragColor = vec4(NdotL);
	gl_FragColor.a = 1.0;
}

#endif
#endif