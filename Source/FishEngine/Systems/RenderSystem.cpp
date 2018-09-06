#include "FishEngine/Systems/RenderSystem.hpp"
#include <FishEngine/Components/Transform.hpp>
#include "FishEngine/Components/Camera.hpp"
#include "FishEngine/Components/Light.hpp"
#include "FishEngine/Components/Renderable.hpp"
#include "FishEngine/GameApp.hpp"
#include "FishEngine/Graphics.hpp"
#include "FishEngine/Material.hpp"
#include <FishEngine/Mesh.hpp>

#include <bx/math.h>

#include <imgui/imgui.h>

SingletonRenderState::SingletonRenderState()
{
	m_UniformLightDir = bgfx::createUniform("lightDir", bgfx::UniformType::Vec4);
	m_UniformCameraPos = bgfx::createUniform("CameraPos", bgfx::UniformType::Vec4);
	m_UniformJointMatrix = bgfx::createUniform("u_jontMatrix", bgfx::UniformType::Vec4, 128);
}


void RenderSystem::OnAdded()
{
	bgfx::Init init;
	init.type = bgfx::RendererType::Enum::OpenGL;
	init.resolution.width = 640;
	init.resolution.height = 480;
	init.resolution.reset = BGFX_RESET_VSYNC | BGFX_RESET_MSAA_X2;
	bgfx::init(init);
	//	bgfx::setDebug(BGFX_DEBUG_STATS);
	bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x303030ff, 1.0f, 0);

	auto state = m_Scene->AddSingletonComponent<SingletonRenderState>();

	state->m_State = 0
		| BGFX_STATE_WRITE_RGB
		| BGFX_STATE_WRITE_A
		| BGFX_STATE_WRITE_Z
		| BGFX_STATE_DEPTH_TEST_LESS
		| BGFX_STATE_CULL_CCW
		| BGFX_STATE_MSAA
		;
	
	imguiCreate();
}

void RenderSystem::Start()
{
	
}

#include <FishEngine/Components/SingletonInput.hpp>
#include <FishEngine/Screen.hpp>

