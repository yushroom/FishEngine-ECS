#pragma once

#include "ClassDef.hpp"
#include <string>

namespace FishEngine
{
	class InputArchive;
	class OutputArchive;

	class Object : public NonCopyable
	{
	public:
		std::string name;

		virtual void Deserialize(InputArchive& archive);
		virtual void Serialize(OutputArchive& archive) const;
	};
}
