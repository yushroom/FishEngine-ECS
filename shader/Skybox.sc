#ifdef VERTEX

$input a_position
$output v_dir

#include <bgfx_shader.sh>

// uniform mat4 u_mtx;

void main()
{
	gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0) );
	v_dir = normalize(a_position);
}

#else
#ifdef FRAGMENT

$input v_dir

#include <bgfx_shader.sh>
#include "shaderlib.sh"

SAMPLERCUBE(s_texCube, 0);
SAMPLERCUBE(s_texCubeIrr, 1);

uniform vec4 u_params;
#define u_exposure u_params.z
#define u_bgType u_params.w

void main()
{
	vec3 dir = normalize(v_dir);

	vec4 color;
	if (u_bgType == 7.0)
	{
		color = toLinear(textureCube(s_texCubeIrr, dir));
	}
	else
	{
		float lod = u_bgType;
		dir = fixCubeLookup(dir, lod, 256.0);
		color = toLinear(textureCubeLod(s_texCube, dir, lod));
	}
	color *= exp2(u_exposure);

	gl_FragColor = toFilmic(color);
}


#endif
#endif