#pragma once

#include <FishEngine/ECS.hpp>

#include <FishEngine/Gizmos.hpp>

class SelectionSystem : public ECS::ISystem
{
public:
	void Update() override
	{
		if (selected != nullptr)
		{
			Gizmos::matrix = selected->GetTransform()->GetLocalToWorldMatrix();
			Gizmos::color = Vector4(1, 0, 0, 1);
			Gizmos::DrawLine(Vector3::zero, Vector3{1, 0, 0});
			Gizmos::color = Vector4(0, 1, 0, 1);
			Gizmos::DrawLine(Vector3::zero, Vector3{ 0, 1, 0 });
			Gizmos::color = Vector4(0, 0, 1, 1);
			Gizmos::DrawLine(Vector3::zero, Vector3{ 0, 0, 1 });
		}
	}
	
	ECS::GameObject* selected = nullptr;
};
