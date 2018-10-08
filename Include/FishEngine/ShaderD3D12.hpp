#pragma once

#include "Object.hpp"

//#include <bgfx/bgfx.h>
#include <wrl.h>
#include <d3dcompiler.h>
using namespace Microsoft::WRL;

#if defined(min)
#undef min
#endif

#if defined(max)
#undef max
#endif

namespace FishEngine
{
	class ShaderD3D
	{
		friend class ShaderUtil;
	
	public:
		ShaderD3D() = default;
	
		//bgfx::ProgramHandle& GetProgram() { return m_Program; }
	
	public:
		//bgfx::ProgramHandle m_Program;
		//bgfx::ShaderHandle m_VertexShdaer;
		//bgfx::ShaderHandle m_FragmentShader;

		ComPtr<ID3DBlob> m_VertexShaderBlob;
		ComPtr<ID3DBlob> m_PixelShaderBlob;
	};
}
