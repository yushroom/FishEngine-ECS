#pragma once

#include "Object.hpp"

#include <bgfx/bgfx.h>

class Shader : public Object
{
	friend class ShaderUtil;
	
public:
	Shader() = default;
	
	bgfx::ProgramHandle& GetProgram() { return m_Program; }
	
public:
	bgfx::ProgramHandle m_Program;
	bgfx::ShaderHandle m_VertexShdaer;
	bgfx::ShaderHandle m_FragmentShader;
};


class ShaderUtil : public Static
{
public:
	static Shader* Compile(const std::string& vs_path, const std::string& fs_path);
};
