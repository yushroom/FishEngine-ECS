#include "FishEngine/Graphics.hpp"

#include <FishEngine/Systems/RenderSystem.hpp>
#include "FishEngine/Engine.hpp"
#include "FishEngine/Mesh.hpp"
#include <FishEngine/Shader.hpp>
#include "FishEngine/Material.hpp"

#include "FishEngine/GameApp.hpp"

void Graphics::DrawMesh(Mesh* mesh, const Matrix4x4& matrix, Material* material, bgfx::ViewId id, int submeshID)
{
	if (mesh == nullptr || material == nullptr)
		return;

	// Set render states.
	auto state = GameApp::GetMainApp()->GetScene()->GetSingletonComponent<SingletonRenderState>();
	bgfx::setState(state->GetState());
	
	// Set model matrix for rendering.
	bgfx::setTransform(matrix.transpose().data());
	material->BindUniforms();
	mesh->Bind(submeshID, id);
	// Submit primitive for rendering to view 0.
	bgfx::submit(id, material->GetShader()->GetProgram());
}

void Graphics::DrawMesh2(Mesh* mesh, const Matrix4x4& matrix, Material* material, uint64_t state)
{
	if (mesh == nullptr || material == nullptr)
		return;

	// Set render states.
	bgfx::setState(state);

	// Set model matrix for rendering.
	bgfx::setTransform(matrix.transpose().data());
	material->BindUniforms();
	mesh->Bind();
	// Submit primitive for rendering to view 0.
	bgfx::submit(0, material->GetShader()->GetProgram());
}
