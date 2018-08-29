#pragma once

#include "ClassDef.hpp"

struct GLFWwindow;

class GameApp : public NonCopyable
{
public:
	GameApp() = default;

	void Init();
	void Run();

	virtual void Start() = 0;
	virtual void Update() = 0;
	virtual void Clean() { };

	void Resize(int width, int height);

	static GameApp* GetMainApp();
	int GetWidth() const { return m_WindowWidth; }
	int GetHeight() const { return m_WindowHeight; }

protected:
	GLFWwindow * 	m_Window 		= nullptr;
	float 			m_WindowWidth 	= 640;
	float 			m_WindowHeight 	= 480;
};
