#include <FishEngine/Systems/InputSystem.hpp>
#include <FishEngine/ECS/Scene.hpp>
#include <FishEngine/Components/SingletonTime.hpp>

using namespace FishEngine;

void InputSystem::OnAdded()
{
	m_Scene->input = m_Scene->AddSingletonComponent<SingletonInput>();
}

void InputSystem::Update()
{

}

void InputSystem::PostUpdate()
{
	auto input = m_Scene->input;
	for (int i = 0; i < SingletonInput::ButtonCount; ++i)
	{
		auto& action = input->m_KeyPressed[i];
		if (action == KeyAction::Normal)
		{
		}
		else if (action == KeyAction::Pressed)
		{
			action = KeyAction::Held;
//			si->m_KeyHeldTime[i] = 0;
		}
		else if (action == KeyAction::Released)
		{
			action = KeyAction::Normal;
			input->m_KeyHeldTime[i] = 0;
		}
		else // held
		{
			input->m_KeyHeldTime[i] += m_Scene->time->deltaTime;
		}
	}
	
	for (auto& a : input->m_Axis)
	{
		a = 0.f;
	}
}

void InputSystem::SetMousePosition(float x, float y)
{
	auto si = m_Scene->input;
	auto& p1 = si->m_MousePosition;
	si->m_Axis[(int)Axis::MouseX] = x - p1.x;
	si->m_Axis[(int)Axis::MouseY] = y - p1.y;
	p1.Set(x, y);
}

void InputSystem::UpdateAxis(Axis axis, float value)
{
	auto si = m_Scene->input;
	si->m_Axis[(int)axis] = value;
}

void InputSystem::PostKeyEvent(const KeyEvent& e)
{
	auto si = m_Scene->input;
	si->m_KeyPressed[(int)e.key] = e.action;
}
