# 跨平台Shader编译

FishEngine使用DirecxShaderCompiler和SPIRV-Cross处理跨平台编译shader的问题。

#### [DirectX Shader Compiler](https://github.com/Microsoft/DirectXShaderCompiler)

windows10下面可以在windows sdk里面找到，如C:\Program Files (x86)\Windows Kits\10\bin\10.0.17134.0\x86\dxc.exe
但win sdk自带的dxc.exe没有SPIR-V codegen，必须从源码编译

问题1：win10+vs17编译错误，汗。。。

解决：直接下Appveyor编译好的

问题2： mac下dxc编译shader的时候有点问题，include到不存在的header文件是会卡住，CPU一直100%

直接include Unreal的shader头文件时发现还是要稍微修改一下语法


##### [SPIRV-Cross](https://github.com/KhronosGroup/SPIRV-Cross)

VS17编译有点问题，需要改个地方，-1U -> UINT_MAX

macos下直接make就好了



### 测试

测试shader如下，

```shader
cbuffer MatrixBuffer
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};

struct VertexInputType
{
    float4 position : POSITION;
    float4 color : COLOR;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

PixelInputType ColorVertexShader(VertexInputType input)
{
    PixelInputType output;
    

	// Change the position vector to be 4 units for proper matrix calculations.
    input.position.w = 1.0f;

	// Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
	// Store the input color for the pixel shader to use.
    output.color = input.color;
    
    return output;
}
```

命令行编译

```shell
./dxc -spirv -T vs_6_0 color.vs -E ColorVertexShader -Fo test.spv
./spirv-cross --version ./spirv-cross --version 310 --es test.spv
```

生成的gles shader

```c
#version 310 es
layout(binding = 0, std140) uniform type_MatrixBuffer
{
    layout(row_major) mat4 worldMatrix;
    layout(row_major) mat4 viewMatrix;
    layout(row_major) mat4 projectionMatrix;
} MatrixBuffer;

layout(location = 0) in vec4 in_var_POSITION;
layout(location = 1) in vec4 in_var_COLOR;
layout(location = 0) out vec4 out_var_COLOR;

void main()
{
    vec4 _25 = in_var_POSITION;
    _25.w = 1.0;
    gl_Position = MatrixBuffer.projectionMatrix * (MatrixBuffer.viewMatrix * (MatrixBuffer.worldMatrix * _25));
    out_var_COLOR = in_var_COLOR;
}

```

生成的metal shader

```c
#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct type_MatrixBuffer
{
    float4x4 worldMatrix;
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
};

struct ColorVertexShader_out
{
    float4 out_var_COLOR [[user(locn0)]];
    float4 gl_Position [[position]];
};

struct ColorVertexShader_in
{
    float4 in_var_POSITION [[attribute(0)]];
    float4 in_var_COLOR [[attribute(1)]];
};

vertex ColorVertexShader_out ColorVertexShader(ColorVertexShader_in in [[stage_in]], constant type_MatrixBuffer& MatrixBuffer [[buffer(0)]])
{
    ColorVertexShader_out out = {};
    float4 _25 = in.in_var_POSITION;
    _25.w = 1.0;
    out.gl_Position = ((_25 * MatrixBuffer.worldMatrix) * MatrixBuffer.viewMatrix) * MatrixBuffer.projectionMatrix;
    out.out_var_COLOR = in.in_var_COLOR;
    return out;
}
```



### 注意事项

metal shader有个限制，同一个Library中的vs/ps的入口不能同名，不然会报错：metallib: multiple symbols

