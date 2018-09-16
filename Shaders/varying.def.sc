vec3 a_position : POSITION;
vec2 a_texcoord0 : TEXCOORD0;
vec3 a_normal   : NORMAL;
vec4 a_tangent  : TANGENT;
vec3 a_color0 : COLOR0;
ivec4 a_joint0 : COLOR1;
vec4 a_weight0 : COLOR2;

// struct VS_OUT
// {
// 	vec3 position;      // in world space
// 	vec3 normal;        // in world space
// 	vec3 tangent;
// 	vec2 uv;
// };

vec3 v_positionW : POSITION1;
vec3 v_normal 	: NORMAL 	= vec3(0.0, 0.0, 1.0);
vec2 v_texcoord0 : TEXCOORD0 = vec2(0.0, 0.0);
vec4 v_tangent 	: TANGENT;
vec3 v_dir		: TEXCOORD1 = vec3(0.0, 0.0, 0.0);
vec3 v_view 	: TEXCOORD2 = vec3(0.0, 0.0, 0.0);
vec3 v_color0 : COLOR0;