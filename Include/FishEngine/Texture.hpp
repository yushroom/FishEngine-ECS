#pragma once

#include "ClassDef.hpp"
#include <bgfx/bgfx.h>
#include <bimg/bimg.h>


class TextureUtils : public Static
{
public:
	static bgfx::TextureHandle LoadTexture(const char* path, uint32_t _flags);
//	static bgfx::TextureHandle LoadTextureFromMemory(void* data, uint32_t size, uint32_t _flags=BGFX_TEXTURE_NONE);
};

bgfx::TextureHandle loadTexture2(void* data, uint32_t size,
								 const char* _filePath,
								 uint32_t _flags,
								 //								uint8_t _skip,
								 bgfx::TextureInfo* _info,
								 bimg::Orientation::Enum* _orientation
								 );
