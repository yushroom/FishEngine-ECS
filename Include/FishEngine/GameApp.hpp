#pragma once

#include "ClassDef.hpp"

struct GLFWwindow;
namespace ECS
{
	class Scene;
}

class GameApp : public NonCopyable
{
public:
	GameApp() = default;

	void Init();
	void Run();

	virtual void Start() { };
	virtual void Update() { };
	virtual void Clean() { };

	void Resize(int width, int height);

	static GameApp* GetMainApp();
	int GetWidth() const { return m_WindowWidth; }
	int GetHeight() const { return m_WindowHeight; }

	ECS::Scene* GetScene() const { return m_Scene; }

protected:
	GLFWwindow * 	m_Window 		= nullptr;
	int 			m_WindowWidth 	= 800;
	int 			m_WindowHeight 	= 600;

	ECS::Scene* m_Scene = nullptr;
};
