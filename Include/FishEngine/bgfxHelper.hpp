#pragma once

#include <vector>
#include <bgfx/bgfx.h>

namespace bgfxHelper
{

	template<class T>
	inline uint32_t Sizeof(const std::vector<T>& v)
	{
		return uint32_t(sizeof(T) * v.size());
	}

	template<class T>
	inline auto MakeCopy(const std::vector<T>& v)
	{
		return bgfx::copy(v.data(), Sizeof(v));
	}

	template<class T>
	inline auto MakeRef(const std::vector<T>& v)
	{
		return bgfx::makeRef(v.data(), Sizeof(v));
	}
}