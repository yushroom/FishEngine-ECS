#pragma once

#include "Object.hpp"

#include <bgfx/bgfx.h>

class Shader : public Object
{
	friend class ShaderUtil;
	
public:
	Shader() = default;
	
	bgfx::ProgramHandle& GetProgram() { return m_Program; }
	
protected:
	bgfx::ProgramHandle m_Program;
};


class ShaderUtil : public Static
{
public:
	static Shader* Compile(const char* vs_path, const char* fs_path);
};
