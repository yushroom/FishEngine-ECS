#pragma once

#include "../ECS/Component.hpp"
#include "../Math/Frustum.hpp"
#include "../Math/Matrix4x4.hpp"

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

		Matrix4x4 GetProjectionMatrix() const;
		
		// Matrix that transforms from world to camera space (i.e. view matrix).
		Matrix4x4 GetWorldToCameraMatrix() const;

		Matrix4x4 GetCameraToWorldMatrix() const;
		
		Matrix4x4 GetViewProjectionMatrix() const;
		
		// Returns a ray going from camera through a screen point.
		Ray ScreenPointToRay(const Vector3& position);

		static Camera* GetMainCamera();

		static Camera* GetEditorCamera();
		
		void OnDrawGizmosSelected() const override;
		
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


		Frustum GetFrustum() const;

		//void Deserialize(InputArchive& archive) override;
		//void Serialize(OutputArchive& archive) const override;
		
		CameraType 	m_Type = CameraType::Game;
		
	private:
		float 		m_FieldOfView = 60.0f;
		float 		m_NearClipPlane = 0.3f;
		float 		m_FarClipPlane = 1000.f;
		bool  		m_Orthographic = false;
		float 		m_OrthographicSize    = 5.f;	// Projection's half-size(vertical) when in orthographic mode.
		
		Meta(NonSerializable)
		mutable Matrix4x4 	m_ProjectionMatrix;
	};

}
