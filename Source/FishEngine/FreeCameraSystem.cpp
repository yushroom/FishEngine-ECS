#include <FishEngine/Systems/FreeCameraSystem.hpp>
#include <FishEngine/Components/Transform.hpp>
#include <FishEngine/Components/SingletonInput.hpp>

void FreeCameraSystem::Update()
{
	SingletonInput* input = m_Scene->GetSingletonComponent<SingletonInput>();
	m_Scene->ForEach<FreeCamera>([this, input](ECS::GameObject* go, FreeCamera* data) {
		UpdateCameraTransform(input, go, data);
	});
}

void FreeCameraSystem::UpdateCameraTransform(SingletonInput* input, ECS::GameObject* cameraGO, FreeCamera* data)
{
	Transform* t = cameraGO->GetTransform();
	
	bool alt = input->IsButtonHeld(KeyCode::LeftAlt) || input->IsButtonHeld(KeyCode::RightAlt);
	bool ctrl = input->IsButtonHeld(KeyCode::LeftControl) || input->IsButtonHeld(KeyCode::RightControl);
	bool cmd = input->IsButtonHeld(KeyCode::LeftCommand) || input->IsButtonHeld(KeyCode::RightCommand);
	bool left = input->IsButtonPressed(KeyCode::MouseLeftButton) || input->IsButtonHeld(KeyCode::MouseLeftButton);
	bool right = input->IsButtonPressed(KeyCode::MouseRightButton) || input->IsButtonHeld(KeyCode::MouseRightButton);
	bool middle = input->IsButtonPressed(KeyCode::MouseMiddleButton) || input->IsButtonHeld(KeyCode::MouseMiddleButton);
	float scrollValue = input->GetAxis(Axis::MouseScrollWheel);
	bool scroll = scrollValue != 0.0f;

	Vector3 rotateCenter = data->m_OrbitCenter;

	ControlType type = ControlType::None;
	if (middle || (alt && ctrl && left) || (alt && cmd && left))
	{
		type = ControlType::Move;
	}
	else if (alt && left)
	{
		type = ControlType::Orbit;
		rotateCenter = data->m_OrbitCenter;
	}
	else if (scroll || (alt && right))
	{
		type = ControlType::Zoom;
	}
	else if (right)
	{
		type = ControlType::Rotate;
		rotateCenter = t->position;
	}

	if (type == ControlType::Move)
	{
		float x = data->m_DragSpeed * input->GetAxis(Axis::MouseX);
		float y = data->m_DragSpeed * input->GetAxis(Axis::MouseY);
		t->Translate({ -x, y, 0 });
	}
	else if (type == ControlType::Rotate || type == ControlType::Orbit)
	{
		float x = data->m_RotateSpeed * input->GetAxis(Axis::MouseX);
		float y = data->m_RotateSpeed * input->GetAxis(Axis::MouseY);
		Vector3 right = t->GetRight();
		right.y = 0;
		t->RotateAround(rotateCenter, right, y);
		t->RotateAround(rotateCenter, Vector3::up, x);
	}
	else if (type == ControlType::Zoom)
	{
		auto forward = t->GetForward();
		float deltaZ = 0;
		if (scrollValue != 0.f)
		{
			deltaZ = 0.2f*scrollValue;
		}
		else
		{
			float x = data->m_DragSpeed * input->GetAxis(Axis::MouseX);
			float y = data->m_DragSpeed * input->GetAxis(Axis::MouseY);
			deltaZ = fabsf(x) > fabsf(y) ? x : -y;
		}
		t->Translate(deltaZ*forward);
	}
	
	if (input->IsButtonPressed(KeyCode::R))
	{
		t->position = {0, 0, -15};
		t->rotation = Quaternion::identity;
	}
}

