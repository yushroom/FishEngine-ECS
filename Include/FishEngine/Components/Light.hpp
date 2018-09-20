#pragma once

#include "../ECS/Component.hpp"

namespace FishEngine
{
	class Light : public Component
	{
		COMPONENT(Light);
	public:
		//	Vector3 direction = { 0, 0, -1 };
	};
}
