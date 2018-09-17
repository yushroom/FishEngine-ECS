#pragma once

#include "ClassDef.hpp"
#include <bgfx/bgfx.h>
#include <bimg/bimg.h>

#include "Assets.hpp"
#include "Object.hpp"


namespace FishEngine
{

class TextureUtils : public Static
{
public:
	static bgfx::TextureHandle LoadTexture(const char* path, uint32_t _flags = BGFX_SAMPLER_MIN_POINT
																				| BGFX_SAMPLER_MAG_POINT
																				| BGFX_SAMPLER_MIP_POINT | BGFX_SAMPLER_U_MIRROR | BGFX_SAMPLER_V_MIRROR);
//	static bgfx::TextureHandle LoadTextureFromMemory(void* data, uint32_t size, uint32_t _flags=BGFX_TEXTURE_NONE);
};

bgfx::TextureHandle loadTexture2(void* data, uint32_t size,
								 const char* _filePath,
								 uint32_t _flags = BGFX_SAMPLER_MIN_POINT
								 | BGFX_SAMPLER_MAG_POINT
								 | BGFX_SAMPLER_MIP_POINT|BGFX_SAMPLER_U_MIRROR|BGFX_SAMPLER_V_MIRROR,
//								uint8_t _skip,
								 bgfx::TextureInfo* _info = nullptr,
								 bimg::Orientation::Enum* _orientation = nullptr
								 );

class Texture : public Object
{
public:
	
	static void StaticInit()
	{
		s_WhiteTexture = TextureUtils::LoadTexture(FISHENGINE_ROOT "Assets/Textures/white.png");
	}
	inline static bgfx::TextureHandle s_WhiteTexture;
};


}
