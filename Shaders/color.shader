#ifdef VERTEX

$input a_position

#include <bgfx_shader.sh>

void main()
{
	gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0) );
}

#else
#ifdef FRAGMENT

#include <bgfx_shader.sh>

uniform vec4 u_color;

void main()
{
	gl_FragColor = u_color;
}

#endif
#endif