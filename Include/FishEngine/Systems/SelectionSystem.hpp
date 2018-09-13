#pragma once

#include <FishEngine/ECS.hpp>

class SelectionSystem : public ECS::ISystem
{
	SYSTEM(SelectionSystem);
public:
	void Update() override;
	
	ECS::GameObject* selected = nullptr;
	
	bool m_EnableTransform = true;

private:
	bool m_Dragging = false;
	int m_SelectedAxis = -1;
	Ray m_Ray;
	Vector2 m_MousePosition;
	Matrix4x4 m_Clip2World;
	
	void __OnDragBegin();
	void __OnDragMove();
	void __OnDragEnd();
};
