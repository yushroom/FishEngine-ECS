#pragma once

#include <FishEngine/ECS.hpp>

enum class TransformToolType
{
	None,
	Translate,
	Rotate,
	Scale,
};

enum class TransformPivot
{
	Pivot,
	Center,
};

enum class TransformSpace
{
	Global,
	Local,
};

enum class ShadingMode
{
	Shaded,
	Wireframe,
	ShadedWireframe,
};

class SceneViewSystem : public ECS::ISystem
{
	SYSTEM(SceneViewSystem);
public:
	void OnAdded() override;
	//void Update() override;

	void DrawGizmos();

	bool m_EnableTransform = true;
	TransformToolType m_transformToolType = TransformToolType::Translate;
	TransformPivot m_transformPivot = TransformPivot::Center;
	TransformSpace m_transformSpace = TransformSpace::Global;

private:
	bool m_Dragging = false;
	int m_SelectedAxis = -1;
	Ray m_Ray;
	Vector2 m_MousePosition;
	Matrix4x4 m_Clip2World;

private:
	void __OnDragBegin();
	void __OnDragMove();
	void __OnDragEnd();
};