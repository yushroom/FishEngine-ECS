https://docs.unrealengine.com/en-us/Engine/Rendering/Materials/PhysicallyBased

### Base Color

> **Base Color** defines the overall color of the Material, taking a Vector3 (RGB) value where each channel is automatically clamped between 0 and 1.

注意，非金属的BaseColor只有一个值（rbg值相同），Unreal Doc中叫做BaseColor Intensity，金属的BaseColor有rgb三个通道。

### Roughness

> The **Roughness** input controls a Material's roughness. Rough Materials scatter reflected light in more directions than smooth Materials, which is how blurry or sharp a reflection is (or in how broad or tight a specular highlight is). A roughness value of 0 (smooth) results in a mirror reflection, and a roughness value of 1 (rough) results in a diffuse (or matte) surface.

### Metallic

> The **Metallic** input controls how 'metal-like' your surface will be. Nonmetals have Metallic values of 0 and metals have Metallic values of 1. This value will either be 0 or 1 for pure surfaces, such as pure metal, stone, or plastic. When creating hybrid surfaces like corroded or dusty metals, you may find that you need some value between 0 and 1.
>
> You may be reluctant at first to make any Material completely metallic. Resist!

## Specular

> When you're editing a *non-metallic* surface material, there are times when you'll want to adjust its ability to reflect light, specifically, its **Specular** property. To update a Material's Specular, input a scalar value between 0 (non-reflective) and 1 (fully-reflective). Note that a Material's Specular default value is 0.5.
>
> For very diffuse Materials, you may be inclined to set this to zero. Resist! All Materials have specular, see this post for examples [[5\]](https://docs.unrealengine.com/en-US/Engine/Rendering/Materials/PhysicallyBased#hable). What you really want to do for very diffuse Materials is make them rough.



Shader开发的Doc见/Engine/Documentation/Source/Programming/Rendering/ShaderDevelopment/ShaderDevelopment.CHN.udn

Unreal生成的Shader是基于模板的，模板可以在Engine/Shaders/Private/MaterialTemplate.ush中找到

模板主要的输入在CalcPixelMaterialInputs函数中

```c++
void CalcPixelMaterialInputs(in out FMaterialPixelParameters Parameters, in out FPixelMaterialInputs PixelMaterialInputs)
{
	// Initial calculations (required for Normal)
%s
	// The Normal is a special case as it might have its own expressions and also be used to calculate other inputs, so perform the assignment here
%s

	// Note that here MaterialNormal can be in world space or tangent space
	float3 MaterialNormal = GetMaterialNormal(Parameters, PixelMaterialInputs);

#if MATERIAL_TANGENTSPACENORMAL
#if SIMPLE_FORWARD_SHADING
	Parameters.WorldNormal = float3(0, 0, 1);
#endif

#if FEATURE_LEVEL >= FEATURE_LEVEL_SM4
	// ES2 will rely on only the final normalize for performance
	MaterialNormal = normalize(MaterialNormal);
#endif

	// normalizing after the tangent space to world space conversion improves quality with sheared bases (UV layout to WS causes shrearing)
	// use full precision normalize to avoid overflows
	Parameters.WorldNormal = TransformTangentNormalToWorld(Parameters.TangentToWorld, MaterialNormal);

#else //MATERIAL_TANGENTSPACENORMAL

	Parameters.WorldNormal = normalize(MaterialNormal);

#endif //MATERIAL_TANGENTSPACENORMAL

#if MATERIAL_TANGENTSPACENORMAL
	// flip the normal for backfaces being rendered with a two-sided material
	Parameters.WorldNormal *= Parameters.TwoSidedSign;
#endif

	Parameters.ReflectionVector = ReflectionAboutCustomWorldNormal(Parameters, Parameters.WorldNormal, false);

#if !PARTICLE_SPRITE_FACTORY
	Parameters.Particle.MotionBlurFade = 1.0f;
#endif // !PARTICLE_SPRITE_FACTORY

	// Now the rest of the inputs
%s
}

```



VertexFactoryCommon.ush

BasePassVertexShader.usf

BasePassPixelShader.usf