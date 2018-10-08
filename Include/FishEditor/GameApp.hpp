#pragma once

#include <FishEngine/ClassDef.hpp>
#include <FishEngine/Evnet.hpp>

struct GLFWwindow;


namespace FishEngine
{
	class Scene;
}

namespace FishEditor
{
	
	class EditorSystem;
	class SceneViewSystem;

	class GameApp : public FishEngine::NonCopyable
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

		FishEngine::Scene* GetScene() const { return m_Scene; }

		FishEngine::Event<void(int, int)> OnWindowSizeChanged;

	protected:
		GLFWwindow * 	m_Window 		= nullptr;
		int 			m_WindowWidth 	= 1280;
		int 			m_WindowHeight 	= 800;

		FishEngine::Scene* m_Scene = nullptr;

	public:
		FishEngine::Scene* m_EditorScene = nullptr;
		EditorSystem* m_EditorSystem = nullptr;
		SceneViewSystem* m_SceneViewSystem = nullptr;
		
	public:
		bool			m_WindowMinimized = false;
	};
	
}
