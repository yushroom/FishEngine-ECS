#pragma once
#include <FishEngine/ECS.hpp>

class SingletonSelection : ECS::SingletonComponent
{
	SINGLETON_COMPONENT(SingletonSelection);
public:
	ECS::GameObject* selected = nullptr;
};