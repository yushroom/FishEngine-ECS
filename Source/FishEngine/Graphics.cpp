#include <FishEngine/Graphics.hpp>
#include <FishEngine/ECS/Scene.hpp>
#include <FishEngine/Components/SingletonRenderState.hpp>
#include <FishEngine/Engine.hpp>
#include <FishEngine/Mesh.hpp>
#include <FishEngine/Shader.hpp>
#include <FishEngine/Material.hpp>
#include <FishEngine/Screen.hpp>

#include <FishEngine/Components/SingletonInput.hpp>

// TODO
#include <FishEditor/GameApp.hpp>

using namespace FishEngine;

#define USE_QUEUE 1

#if USE_QUEUE
struct RenderObject
{
	Material* material;
	Mesh* mesh;
	Matrix4x4 localToWorld;
	int submeshID;
	
//	RenderObject(Material* mat, Mesh* mesh, M)
};

std::vector<RenderObject> g_RenderQueue;
#endif

RenderPipelineState g_rps;
RenderPipelineState g_PickingRPS;
Material* g_PickingMaterial = nullptr;
extern TextureHandle g_PickingRT;
extern TextureHandle g_PickingRTDepth;
extern TextureHandle g_MainColorRT;
extern TextureHandle g_MainDepthRT;
extern FishEditor::GameApp* mainApp;

void Graphics::StaticInit()
{
	// before shader compile
	Shader* pbrShader = Shader::Find("pbrMetallicRoughness");
	g_rps.SetShader(pbrShader);
	g_rps.SetVertexDecl(PUNTVertex::ms_decl);
	g_rps.Create("pbrMetallicRoughness");
	
	g_PickingRPS.SetShader(Shader::Find("Color"));
	g_PickingRPS.SetVertexDecl(PUNTVertex::ms_decl);
	g_PickingRPS.SetDepthAttachmentFormat(TextureFormat::Depth32Float);
	g_PickingRPS.Create("PickingID");
	g_PickingMaterial = Material::Clone(Material::ColorMaterial);
}

void Graphics::BeginFrame()
{
#if USE_QUEUE
	g_RenderQueue.clear();
#endif
}

void Graphics::DrawMesh(Mesh* mesh, const Matrix4x4& matrix, Material* material, int submeshID)
{
	if (mesh == nullptr || material == nullptr)
		return;

#if !USE_QUEUE
	auto state = Scene::s_Current->GetSingletonComponent<SingletonRenderState>()->GetState();
	Graphics::DrawMesh2(mesh, matrix, material, state, submeshID);
#else
	auto& obj = g_RenderQueue.emplace_back();
	obj.material = material;
	obj.mesh = mesh;
	obj.localToWorld = matrix;
	obj.submeshID = submeshID;
#endif
}


void Graphics::DrawMesh2(Mesh* mesh, const Matrix4x4& matrix, Material* material, uint64_t state, int submeshID)
{
	if (mesh == nullptr || material == nullptr)
		return;
	
	FishEngine::SetModelMatrix(matrix);
	FishEngine::BindMaterial(material);
	FishEngine::Draw(mesh, submeshID);
	
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
	
	Viewport viewport;
	viewport.originX = 0;
	viewport.originY = 0;
//	viewport.width = Screen::width*2;		// TODO
//	viewport.height = Screen::height*2;		// TODO
	viewport.znear = 0;
	viewport.zfar = 1;
	
	
#if USE_QUEUE
	auto pbr = Material::pbrMetallicRoughness->GetShader();
	
	auto input = mainApp->GetEditorScene()->input;
	
#if 0
	//viewport.originX = input->GetMousePosition_Unity().x * 2;	// TODO
	//viewport.originY = input->GetMousePosition_Unity().y * 2;	// TODO
	viewport.originX = 0;
	viewport.originY = 0;
	viewport.width = 5;
	viewport.height = 5;
	SetViewport(viewport);
	//if (mainApp->GetEditorScene()->input->IsButtonPressed(KeyCode::MouseLeftButton))
	{
		BeginPass(g_PickingRPS, g_PickingRT, g_PickingRTDepth);
		g_PickingMaterial->SetVector("u_color", Vector4::one);
		Vector4 id = Vector4::one;
		FishEngine::BindMaterial(g_PickingMaterial);
		for (int i = 0; i < g_RenderQueue.size(); ++i)
		{
			auto& ro = g_RenderQueue[i];
			if (pbr == ro.material->GetShader())
			{
				id = Vector4::one * ((float)i/255.0f);
				g_PickingMaterial->SetVector("u_color", id);
				FishEngine::SetModelMatrix(ro.localToWorld);
				FishEngine::UpdatePerDrawUniforms(ro.material);
				FishEngine::Draw(ro.mesh, ro.submeshID);
			}
		}
	}
#endif
	
	viewport.originX = 0;
	viewport.originY = 0;
	viewport.width = Screen::width*2;		// TODO
	viewport.height = Screen::height*2;		// TODO
	SetViewport(viewport);
	
//	FishEngine::Blit(g_MainColorRT, 0, 0, g_PickingRT, 0, 0, 5, 5);
	
	
	Material* last = nullptr;
	BeginPass(g_rps, true);
	for (auto& ro : g_RenderQueue)
	{
//		DrawMesh2(ro.mesh, ro.localToWorld, ro.material, ro.submeshID);
		if (pbr == ro.material->GetShader())
		{
			FishEngine::SetModelMatrix(ro.localToWorld);
			if (last == nullptr)
			{
				BindMaterial(ro.material);
			}
			else if (last == ro.material)
			{
				UpdatePerDrawUniforms(ro.material);
			}
			else
			{
				UpdatePerDrawUniforms(ro.material);
				UpdateNonInternalUniforms(ro.material);
			}
			last = ro.material;
			FishEngine::Draw(ro.mesh, ro.submeshID);
		}
	}
//	EndPass();
#endif
}
