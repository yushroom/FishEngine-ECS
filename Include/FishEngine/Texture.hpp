#pragma once

#include "ClassDef.hpp"
#include <bgfx/bgfx.h>



class TextureUtils : public Static
{
public:
	static bgfx::TextureHandle LoadTexture(const char* path, uint32_t _flags);
};