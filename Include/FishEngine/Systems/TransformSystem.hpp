#pragma once
#include <FishEngine/ECS.hpp>
#include <FishEngine/Components/Transform.hpp>


class TransformSystem : public ECS::ISystem
{
	SYSTEM(TransformSystem);
public:
	void Update() override
	{
	//	m_Scene->All([this](ECS::GameObject* go){
	//		Transform* t = go->GetTransform();
	//		Transform* parent = t->GetParent();
	//		t->m_LocalToWorldMatrix = Matrix4x4::TRS(t->m_, t->rotation, t->scale);
	//		if (parent != nullptr)
	//		{
	//			t->m_LocalToWorldMatrix = parent->GetLocalToWorldMatrix() * t->m_LocalToWorldMatrix;
	//		}
	//	});
	}
};
