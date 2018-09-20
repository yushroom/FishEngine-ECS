#pragma once

#if __APPLE__
#	define FISHENGINE_ROOT "/Users/yushroom/program/FishEngine-ECS/"
#else
#	define FISHENGINE_ROOT "I:\\FishEngine-ECS\\"
#endif

#include "ClassDef.hpp"

namespace FishEngine
{
	class Asset : public NonCopyable
	{
	public:
		Asset() = default;

		void AddRef();
		void ReduceRef();

	private:
		int m_RefCount = 1;
	};
}