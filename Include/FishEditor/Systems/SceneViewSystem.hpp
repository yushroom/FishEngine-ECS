#pragma once

#include <FishEngine/ECS/System.hpp>

namespace FishEditor
{
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

	class SceneViewSystem : public FishEngine::System
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

	};

}
