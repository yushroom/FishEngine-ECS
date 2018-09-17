#pragma once
#include <FishEngine/ECS/System.hpp>
#include <FishEngine/Components/Transform.hpp>

namespace FishEngine
{

class TransformSystem : public System
{
	SYSTEM(TransformSystem);
public:
	void Update() override
	{
	//	m_Scene->All([this](GameObject* go){
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

}
