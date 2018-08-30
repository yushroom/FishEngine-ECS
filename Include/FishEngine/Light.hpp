#pragma once
#include "ECS.hpp"


class Light : public Component
{
	COMPONENT(Light);
public:
	Vector3 direction = { 0, 0, -1 };
};
