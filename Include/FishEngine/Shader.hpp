#pragma once

#include "Object.hpp"

//#include <bgfx/bgfx.h>
//#include <d3d12.h>
//#include <wrl.h>

namespace FishEngine
{

	class ShaderImpl;

	class Shader : public Object
	{
		friend class ShaderUtil;
	
	public:
		Shader() = default;
	
		//bgfx::ProgramHandle& GetProgram() { return m_Program; }
	
	public:
		//bgfx::ProgramHandle m_Program;
		//bgfx::ShaderHandle m_VertexShdaer;
		//bgfx::ShaderHandle m_FragmentShader;

		//Microsoft::WRL::ComPtr<ID3DBlob> m_VertexShaderBlob;
		//Microsoft::WRL::ComPtr<ID3DBlob> m_PixelShaderBlob;
		ShaderImpl* m_Impl = nullptr;
	};


	class ShaderUtil : public Static
	{
	public:
		static Shader* Compile(const std::string& vs_path, const std::string& fs_path);
	};

}
