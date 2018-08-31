#include "FishEngine/Shader.hpp"

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>

#include <bx/file.h>
#include <bx/pixelformat.h>


#include <cstdio>
#define DBG printf

void* load(bx::FileReaderI* _reader, bx::AllocatorI* _allocator, const char* _filePath, uint32_t* _size)
{
	if (bx::open(_reader, _filePath) )
	{
		uint32_t size = (uint32_t)bx::getSize(_reader);
		void* data = BX_ALLOC(_allocator, size);
		bx::read(_reader, data, size);
		bx::close(_reader);
		if (NULL != _size)
		{
			*_size = size;
		}
		return data;
	}
	else
	{
		DBG("Failed to open: %s.", _filePath);
	}
	
	if (NULL != _size)
	{
		*_size = 0;
	}
	
	return NULL;
}


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


bgfx::ProgramHandle loadProgram(bx::FileReaderI* _reader, const char* _vsName, const char* _fsName)
{
	bgfx::ShaderHandle vsh = loadShader(_reader, _vsName);
	bgfx::ShaderHandle fsh = BGFX_INVALID_HANDLE;
	if (NULL != _fsName)
	{
		fsh = loadShader(_reader, _fsName);
	}
	
	return bgfx::createProgram(vsh, fsh, true /* destroy shaders when program is destroyed */);
}


static bx::FileReader s_fileReader;

Shader* ShaderUtil::Compile(const char* vs_path, const char* fs_path)
{
	Shader* s = new Shader();
	s->m_Program = loadProgram(&s_fileReader, vs_path, fs_path);
	return s;
}

