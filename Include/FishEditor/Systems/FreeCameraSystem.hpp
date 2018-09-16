#pragma once

#include <FishEngine/ECS.hpp>
#include <FishEditor/Components/FreeCamera.hpp>

class SingletonInput;

class FreeCameraSystem : public ECS::ISystem
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
	void UpdateCameraTransform(SingletonInput* input, ECS::GameObject* cameraGO, FreeCamera* data);
};