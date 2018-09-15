#pragma once

#include "../ECS.hpp"
#include <FishEditor/HierarchyView.hpp>


class EditorSystem : public ECS::ISystem
{
	SYSTEM(EditorSystem);
public:
	void OnAdded() override;
	void Draw();

	void Play();
	void Pause();
	void Stop();

	bool m_IsPlaying = false;
	int m_WindowWidth = 4;
	int m_WindowHeight = 4;
	ECS::Scene* m_GameScene = nullptr;

	Vector4 m_SceneViewRect{ 0, 0, 1, 1 };
	Transform* selected = nullptr;

private:
	void MainMenu();
	void MainToolBar();
	void Hierarchy();
	void Inspector();
	
	HierarchyView m_HierarchyView;
};
