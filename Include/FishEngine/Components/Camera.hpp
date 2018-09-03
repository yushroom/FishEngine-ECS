#pragma once
#include <FishEngine/ECS.hpp>
#include <FishEngine/Components/Transform.hpp>

class Camera : public ECS::Component
{
	COMPONENT(Camera);
public:

	// Matrix that transforms from world to camera space (i.e. view matrix).
	Matrix4x4 GetWorldToCameraMatrix() const
	{
		return GetTransform()->GetWorldToLocalMatrix();
	}

public:
	float m_NearClipPlane = 0.1f;
	float m_FarClipPlane = 100.f;
};
