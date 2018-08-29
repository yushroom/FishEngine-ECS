#define _ITERATOR_DEBUG_LEVEL 0
#include "Graphics.hpp"

#include "RenderSystem.hpp"
#include "Engine.hpp"
#include "Mesh.hpp"
#include "Material.hpp"

void Graphics::DrawMesh(Mesh* mesh, Matrix matrix, Material* material)
{
	if (mesh == nullptr || material == nullptr)
		return;
	
	auto& rs = RenderSystem::GetInstance();
	
	// Set model matrix for rendering.
	bgfx::setTransform(matrix);
	
	mesh->Bind();
	
	// Set render states.
	bgfx::setState(rs.GetState());
	
	// Submit primitive for rendering to view 0.
	bgfx::submit(0, material->m_Shader->GetProgram());
}
