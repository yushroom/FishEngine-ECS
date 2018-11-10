#include <FishEngine/Graphics.hpp>
#include <FishEngine/ECS/Scene.hpp>
#include <FishEngine/Components/SingletonRenderState.hpp>
#include <FishEngine/Engine.hpp>
#include <FishEngine/Mesh.hpp>
#include <FishEngine/Shader.hpp>
#include <FishEngine/Material.hpp>

//#include "FishEngine/GameApp.hpp"

using namespace FishEngine;

struct RenderObject
{
	Material* material;
	Mesh* mesh;
	Matrix4x4 localToWorld;
	int submeshID;
	
//	RenderObject(Material* mat, Mesh* mesh, M)
};

//std::vector<RenderObject> g_RenderQueue;

void Graphics::BeginFrame()
{
//	g_RenderQueue.clear();
}

void Graphics::DrawMesh(Mesh* mesh, const Matrix4x4& matrix, Material* material, int submeshID)
{
	if (mesh == nullptr || material == nullptr)
		return;

	auto state = Scene::s_Current->GetSingletonComponent<SingletonRenderState>()->GetState();
	
	Graphics::DrawMesh2(mesh, matrix, material, state, submeshID);
//	auto& obj = g_RenderQueue.emplace_back();
//	obj.material = material;
//	obj.mesh = mesh;
//	obj.localToWorld = matrix;
//	obj.submeshID = submeshID;
}


void Graphics::DrawMesh2(Mesh* mesh, const Matrix4x4& matrix, Material* material, uint64_t state, int submeshID)
{
	if (mesh == nullptr || material == nullptr)
		return;
	
	FishEngine::SetModelMatrix(matrix);
	FishEngine::Draw(mesh, material, submeshID);
	
#if 0
	// Set render states.
	bgfx::setState(state);

	// Set model matrix for rendering.
	bgfx::setTransform(matrix.transpose().data());
	material->BindUniforms();
	mesh->Bind(submeshID, id);
	// Submit primitive for rendering to view 0.
	bgfx::submit(id, material->GetShader()->GetProgram());
#endif
}


void Graphics::EndFrame()
{
//	std::sort(std::begin(g_RenderQueue), std::end(g_RenderQueue), [](const RenderObject& a, const RenderObject& b){
//		return a.material < b.material;
//	});
	
	//std::count(g_RenderQueue.begin(), g_RenderQueue.end(), <#const _Tp &__value_#>)
	
//	for (auto& ro : g_RenderQueue)
//	{
//		DrawMesh2(ro.mesh, ro.localToWorld, ro.material, ro.submeshID);
//	}
}
