#pragma once
#include "ECS.hpp"


class TransformSystem : public ECS::ISystem
{
public:
	void Update() override
	{
		m_Scene->All([this](ECS::GameObject* go){
			ECS::EntityID parentID = go->GetParentID();
			auto& pos = go->GetTransform()->position;
			auto& mtx = go->GetTransform()->m_LocalToWorldMatrix;
			if (parentID != 0)
			{
				ECS::GameObject* parent = m_Scene->GetGameObjectByID(parentID);
				auto& ppos = parent->GetTransform()->position;
				float x = pos.x + ppos.x;
				float y = pos.y + ppos.y;
				mtx[3][0] = x;
				mtx[3][1] = y;
				mtx[3][2] = 0.0f;
			}
			else
			{
				mtx[3][0] = pos.x;
				mtx[3][1] = pos.y;
				mtx[3][2] = 0.0f;
			}
		});
	}
};
