#pragma once

#include "ECS.hpp"

class FreeCamera : public ECS::Component
{
	COMPONENT(FreeCamera);
public:
	bool m_LookAtMode = false;
	float m_RotateSpeed = 200;
	float m_DragSpeed = 20;
	Vector3 m_OrbitCenter = Vector3::zero;
};

class SingletonInput;

class FreeCameraController : public ECS::ISystem
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
