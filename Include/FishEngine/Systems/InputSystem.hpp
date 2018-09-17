#pragma once
#include <FishEngine/ECS/System.hpp>
#include <FishEngine/Components/SingletonInput.hpp>

namespace FishEngine
{

	struct KeyEvent
	{
		KeyCode key = KeyCode::None;
		KeyAction action = KeyAction::Normal;
	};


	class InputSystem : public System
	{
		SYSTEM(InputSystem);
	public:

		void OnAdded() override;

		void Start() override
		{
		}

		void Update() override;
		void PostUpdate() override;

		void SetMousePosition(float x, float y);
		void UpdateAxis(Axis axis, float value);
		void PostKeyEvent(const KeyEvent& e);
	};

}
