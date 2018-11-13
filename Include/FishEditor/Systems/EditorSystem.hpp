#pragma once

#include <FishEngine/ECS/System.hpp>
#include <FishEditor/HierarchyView.hpp>

namespace FishEditor
{
	class EditorSystem : public FishEngine::System
	{
		SYSTEM(EditorSystem);
	public:
		void OnAdded() override;
		void Update() override;
		void Draw();

		void Play();
		void Pause();
		void Stop();

		bool m_IsPlaying = false;
//		int m_WindowWidth = 4;
//		int m_WindowHeight = 4;
		FishEngine::Scene* m_GameScene = nullptr;

		FishEngine::Vector4 m_SceneViewRect{ 0, 0, 1, 1 };
		FishEngine::Transform* selected = nullptr;

	private:
		void MainMenu();
		void MainToolBar();
		void Hierarchy();
		void Inspector();
		void StatusBar();
		
		HierarchyView m_HierarchyView;
	};

}
