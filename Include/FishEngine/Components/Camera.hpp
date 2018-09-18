#pragma once

#include "../ECS/Component.hpp"
#include <FishEngine/Components/Transform.hpp>
#include <FishEngine/Screen.hpp>
#include <FishEngine/Gizmos.hpp>

#include <bx/math.h>

namespace FishEngine
{

	enum class CameraType
	{
		Game,
		Editor
	};

	class Camera : public Component
	{
		COMPONENT(Camera);
	public:

		Matrix4x4 GetProjectionMatrix() const
		{
			float aspectRatio = Screen::GetAspectRatio();
			float proj[16];
			if (m_Orthographic)
			{
				float y = m_OrthographicSize;
				float x = y * aspectRatio;
				bx::mtxOrtho(proj, -x, x, -y, y, m_NearClipPlane, m_FarClipPlane, 0, bgfx::getCaps()->homogeneousDepth);

			}
			else
			{
				bx::mtxProj(proj, m_FieldOfView, aspectRatio, m_NearClipPlane, m_FarClipPlane, bgfx::getCaps()->homogeneousDepth);
			}
			memcpy(m_ProjectionMatrix.data(), proj, sizeof(Matrix4x4));
			m_ProjectionMatrix = m_ProjectionMatrix.transpose();
			return m_ProjectionMatrix;
		}
		
		// Matrix that transforms from world to camera space (i.e. view matrix).
		Matrix4x4 GetWorldToCameraMatrix() const
		{
			return GetTransform()->GetWorldToLocalMatrix();
		}

		Matrix4x4 GetCameraToWorldMatrix() const
		{
			return GetTransform()->GetLocalToWorldMatrix();
		}
		
		Matrix4x4 GetViewProjectionMatrix() const
		{
			return GetProjectionMatrix() * GetWorldToCameraMatrix();
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
		
		void OnDrawGizmosSelected() const override
		{
			Gizmos::color = Vector4(1, 1, 1, 1);
			Gizmos::DrawFrustum(GetFrustum(), GetCameraToWorldMatrix());
		}
		
		float GetFarClipPlane() const { return m_FarClipPlane; }
		void SetFarClipPlane(float value) { m_FarClipPlane = value; }
		
		
		float GetNearClipPlane() const { return m_NearClipPlane; }
		void SetNearClipPlane(float value) { m_NearClipPlane = value; }
		
		
		float GetFieldOfView() const { return m_FieldOfView; }
		void SetFieldOfView(float value) { m_FieldOfView = value; }
		
		
		bool GetOrthographic() const { return m_Orthographic; }
		void SetOrthographic(bool value) { m_Orthographic = value; }
		
		
		float GetOrthographicSize() const { return m_OrthographicSize; }
		void SetOrthographicSize(float value) { m_OrthographicSize = value; }


		Frustum GetFrustum() const
		{
			return {m_FieldOfView, m_FarClipPlane, m_NearClipPlane, Screen::GetAspectRatio()};
		}

		//void Deserialize(InputArchive& archive) override;
		//void Serialize(OutputArchive& archive) const override;
		
		CameraType 	m_Type = CameraType::Game;
		
	private:
		float 		m_FieldOfView = 60.0f;
		float 		m_NearClipPlane = 0.3f;
		float 		m_FarClipPlane = 1000.f;
		bool  		m_Orthographic = false;
		float 		m_OrthographicSize    = 5.f;	// Projection's half-size(vertical) when in orthographic mode.
		
		mutable Matrix4x4 	m_ProjectionMatrix;
	};

}
