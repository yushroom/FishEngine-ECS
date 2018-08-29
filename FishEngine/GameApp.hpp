#pragma once

#include "ClassDef.hpp"

struct GLFWwindow;
class Scene;

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

	Scene* GetScene() const { return m_Scene; }

protected:
	GLFWwindow * 	m_Window 		= nullptr;
	int 			m_WindowWidth 	= 640;
	int 			m_WindowHeight 	= 480;

	Scene* m_Scene = nullptr;
};
