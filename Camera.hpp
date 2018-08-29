#pragma once
#include "ECS.hpp"

class Camera : public Component
{
	COMPONENT(Camera);
	
public:
	float at[3]  = { 0.0f, 0.0f,   0.0f };
	float eye[3] = { 0.0f, 0.0f, -15.0f };
};