void RenderSystem::Draw()
{
	Camera* camera = m_Scene->FindComponent<Camera>();
	if (camera == nullptr)
		return;
	float cameraPos[4];
	Matrix4x4 view;
	{
		auto go = m_Scene->GetGameObjectByID(camera->entityID);
		auto p = go->GetTransform()->GetPosition();
		cameraPos[0] = p.x;
		cameraPos[1] = p.y;
		cameraPos[2] = p.z;
		cameraPos[3] = 1.0f;
		view = go->GetTransform()->GetLocalToWorldMatrix().inverse().transpose();
	}

	auto renderState = m_Scene->GetSingletonComponent<SingletonRenderState>();

	bgfx::setUniform(renderState->m_UniformCameraPos, cameraPos);

	Light* light = m_Scene->FindComponent<Light>();
	if (light != nullptr)
	{
		Vector3 lightDir = -m_Scene->GetGameObjectByID(light->entityID)->GetTransform()->GetForward();
		Vector3 d = Vector3::Normalize(lightDir);
		bgfx::setUniform(renderState->m_UniformLightDir, &d);
	}
	
	
	float width = (float)GameApp::GetMainApp()->GetWidth();
	float height = (float)GameApp::GetMainApp()->GetHeight();
	float ratio = width / height;
	float proj[16];
	bx::mtxProj(proj, 60.0f, ratio, camera->m_NearClipPlane, camera->m_FarClipPlane, bgfx::getCaps()->homogeneousDepth);
	bgfx::setViewTransform(0, view.data(), proj);
	bgfx::setViewTransform(1, view.data(), proj);
	
	// draw skybox first
	auto old_state = renderState->m_State;
	renderState->m_State = BGFX_STATE_CULL_CW | BGFX_STATE_WRITE_MASK | BGFX_STATE_DEPTH_TEST_LESS;
	m_Scene->ForEach<Skybox>([cameraPos](ECS::GameObject* go, Skybox* skybox)
	{
		auto mat = Matrix4x4::TRS(Vector3(cameraPos[0], cameraPos[1], cameraPos[2]), Quaternion::identity, Vector3::one*100);
		Graphics::DrawMesh(Mesh::Sphere, mat, skybox->m_skyboxMaterial);
	});
	renderState->m_State = old_state;

	// CPU skinning
	Matrix4x4 u_jointMatrix[256];
	m_Scene->ForEach<Renderable>([&u_jointMatrix](ECS::GameObject* go, Renderable* r)
	{
		auto mesh = r->mesh;
		if (mesh->IsSkinned())
		{
			auto worldToObject = r->skin->root->GetTransform()->GetWorldToLocalMatrix();
			//auto worldToObject = Matrix4x4::identity;
			for (int i = 0; i < r->skin->joints.size(); ++i)
			{
				auto bone = r->skin->joints[i]->GetTransform();
				auto& bindpose = r->skin->inverseBindMatrices[i];
				u_jointMatrix[i] = worldToObject * bone->GetLocalToWorldMatrix() * bindpose;
			}
			
			
			mesh->m_DynamicVertices = mesh->m_Vertices;
			for (int i = 0; i < mesh->m_Vertices.size(); ++i)
			{
				const Vector4& a_weight = mesh->weights[i];
				const auto& a_joint = mesh->joints[i];
				Matrix4x4 skinMatrix = u_jointMatrix[a_joint.x] * a_weight.x;
				//if (a_weight.y > 0)
					skinMatrix += u_jointMatrix[a_joint.y] * a_weight.y;
				//if (a_weight.z > 0)
					skinMatrix += u_jointMatrix[a_joint.z] * a_weight.z;
				//if (a_weight.w > 0)
					skinMatrix += u_jointMatrix[a_joint.w] * a_weight.w;
				auto& dv = mesh->m_DynamicVertices[i];
				auto& v = mesh->m_Vertices[i];
				dv.position = skinMatrix.MultiplyPoint3x4(v.position);
				dv.normal = skinMatrix.MultiplyVector(v.normal);
			}
			
			auto mem = bgfx::makeRef(mesh->m_DynamicVertices.data(), sizeof(PUNTVertex)*mesh->m_Vertices.size());
			if (!bgfx::isValid(mesh->m_DynamicVertexBuffer))
			{
				mesh->m_DynamicVertexBuffer = bgfx::createDynamicVertexBuffer(mem, PUNTVertex::ms_decl);
			}
			else
			{
				bgfx::update(mesh->m_DynamicVertexBuffer, 0, mem);
			}
		}
	});

	m_Scene->ForEach<Renderable>([](ECS::GameObject* go, Renderable* rend)
	{
		auto& mtx = go->GetTransform()->GetLocalToWorldMatrix();
		Graphics::DrawMesh(rend->mesh, mtx, rend->material);
	});
	
#if 0
	//printf("============here==========\n\n");
	auto input = m_Scene->GetSingletonComponent<SingletonInput>();
	Vector2 mousePos = input->GetMousePosition();
	auto mouseBtns =
	(input->IsButtonPressed(KeyCode::MouseLeftButton) ? IMGUI_MBUT_LEFT : 0) |
	(input->IsButtonPressed(KeyCode::MouseRightButton) ? IMGUI_MBUT_RIGHT : 0) |
	(input->IsButtonPressed(KeyCode::MouseMiddleButton) ? IMGUI_MBUT_MIDDLE : 0);
	
	ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(100, 100), ImGuiCond_FirstUseEver);
	imguiBeginFrame(mousePos.x, mousePos.y, mouseBtns, input->GetAxis(Axis::MouseScrollWheel), Screen::width, Screen::height);
	ImGui::Begin("Hierarchy", NULL, 0);
	ImGui::Text("Environment light:");
	ImGui::End();
	imguiEndFrame();
#endif
}


void RenderSystem::Resize(int width, int height)
{
//	bgfx::reset(width*2, height*2, BGFX_RESET_VSYNC | BGFX_RESET_HIDPI);
	bgfx::reset(width, height, BGFX_RESET_VSYNC);
}
