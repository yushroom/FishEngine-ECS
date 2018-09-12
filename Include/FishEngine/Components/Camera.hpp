#pragma once
#include <FishEngine/ECS.hpp>
#include <FishEngine/Components/Transform.hpp>
#include <FishEngine/Screen.hpp>

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
	
	// Returns a ray going from camera through a screen point.
	Ray ScreenPointToRay(const Vector3& position)
	{
		//http://antongerdelan.net/opengl/raycasting.html

		// NDC space
		float x = (2.0f * position.x) / Screen::width - 1.0f;
		float y = (2.0f * position.y) / Screen::height - 1.0f;
		Vector4 ray_clip(x, y, 1.f, 1.0f);

		Vector4 ray_eye = m_ProjectionMatrix.inverse() * ray_clip;
		ray_eye.z = 1.0f; // forward
		ray_eye.w = 0.0f;

		Vector4 ray_world_h = GetTransform()->GetLocalToWorldMatrix() * ray_eye;
		Vector3 ray_world(ray_world_h.x, ray_world_h.y, ray_world_h.z);
		return Ray(GetTransform()->GetPosition(), ray_world.normalized());
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
	
	Matrix4x4 m_ProjectionMatrix;
};
