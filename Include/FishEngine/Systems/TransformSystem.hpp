#pragma once
#include <FishEngine/ECS.hpp>
#include <FishEngine/Components/Transform.hpp>


class TransformSystem : public ECS::ISystem
{
public:
	void Update() override
	{
		m_Scene->All([this](ECS::GameObject* go){
			auto parentID = go->GetParentID();
			Transform* t = go->GetTransform();
			t->m_LocalToWorldMatrix = Matrix4x4::TRS(t->position, t->rotation, t->scale);
			if (parentID != 0)
			{
				ECS::GameObject* parent = m_Scene->GetGameObjectByID(parentID);
				t->m_LocalToWorldMatrix = parent->GetTransform()->GetLocalToWorldMatrix() * t->m_LocalToWorldMatrix;
			}
		});
	}
};
