#include "FishEngine/Graphics.hpp"

#include <FishEngine/Systems/RenderSystem.hpp>
#include "FishEngine/Engine.hpp"
#include "FishEngine/Mesh.hpp"
#include <FishEngine/Shader.hpp>
#include "FishEngine/Material.hpp"

#include "FishEngine/GameApp.hpp"

void Graphics::DrawMesh(Mesh* mesh, const Matrix4x4& matrix, Material* material, bgfx::ViewId id)
{
	if (mesh == nullptr || material == nullptr)
		return;

	auto state = GameApp::GetMainApp()->GetScene()->GetSingletonComponent<SingletonRenderState>();
	
	// Set model matrix for rendering.
	bgfx::setTransform(matrix.transpose().data());
	// Set render states.
	bgfx::setState(state->GetState());
	material->BindUniforms();

	mesh->Bind(-1, id);
	// Submit primitive for rendering to view 0.
	bgfx::submit(id, material->GetShader()->GetProgram());
}
