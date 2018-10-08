#include <FishEngine/Graphics.hpp>
#include <FishEngine/ECS/Scene.hpp>
#include <FishEngine/Components/SingletonRenderState.hpp>
#include <FishEngine/Engine.hpp>
#include <FishEngine/Mesh.hpp>
#include <FishEngine/Shader.hpp>
#include <FishEngine/Material.hpp>

//#include "FishEngine/GameApp.hpp"

using namespace FishEngine;
#if 0
void Graphics::DrawMesh(Mesh* mesh, const Matrix4x4& matrix, Material* material, bgfx::ViewId id, int submeshID)
{
	if (mesh == nullptr || material == nullptr)
		return;

	auto state = Scene::s_Current->GetSingletonComponent<SingletonRenderState>()->GetState();
	
	Graphics::DrawMesh2(mesh, matrix, material, state, id, submeshID);
}


void Graphics::DrawMesh2(Mesh* mesh, const Matrix4x4& matrix, Material* material, uint64_t state, bgfx::ViewId id, int submeshID)
{
	if (mesh == nullptr || material == nullptr)
		return;

	// Set render states.
	bgfx::setState(state);

	// Set model matrix for rendering.
	bgfx::setTransform(matrix.transpose().data());
	material->BindUniforms();
	mesh->Bind(submeshID, id);
	// Submit primitive for rendering to view 0.
	bgfx::submit(id, material->GetShader()->GetProgram());
}
#endif

#include <FishEngine/Render/Application.h>

void FishEngine::Graphics::DrawMesh(Mesh * mesh, const Matrix4x4 & matrix, Material * material, int submeshID)
{
	abort();
}
