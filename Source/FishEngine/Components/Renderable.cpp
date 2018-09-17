#include <FishEngine/Components/Renderable.hpp>
#include <FishEngine/Gizmos.hpp>
#include <FishEngine/Components/Transform.hpp>
#include <FishEngine/Mesh.hpp>

using namespace FishEngine;

void Renderable::OnDrawGizmosSelected() const
{
	// draw bounding box
	Gizmos::color = Vector4(0, 1, 0, 1);
	Gizmos::matrix = GetTransform()->GetLocalToWorldMatrix();
	if (this->mesh != nullptr)
	{
		Gizmos::DrawBounds(mesh->bounds);
	}
}
