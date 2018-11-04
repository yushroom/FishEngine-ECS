#include <FishEngine/Systems/RenderSystem.hpp>
#include <FishEngine/Components/Transform.hpp>
#include <FishEngine/Components/Camera.hpp>
#include <FishEngine/Components/Light.hpp>
#include <FishEngine/Components/Renderable.hpp>
#include <FishEngine/Components/SingletonRenderState.hpp>
#include <FishEngine/Graphics.hpp>
#include <FishEngine/Material.hpp>
#include <FishEngine/Mesh.hpp>
#include <FishEngine/Gizmos.hpp>
#include <FishEngine/ECS/Scene.hpp>
//#include <FishEngine/Systems/SelectionSystem.hpp>
#include <FishEngine/bgfxHelper.hpp>
#include <FishEngine/Render/RenderViewType.hpp>
#include <FishEngine/Components/Animator.hpp>
#include <FishEngine/Render/CameraFrustumCulling.hpp>

using namespace FishEngine;

void RenderSystem::OnAdded()
{
#if 0
	bgfx::Init init;
#if FISHENGINE_PLATFORM_APPLE
	init.type = bgfx::RendererType::Enum::OpenGL;
#else
	init.type = bgfx::RendererType::Enum::Direct3D12;
#endif
	init.resolution.width = 800;
	init.resolution.height = 600;
	init.resolution.reset = BGFX_RESET_VSYNC | BGFX_RESET_MSAA_X8;
	init.resolution.reset = BGFX_RESET_VSYNC;
	bgfx::init(init);
//	bgfx::setDebug(BGFX_DEBUG_STATS);
	bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL, 0x303030ff, 1.0f, 0);

	auto state = m_Scene->AddSingletonComponent<SingletonRenderState>();

	state->m_State = 0
		| BGFX_STATE_WRITE_RGB
		| BGFX_STATE_WRITE_A
		| BGFX_STATE_WRITE_Z
		| BGFX_STATE_DEPTH_TEST_LESS
		| BGFX_STATE_CULL_CCW
//		| BGFX_STATE_MSAA
		;
	
	//assert(bgfx::getCaps()->supported & BGFX_CAPS_COMPUTE);
#endif
	
	auto state = m_Scene->AddSingletonComponent<SingletonRenderState>();
}

void RenderSystem::Start()
{
//	bgfx::setViewClear((bgfx::ViewId)RenderViewType::Scene,
//					   BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL,
//					   0x000000ff, 1.0f, 0);
//	bgfx::setViewClear((bgfx::ViewId)RenderViewType::UI,
//					   BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL,
//					   0x000000ff, 1.0f, 0);
//	bgfx::setViewClear((bgfx::ViewId)RenderViewType::Editor,
//					   BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL,
//					   0x000000ff, 1.0f, 0);
//	bgfx::setViewClear((bgfx::ViewId)RenderViewType::SceneGizmos,
//					   BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL,
//					   0x000000ff, 1.0f, 0);
//	bgfx::setViewClear((bgfx::ViewId)RenderViewType::Picking,
//					   BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL,
//					   0x000000ff, 1.0f, 0);
}

#include <FishEngine/Components/SingletonInput.hpp>
#include <FishEngine/Screen.hpp>

Matrix4x4 u_jointMatrix[256];

