#pragma once
#include "ECS.hpp"


class TransformSystem : public ISystem
{
public:
	void Update() override
	{
		m_Scene->All([this](GameObject* go){
			EntityID parentID = go->GetParentID();
			auto& pos = go->GetTransform()->position;
			auto& mtx = go->GetTransform()->m_LocalToWorldMatrix;
			if (parentID != 0)
			{
				GameObject* parent = m_Scene->GetGameObjectByID(parentID);
				auto& ppos = parent->GetTransform()->position;
				float x = pos.x + ppos.x;
				float y = pos.y + ppos.y;
				mtx[12] = x;
				mtx[13] = y;
				mtx[14] = 0.0f;
			}
			else
			{
				mtx[12] = pos.x;
				mtx[13] = pos.y;
				mtx[14] = 0.0f;
			}
		});
	}
};
