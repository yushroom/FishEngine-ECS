#pragma once

#include "ClassDef.hpp"
#include <string>

namespace FishEngine
{
	class Object : public NonCopyable
	{
	public:
//		std::string name;
		std::uint32_t id;
	};
	
	class NamedObject : public Object
	{
	public:
		std::string name;
	};
}
