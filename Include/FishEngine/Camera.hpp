#pragma once
#include "ECS.hpp"

class Camera : public Component
{
	COMPONENT(Camera);
	
public:
	Vector3 lookAt = { 0.0f, 0.0f, 0.0f };
};
