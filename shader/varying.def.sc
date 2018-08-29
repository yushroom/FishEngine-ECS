vec3 a_position  : POSITION;
vec3 a_normal    : NORMAL;
vec2 a_uv : TEXCOORD0;
vec3 a_tangent : TANGENT;

struct VS_OUT
{
	vec3 position;      // in world space
	vec3 normal;        // in world space
	vec3 tangent;
	vec2 uv;
};

vec3 v_position : POSITION;
vec3 v_normal : NORMAL;
vec2 v_uv : TEXCOORD0;
vec3 v_tangent : TANGENT;