#pragma once
#include <FishEngine/ECS.hpp>
#include <FishEngine/Components/Transform.hpp>

enum class CameraType
{
	Game,
	Editor
};

class Camera : public ECS::Component
{
	COMPONENT(Camera);
public:

	// Matrix that transforms from world to camera space (i.e. view matrix).
	Matrix4x4 GetWorldToCameraMatrix() const
	{
		return GetTransform()->GetWorldToLocalMatrix();
	}

	Matrix4x4 GetCameraToWorldMatrix() const
	{
		return GetTransform()->GetLocalToWorldMatrix();
	}

	static Camera* GetMainCamera()
	{
		for (auto c : Camera::components)
		{
			if (c->m_Type == CameraType::Game)
				return c;
		}
		return nullptr;
	}

	static Camera* GetEditorCamera()
	{
		for (auto c : Camera::components)
		{
			if (c->m_Type == CameraType::Editor)
				return c;
		}
		return nullptr;
	}

public:
	float m_FOV = 60.0f;
	float m_NearClipPlane = 0.1f;
	float m_FarClipPlane = 100.f;
	CameraType m_Type = CameraType::Game;
};
