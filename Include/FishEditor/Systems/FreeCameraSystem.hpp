#pragma once

#include <FishEngine/ECS/System.hpp>
#include <FishEditor/Components/FreeCamera.hpp>

namespace FishEngine
{
	class SingletonInput;
}


namespace FishEditor
{
	class FreeCameraSystem : public FishEngine::System
	{
		SYSTEM(FreeCameraSystem);
	public:
		
		enum class ControlType
		{
			None,
			Move,
			Rotate,
			Orbit,
			Zoom,
		};
		
		virtual void Update() override;

	private:
		void UpdateCameraTransform(FishEngine::SingletonInput* input, FishEngine::GameObject* cameraGO, FreeCamera* data);
	};
	
}
