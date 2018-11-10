#include <FishEngine/Components/Renderable.hpp>
#include <FishEngine/Gizmos.hpp>
#include <FishEngine/Components/Transform.hpp>
#include <FishEngine/Mesh.hpp>
#include <FishEngine/ECS/GameObject.hpp>

using namespace FishEngine;

void Renderable::OnDrawGizmosSelected() const
{
	// draw bounding box
	Gizmos::color = Vector4(0, 1, 0, 1);
	Gizmos::matrix = m_GameObject->GetTransform()->GetLocalToWorldMatrix();
	if (this->m_Mesh != nullptr)
	{
		Gizmos::DrawBounds(m_Mesh->m_Bounds);
	}
}
