#include <FishEngine/Components/Camera.hpp>
#include <FishEngine/Components/Transform.hpp>
#include <FishEngine/Screen.hpp>
#include <FishEngine/Gizmos.hpp>
#include <FishEngine/ECS/GameObject.hpp>

//#include <bx/math.h>

using namespace FishEngine;

Matrix4x4 FishEngine::Camera::GetProjectionMatrix() const
{
	float aspectRatio = Screen::GetAspectRatio();
	//float proj[16];
	if (m_Orthographic)
	{
		float y = m_OrthographicSize;
		float x = y * aspectRatio;
		//bx::mtxOrtho(proj, -x, x, -y, y, m_NearClipPlane, m_FarClipPlane, 0, bgfx::getCaps()->homogeneousDepth);
		m_ProjectionMatrix = Matrix4x4::Ortho(-x, x, -y, y, m_NearClipPlane, m_FarClipPlane);
	}
	else
	{
		//bx::mtxProj(proj, m_FieldOfView, aspectRatio, m_NearClipPlane, m_FarClipPlane, bgfx::getCaps()->homogeneousDepth);
		m_ProjectionMatrix = Matrix4x4::Perspective(m_FieldOfView, aspectRatio, m_NearClipPlane, m_FarClipPlane);
	}
	//memcpy(m_ProjectionMatrix.data(), proj, sizeof(Matrix4x4));
	//m_ProjectionMatrix = m_ProjectionMatrix.transpose();
	return m_ProjectionMatrix;
}

// Matrix that transforms from world to camera space (i.e. view matrix).

Matrix4x4 FishEngine::Camera::GetWorldToCameraMatrix() const
{
	return GetTransform()->GetWorldToLocalMatrix();
}

inline Matrix4x4 FishEngine::Camera::GetCameraToWorldMatrix() const
{
	return GetTransform()->GetLocalToWorldMatrix();
}

Matrix4x4 FishEngine::Camera::GetViewProjectionMatrix() const
{
	return GetProjectionMatrix() * GetWorldToCameraMatrix();
	//return GetWorldToCameraMatrix() * GetProjectionMatrix();
}

// Returns a ray going from camera through a screen point.

Ray FishEngine::Camera::ScreenPointToRay(const Vector3 & position)
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

Camera * FishEngine::Camera::GetMainCamera()
{
	for (auto c : Camera::components)
	{
		if (c->m_Type == CameraType::Game)
			return c;
	}
	return nullptr;
}

Camera * FishEngine::Camera::GetEditorCamera()
{
	for (auto c : Camera::components)
	{
		if (c->m_Type == CameraType::Editor)
			return c;
	}
	return nullptr;
}

void FishEngine::Camera::OnDrawGizmosSelected() const
{
	Gizmos::color = Vector4(1, 1, 1, 1);
	Gizmos::DrawFrustum(GetFrustum(), GetCameraToWorldMatrix());
}

Frustum FishEngine::Camera::GetFrustum() const
{
	return { m_FieldOfView, m_FarClipPlane, m_NearClipPlane, Screen::GetAspectRatio() };
}
