#pragma once

#include <FishEngine/ECS.hpp>
#include <FishEngine/Gizmos.hpp>
#include <FishEngine/Components/Camera.hpp>
#include <FishEngine/ECS/Scene.hpp>
#include <FishEditor/Components/SingletonSelection.hpp>

namespace FishEditor
{

	class DrawGizmosSystem : public FishEngine::System
	{
		SYSTEM(DrawGizmosSystem);
	public:
		// void OnAdded() override;
		
		void Update() override
		{
			auto selected = m_Scene->GetSingletonComponent<SingletonSelection>()->selected;
			if (selected == nullptr)
				return;
			for (auto comp : selected->GetComponents())
			{
				comp->OnDrawGizmosSelected();
			}
		}
	};

}
