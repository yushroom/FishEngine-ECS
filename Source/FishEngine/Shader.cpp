#include "FishEngine/Shader.hpp"

//#include <bgfx/bgfx.h>
//#include <bgfx/platform.h>

//#include <bx/file.h>
//#include <bx/pixelformat.h>


#include <cstdio>
#define DBG printf

using namespace FishEngine;

#if 0
static const bgfx::Memory* loadMem(bx::FileReaderI* _reader, const char* _filePath)
{
	if (bx::open(_reader, _filePath) )
	{
		uint32_t size = (uint32_t)bx::getSize(_reader);
		const bgfx::Memory* mem = bgfx::alloc(size+1);
		bx::read(_reader, mem->data, size);
		bx::close(_reader);
		mem->data[mem->size-1] = '\0';
		return mem;
	}
	
	DBG("Failed to load %s.", _filePath);
	return NULL;
}

static bgfx::ShaderHandle loadShader(bx::FileReaderI* _reader, const char* _name)
{
	bgfx::ShaderHandle handle = bgfx::createShader(loadMem(_reader, _name) );
	bgfx::setName(handle, _name);
	
	return handle;
}


bgfx::ProgramHandle loadProgram(bx::FileReaderI* _reader, const char* _vsName, const char* _fsName, bgfx::ShaderHandle & vsh, bgfx::ShaderHandle & fsh)
{
	vsh = loadShader(_reader, _vsName);
	fsh = BGFX_INVALID_HANDLE;
	if (NULL != _fsName)
	{
		fsh = loadShader(_reader, _fsName);
	}
	
	return bgfx::createProgram(vsh, fsh, true /* destroy shaders when program is destroyed */);
}


static bx::FileReader s_fileReader;

Shader* ShaderUtil::Compile(const std::string& vs_path, const std::string& fs_path)
{
	Shader* s = new Shader();
	s->m_Program = loadProgram(&s_fileReader, vs_path.c_str(), fs_path.c_str(), s->m_VertexShdaer, s->m_FragmentShader);
//	
//	bgfx::UniformHandle uniforms[16];
//	int count = bgfx::getShaderUniforms(s->m_FragmentShader, uniforms, 16);
//	for (int i = 0; i < count; ++i)
//	{
//		bgfx::UniformInfo info;
//		bgfx::getUniformInfo(uniforms[i], info);
//		printf("%d: %s\n", i, info.name);
//	}
	return s;
}

#endif
//
//Shader* ShaderUtil::Compile(const std::string& vs_path, const std::string& fs_path)
//{
//	return nullptr;
//}


Shader* ShaderUtil::CompileFromShaderName(const std::string& shaderName)
{
	Shader* s = new Shader;
	std::string name = shaderName + "_VS";
	s->m_VertexShader = FishEngine::CreateShader(name.c_str());
	name = shaderName + "_PS";
	s->m_FragmentShader = FishEngine::CreateShader(name.c_str());
	return s;
}
