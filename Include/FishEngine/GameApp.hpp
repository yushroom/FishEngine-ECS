#pragma once

#include "ClassDef.hpp"
#include "Evnet.hpp"

struct GLFWwindow;
namespace ECS
{
	class Scene;
}

class EditorSystem;
class SceneViewSystem;

class GameApp : public NonCopyable
{
public:
	GameApp() = default;

	void Init();
	void Run();

	virtual void Start() { };
//	void Update();
	virtual void Clean() { };

	void Resize(int width, int height);

	static GameApp* GetMainApp();
	//int GetWidth() const { return m_WindowWidth; }
	//int GetHeight() const { return m_WindowHeight; }

	ECS::Scene* GetScene() const { return m_Scene; }

	Event<void(int, int)> OnWindowSizeChanged;

protected:
	GLFWwindow * 	m_Window 		= nullptr;
	int 			m_WindowWidth 	= 800;
	int 			m_WindowHeight 	= 600;

	ECS::Scene* m_Scene = nullptr;

public:
	ECS::Scene* m_EditorScene = nullptr;
	EditorSystem* m_EditorSystem = nullptr;
	SceneViewSystem* m_SceneViewSystem = nullptr;
	
public:
	bool			m_WindowMinimized = false;
};
