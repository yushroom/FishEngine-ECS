#include <FishEngine/Systems/SelectionSystem.hpp>
#include <FishEngine/Gizmos.hpp>
#include <FishEngine/Components/Transform.hpp>
#include <FishEngine/Components/Renderable.hpp>
#include <FishEngine/Mesh.hpp>

void SelectionSystem::Update()
{
	if (selected != nullptr)
	{
		Gizmos::matrix = selected->GetTransform()->GetLocalToWorldMatrix();
		Gizmos::color = Vector4(1, 0, 0, 1);
		Gizmos::DrawLine(Vector3::zero, Vector3{ 1, 0, 0 });
		Gizmos::color = Vector4(0, 1, 0, 1);
		Gizmos::DrawLine(Vector3::zero, Vector3{ 0, 1, 0 });
		Gizmos::color = Vector4(0, 0, 1, 1);
		Gizmos::DrawLine(Vector3::zero, Vector3{ 0, 0, 1 });

		auto r = selected->GetComponent<Renderable>();
		if (r != nullptr && r->mesh != nullptr)
		{
			Gizmos::DrawBounds(r->mesh->bounds);
		}
	}
}
