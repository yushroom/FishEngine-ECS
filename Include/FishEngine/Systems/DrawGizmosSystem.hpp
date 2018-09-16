#pragma once

#include "../ECS.hpp"

#include "../Components/SingletonSelection.hpp"
#include "../Components/Camera.hpp"
#include "../Gizmos.hpp"

class DrawGizmosSystem : public ECS::ISystem
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
