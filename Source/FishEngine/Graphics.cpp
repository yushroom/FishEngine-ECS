#include "Graphics.hpp"

#include "RenderSystem.hpp"
#include "Engine.hpp"
#include "Mesh.hpp"
#include "Material.hpp"

#include "GameApp.hpp"

void Graphics::DrawMesh(Mesh* mesh, const Matrix4x4& matrix, Material* material)
{
	if (mesh == nullptr || material == nullptr)
		return;

	auto state = GameApp::GetMainApp()->GetScene()->GetSingletonComponent<SingletonRenderState>();
	
	// Set model matrix for rendering.
	bgfx::setTransform(matrix.transpose().data());
	
	mesh->Bind();
	
	// Set render states.
	bgfx::setState(state->GetState());
	
	// Submit primitive for rendering to view 0.
	bgfx::submit(0, material->m_Shader->GetProgram());
}
