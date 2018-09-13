#pragma once

#include "../ECS.hpp"

#include "SelectionSystem.hpp"
#include "../Components/Camera.hpp"
#include "../Gizmos.hpp"

class DrawGizmosSystem : public ECS::ISystem
{
	SYSTEM(DrawGizmosSystem);
public:
	// void OnAdded() override;
	
	void Update() override
	{
		auto selected = m_Scene->GetSystem<SelectionSystem>()->selected;
		for (auto comp : selected->GetComponents())
		{
			comp->OnDrawGizmosSelected();
		}
	}
};
