#pragma once
#include "../ECS/SingletonSystem.hpp"

namespace FishEngine
{
	class SingletonTime : public SingletonComponent
	{
		SINGLETON_COMPONENT(SingletonInput);

	public:
		float deltaTime = 0;
		float fixedDeltaTime = 0.02f;
	};
}