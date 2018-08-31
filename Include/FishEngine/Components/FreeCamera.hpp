#pragma once

#include <FishEngine/ECS.hpp>

class FreeCamera : public ECS::Component
{
	COMPONENT(FreeCamera);
public:
	bool m_LookAtMode = false;
	float m_RotateSpeed = 200;
	float m_DragSpeed = 20;
	Vector3 m_OrbitCenter = Vector3::zero;
};