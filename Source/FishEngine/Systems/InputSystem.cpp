#include <FishEngine/Systems/InputSystem.hpp>

bool SingletonInput::IsButtonPressed(KeyCode code) const
{
	int icode = (int)code;
	return m_KeyPressed[icode] == KeyAction::Pressed;
}

bool SingletonInput::IsButtonHeld(KeyCode code) const
{
	int icode = (int)code;
	return m_KeyPressed[icode] == KeyAction::Held;
}

bool SingletonInput::IsButtonReleased(KeyCode code) const
{
	int icode = (int)code;
	return m_KeyPressed[icode] == KeyAction::Released;
}

void InputSystem::OnAdded()
{
	m_Scene->AddSingletonComponent<SingletonInput>();
}

void InputSystem::Update()
{

}

void InputSystem::PostUpdate()
{
	auto si = m_Scene->GetSingletonComponent<SingletonInput>();
	for (auto& action : si->m_KeyPressed)
	{
		if (action == KeyAction::Normal)
			;
		if (action == KeyAction::Pressed)
			action = KeyAction::Held;
		else if (action == KeyAction::Released)
			action = KeyAction::Normal;
	}
	
	for (auto& a : si->m_Axis)
	{
		a = 0.f;
	}
}

void InputSystem::SetMousePosition(float x, float y)
{
	auto si = m_Scene->GetSingletonComponent<SingletonInput>();
	auto& p1 = si->m_MousePosition;
	si->m_Axis[(int)Axis::MouseX] = x - p1.x;
	si->m_Axis[(int)Axis::MouseY] = y - p1.y;
	p1.Set(x, y);
}

void InputSystem::UpdateAxis(Axis axis, float value)
{
	auto si = m_Scene->GetSingletonComponent<SingletonInput>();
	si->m_Axis[(int)axis] = value;
}

void InputSystem::PostKeyEvent(const KeyEvent& e)
{
	auto si = m_Scene->GetSingletonComponent<SingletonInput>();
	si->m_KeyPressed[(int)e.key] = e.action;
}
