#pragma once
#include "ECS.hpp"

enum class KeyCode
{
	None = 0,
	ECS,
	F1, F2, F3, F4, F5, F6, F7, F8, F9, F10,

	MouseLeftButton,
	MouseRightButton,
	MouseMiddleButton,
	RightShift,
	LeftShift,
	RightControl,
	LeftControl,
	RightAlt,
	LeftAlt,
	LeftCommand,
	RightCommand,
};

enum class KeyAction
{
	Normal,
	Pressed,
	Held,
	Released
};

enum class Axis {
	Vertical = 0,   // w, a, s, d and arrow keys
	Horizontal,
	Fire1,          // Control
	Fire2,          // Option(Alt)
	Fire3,          // Command
	Jump,
	MouseX,         // delta of mouse movement
	MouseY,
	MouseScrollWheel,
	WindowShakeX,   // movement of the window
	WindwoShakeY,
	AxisCount,
};


class SingletonInput : public ECS::SingletonComponent
{
	friend class ECS::Scene;
	friend class InputSystem;
protected:
	SingletonInput()
	{
	}

public:
	bool IsButtonPressed(KeyCode code) const;
	bool IsButtonHeld(KeyCode code) const;
	bool IsButtonReleased(KeyCode code) const;

	Vector2 GetMousePosition() const { return m_MousePosition; }
	float GetAxis(Axis axis) const { return m_Axis[(int)axis]; }
	
private:
	KeyAction m_KeyPressed[256] = {KeyAction::Normal};
	float m_Axis[(int)Axis::AxisCount] = {0};
	Vector2 m_MousePosition;
};


struct KeyEvent
{
	KeyCode key = KeyCode::None;
	KeyAction action = KeyAction::Normal;
};


class InputSystem : public ECS::ISystem
{
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
