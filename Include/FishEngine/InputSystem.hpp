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
};

enum class KeyAction
{
	Normal,
	Pressed,
	Held,
	Released
};

class SingletonInput : public SingletonComponent
{
	friend class Scene;
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

private:
	KeyAction m_KeyPressed[256] = {KeyAction::Normal};
	Vector2 m_MousePosition;
};


struct KeyEvent
{
	KeyCode key = KeyCode::None;
	KeyAction action = KeyAction::Normal;
};


class InputSystem : public ISystem
{
public:

	void OnAdded() override;

	void Start() override
	{
	}

	void Update() override;
	void PostUpdate() override;

	void SetMousePosition(float x, float y);
	void PostKeyEvent(const KeyEvent& e);
};
