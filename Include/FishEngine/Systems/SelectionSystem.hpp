#pragma once

#include <FishEngine/ECS.hpp>

class SelectionSystem : public ECS::ISystem
{
	SYSTEM(SelectionSystem);
public:
	void Update() override;
	
	ECS::GameObject* selected = nullptr;
	
private:
	bool m_Dragging = false;
	int m_SelectedAxis = -1;
	Ray m_Ray;
	Vector2 m_MousePosition;
	Matrix4x4 clip2World;
	
	void __OnDragBegin();
	void __OnDragMove();
	void __OnDragEnd();
};