[这里](https://forums.developer.apple.com/thread/45493)提到:

> The Metal shading language supports function overloading, but graphics functions (vertex and fragment) and kernel functions cannot be overloaded. If multiple vertex/fragment/kernel functions with the same name could be compiled into an MTLLibrary, it wouldn't be possible to retrieve them unambiguously.

放在不同的Library中虽然可以解决这个问题，但是Apple官方的[Best Practices Guide](https://developer.apple.com/library/archive/documentation/3DDrawing/Conceptual/MTLBestPracticesGuide/FunctionsandLibraries.html)中不提倡这种做法，最好还是

> Group Your Functions into a Single Library

还好SPIRV-Cross提供了可以修改entry point的参数

```shell
--rename-entry-point <old> <new> <stage>
```



#### cbuffer

像这样的语法在hlsl中没啥问题：

```c
cbuffer PerFrame : register(b0)
{
	float4x4 u_modelViewProj;
	float4x4 u_model[32];
	float4 CameraPos;
}

cbuffer PerFrame : register(b1)
{
	float4 baseColorFactor;
	float4 lightDir;
	float4 PBRFactor;	// Metallic, Roughness, Specular
}
```

但是转完的shader

```c
struct type_PerFrame
{
    float4x4 u_modelViewProj;
    float4x4 u_model[32];
    float4 CameraPos;
};

struct type_PerFrame_1
{
    float4 baseColorFactor;
    float4 lightDir;
    float4 PBRFactor;
};

fragment pbrMetallicRoughness_PS_out pbrMetallicRoughness_PS(pbrMetallicRoughness_PS_in in [[stage_in]], constant type_PerFrame& PerFrame [[buffer(0)]], constant type_PerFrame_1& PerFrame [[buffer(1)]], texture2d<float> baseColorTexture [[texture(2)]], sampler baseColorTextureSampler [[sampler(3)]], float4 gl_FragCoord [[position]])
{
...
}
```

函数参数名和类型名冲突了



#### 矩阵-向量乘法

注意左乘和右乘的问题

```c
mul(MATRIX_MVP, float4(PosL, 1));
```

```
mul(float4(PosL, 1), MATRIX_MVP);
```

metal和d3d里面是一样的

d3d doc mul一节提到

> x [in] The x input value. If x is a vector, it treated as a row vector.
> y [in] The y input value. If y is a vector, it treated as a column vector.

在metal[文档][Metal Shading Language Specification V2.1]里面

> Applying a single subscript to a matrix treats the matrix as an array of column vectors. Two subscripts
> select a column and then a row. The top column is column 0. A second subscript then operates on the resulting vector, as defined earlier for vectors.

> For vector – matrix, matrix – vector and matrix – matrix multiplication, the number of columns of the left operand is required to be equal to the number of rows of the right operand. The multiply operation does a linear algebraic multiply, yielding a vector or a matrix that has the same number of rows as the left operand and the same number of columns as the right operand. 

> The following vector-to-matrix multiplication 
>
> ```c#
> float3 u = v * m;
> ```
>
> is equivalent to: 
>
> ```c#
> u.x = dot(v, m[0]);
> u.y = dot(v, m[1]);
> u.z = dot(v, m[2]);
> ```
>
> The following matrix-to-vector multiplication 
>
> ```c#
> float3 u = m * v;
> ```
>
> is equivalent to: 
>
> ```c
> u = v.x * m[0];
> u += v.y * m[1];
> u += v.z * m[2];
> ```

因为FishEngine的Matrix4x4是row major的，为了不想每次传给GPU时都取转置，所以shader里面使用右乘，即矩阵在后的写法。



```c
#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct type_PerDrawUniforms
{
    float4x4 MATRIX_MVP;
    float4x4 MATRIX_MV;
    float4x4 MATRIX_M;
    float4x4 MATRIX_IT_M;
};

struct Color_VS_out
{
    float4 gl_Position [[position]];
};

struct Color_VS_in
{
    float3 in_var_POSITION [[attribute(0)]];
};

vertex Color_VS_out Color_VS(Color_VS_in in [[stage_in]], constant type_PerDrawUniforms& PerDrawUniforms [[buffer(1)]])
{
    Color_VS_out out = {};
    out.gl_Position = float4(in.in_var_POSITION, 1.0) * PerDrawUniforms.MATRIX_MVP;
    return out;
}

```

PerDrawUniforms的buffer设为0就不对了，莫非是盖了vertex buffer？



### 例子

```shell
dxc -spirv -T ps_6_0 -E PS SimpleShading.hlsl -I ./include -Fo runtime/SimpleShading_ps.spv
spirv-cross --msl runtime/SimpleShading_ps.spv --output runtime/SimpleShading_ps.metal --rename-entry-point PS SimpleShading_PS frag
```



### 总结

FishEngine中的shader使用HLSL编写，依靠SPIRV为桥梁转为glsl和metal sl。





### reference

[Metal Shading Language Specification V2.1]: https://developer.apple.com/metal/Metal-Shading-Language-Specification.pdf)