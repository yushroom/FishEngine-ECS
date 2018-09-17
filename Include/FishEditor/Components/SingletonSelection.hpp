#pragma once
#include <FishEngine/ECS/SingletonSystem.hpp>

namespace FishEditor
{

	class SingletonSelection : FishEngine::SingletonComponent
	{
		SINGLETON_COMPONENT(SingletonSelection);
	public:
		FishEngine::GameObject* selected = nullptr;
	};

}
