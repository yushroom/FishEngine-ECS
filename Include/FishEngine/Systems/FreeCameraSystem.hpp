#pragma once

#include <FishEngine/ECS.hpp>
#include <FishEngine/Components/FreeCamera.hpp>

class SingletonInput;

class FreeCameraSystem : public ECS::ISystem
{
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
	void UpdateCameraTransform(SingletonInput* input, ECS::GameObject* cameraGO, FreeCamera* data);
};
