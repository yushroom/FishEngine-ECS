#pragma once
#include "ECS.hpp"


class SingletonInput : public SingletonComponent
{
	friend class Scene;
protected:
	SingletonInput()
	{
	}

public:
	bool IsButtonDown() const;

private:
};


class InputSystem : public ISystem
{
public:
	void Start() override
	{
		m_Scene->AddSingletonComponent<SingletonInput>();
	}

	void Update() override
	{
	}
};
