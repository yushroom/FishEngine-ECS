#pragma once

#include <FishEngine/ECS.hpp>

namespace FishEditor
{

	class FreeCamera : public FishEngine::Component
	{
		COMPONENT(FreeCamera);
	public:
		bool m_LookAtMode = false;
		float m_RotateSpeed = 200;
		float m_DragSpeed = 20;
		FishEngine::Vector3 m_OrbitCenter = FishEngine::Vector3::zero;
	};

}
