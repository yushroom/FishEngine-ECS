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
vec3 v_normal 	: NORMAL 	= vec3(0.0, 0.0, 1.0);
vec2 v_uv 		: TEXCOORD0 = vec3(0.0, 0.0, 0.0);
vec3 v_tangent 	: TANGENT;
vec3 v_dir		: TEXCOORD1 = vec3(0.0, 0.0, 0.0);
vec3 v_view 	: TEXCOORD2 = vec3(0.0, 0.0, 0.0);