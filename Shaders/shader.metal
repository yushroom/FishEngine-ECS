//
//  shader.metal
//  Editor
//
//  Created by 俞云康 on 2018/10/27.
//

#include <metal_stdlib>
using namespace metal;

struct VSInput
{
	float3 position [[attribute(0)]];
	float2 uv 		[[attribute(1)]];
	float3 normal 	[[attribute(2)]];
	float4 tangent 	[[attribute(3)]];
};

struct VSOutput
{
	float4 position [[position]];
	float3 worldNormal;
};

struct Uniforms
{
	float4x4 modelMat;
	float4x4 mvpMat;
};

vertex VSOutput vs_main(VSInput input [[stage_in]],
						constant Uniforms& uniforms [[buffer(1)]])
{
	VSOutput out;
	out.position = uniforms.mvpMat * float4(input.position, 1);
	out.worldNormal = float3(uniforms.modelMat * float4(input.normal, 0));
	out.worldNormal = normalize(out.worldNormal);
	return out;
}

fragment float4 fs_main(VSOutput input [[stage_in]])
{
	return float4(input.worldNormal*0.5+0.5, 1);
//	return float4(1, 1, 0, 1);
}
