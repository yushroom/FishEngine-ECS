#pragma once

#include "ClassDef.hpp"

namespace FishEngine
{

class Screen : public Static
{
public:
	inline static int width = 16;
	inline static int height = 16;
	
	static float GetAspectRatio()
	{
		return (float)width / (float)height;
	}
};

class EditorScreen : public Static
{
public:
	inline static int width = 16;
	inline static int height = 16;
};

}