void RenderSystem::Draw()
{
//	bgfx::touch((bgfx::ViewId)RenderViewType::SceneGizmos);
	Camera* camera = Camera::GetEditorCamera();
	if (camera == nullptr)
		return;
	Vector4 cameraPos(camera->GetGameObject()->GetTransform()->GetPosition(), 1.0f);
	
	FishEngine::SetCamera(camera);

	auto renderState = m_Scene->GetSingletonComponent<SingletonRenderState>();

//	bgfx::setUniform(renderState->m_UniformCameraPos, cameraPos.data());

	Light* light = m_Scene->FindComponent<Light>();
	if (light != nullptr)
	{
		Vector3 lightDir = -light->m_GameObject->GetTransform()->GetForward();
		Vector3 d = Vector3::Normalize(lightDir);
//		bgfx::setUniform(renderState->m_UniformLightDir, &d);
	}
	
	FishEngine::SetLight(light);
	
	
	float width = (float)Screen::width;
	float height = (float)Screen::height;
	float aspectRatio = width / height;
	Matrix4x4 viewMat = camera->GetWorldToCameraMatrix();
	Matrix4x4 projMat = camera->GetProjectionMatrix();
//	Matrix4x4 viewT = viewMat.transpose();
//	Matrix4x4 projT = projMat.transpose();
//	bgfx::setViewTransform((bgfx::ViewId)RenderViewType::Scene, viewT.data(), projT.data());
//	bgfx::setViewTransform((bgfx::ViewId)RenderViewType::SceneGizmos, viewT.data(), projT.data());
	SetViewProjectionMatrix(viewMat, projMat);
	
	// draw skybox first
	auto old_state = renderState->m_State;
//	renderState->m_State = BGFX_STATE_CULL_CW | BGFX_STATE_WRITE_MASK | BGFX_STATE_DEPTH_TEST_LESS;
	m_Scene->ForEach<Skybox>([cameraPos](GameObject* go, Skybox* skybox)
	{
		auto mat = Matrix4x4::TRS(Vector3(cameraPos[0], cameraPos[1], cameraPos[2]), Quaternion::identity, Vector3::one*100);
		Graphics::DrawMesh(Mesh::Sphere, mat, skybox->m_SkyboxMaterial);
	});
	renderState->m_State = old_state;

#if 1
	// CPU skinning
	
	m_Scene->ForEach<Renderable>([](GameObject* go, Renderable* r)
	{
		if (r == nullptr || !r->m_Enabled)
		{
			return;
		}
		auto mesh = r->m_Mesh;
		if (mesh != nullptr && mesh->IsSkinned())
		{
			auto skin = r->m_Skin;
			const auto worldToObject = r->GetGameObject()->GetTransform()->GetWorldToLocalMatrix();
			for (int i = 0; i < skin->joints.size(); ++i)
			{
				auto bone = skin->joints[i]->GetTransform();
				auto& bindpose = skin->inverseBindMatrices[i];
				u_jointMatrix[i] = worldToObject * bone->GetLocalToWorldMatrix() * bindpose;
			}
			
			
			if (mesh->m_DynamicVertices.size() != mesh->m_Vertices.size())
			{
//				mesh->m_DynamicVertices.resize(mesh->m_Vertices.size());
//				memcpy(mesh->m_DynamicVertices.data(), mesh->m_Vertices.data(), sizeof(mesh->m_Vertices));
				mesh->m_DynamicVertices = mesh->m_Vertices;
			}
			for (int i = 0; i < mesh->m_Vertices.size(); ++i)
			{
				const Vector4& a_weight = mesh->weights[i];
				const auto& a_joint = mesh->joints[i];
				Matrix4x4 skinMatrix = u_jointMatrix[a_joint.x] * a_weight.x;
				if (a_weight.y > 0)
					skinMatrix += u_jointMatrix[a_joint.y] * a_weight.y;
				if (a_weight.z > 0)
					skinMatrix += u_jointMatrix[a_joint.z] * a_weight.z;
				if (a_weight.w > 0)
					skinMatrix += u_jointMatrix[a_joint.w] * a_weight.w;
				auto& dv = mesh->m_DynamicVertices[i];
				auto& v = mesh->m_Vertices[i];
				dv.position = skinMatrix.MultiplyPoint(v.position);
				dv.normal = skinMatrix.MultiplyVector(v.normal);
			}
			
			
#if 0
			auto mem = bgfxHelper::MakeRef(mesh->m_DynamicVertices);
			if (!bgfx::isValid(mesh->m_DynamicVertexBuffer))
			{
				mesh->m_DynamicVertexBuffer = bgfx::createDynamicVertexBuffer(mem, PUNTVertex::ms_decl);
			}
			else
			{
				bgfx::update(mesh->m_DynamicVertexBuffer, 0, mem);
			}
#endif
		}
	});
#endif
	
	// test frustum culling
//	auto gameCamera = Camera::GetMainCamera();
	CameraFrustumCulling culling(camera, aspectRatio);


#if 1
	m_Scene->ForEach<Renderable>([&culling, renderState](GameObject* go, Renderable* rend)
	{
		if (rend == nullptr || !rend->m_Enabled || rend->m_Mesh == nullptr)
			return;

		auto& modelMat = go->GetTransform()->GetLocalToWorldMatrix();


		bool visiable = true;
		if (renderState->m_EnableFrustumCulling)
		{
			if (!rend->m_Mesh->IsSkinned())
				visiable = culling.Visiable(rend->m_Mesh, modelMat);
		}

#if 0
		if (visiable)
			Gizmos::color = Vector4(0, 1, 0, 1);
		else
			Gizmos::color = Vector4(0, 0, 1, 1);
		Gizmos::matrix = modelMat;
		Gizmos::DrawBounds(rend->mesh->bounds);
#endif
		
		if (!visiable)
			return;
		
		if (rend->m_Materials.empty())
		{
			// render with error material
			Graphics::DrawMesh(rend->m_Mesh, modelMat, Material::ErrorMaterial);
		}
		else
		{
			if (rend->m_Materials.size() != rend->m_Mesh->m_SubMeshCount)
			{
				abort();	// mismatch
				Graphics::DrawMesh(rend->m_Mesh, modelMat, rend->m_Materials[0]);
			}
			else
			{
				if (rend->m_Materials.size() == 1)
				{
					Graphics::DrawMesh(rend->m_Mesh, modelMat, rend->m_Materials[0]);
				}
				else
				{
					for (int i = 0; i < rend->m_Materials.size(); ++i)
					{
						Graphics::DrawMesh(rend->m_Mesh, modelMat, rend->m_Materials[i], i);
					}
				}
			}
		}
	});
#endif
	
	Gizmos::__Draw();
}


void RenderSystem::Resize(int width, int height)
{
//	bgfx::reset(width*2, height*2, BGFX_RESET_VSYNC | BGFX_RESET_HIDPI);
//	bgfx::reset(width, height, BGFX_RESET_VSYNC);
}
