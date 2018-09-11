#include "FishEngine/Systems/RenderSystem.hpp"
#include <FishEngine/Components/Transform.hpp>
#include "FishEngine/Components/Camera.hpp"
#include "FishEngine/Components/Light.hpp"
#include "FishEngine/Components/Renderable.hpp"
#include "FishEngine/GameApp.hpp"
#include "FishEngine/Graphics.hpp"
#include "FishEngine/Material.hpp"
#include <FishEngine/Mesh.hpp>
#include <FishEngine/Gizmos.hpp>
#include <FishEngine/Systems/SelectionSystem.hpp>

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
	init.type = bgfx::RendererType::Enum::Metal;
	init.resolution.width = 800;
	init.resolution.height = 600;
//	init.resolution.reset = BGFX_RESET_VSYNC | BGFX_RESET_MSAA_X2;
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
	
	imguiCreate();
}

void RenderSystem::Start()
{
	
}

#include <FishEngine/Components/SingletonInput.hpp>
#include <FishEngine/Screen.hpp>

Transform* selected = nullptr;

void HierarchyNode(Transform* t)
{
	auto name = t->m_GameObject->m_Name;
	if (name.empty())
		name = "go:" + std::to_string(t->entityID);
#if 1
	//ImGui::PushID(t);
	ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
	if (selected == t)
		node_flags |= ImGuiTreeNodeFlags_Selected;
	bool isLeaf = t->GetChildren().empty();
	if (t->GetChildren().empty())
		node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
	node_flags |= ImGuiTreeNodeFlags_DefaultOpen;
	bool node_open = ImGui::TreeNodeEx((void*)t, node_flags, name.c_str());
	if (ImGui::IsItemClicked())
		selected = t;
	if (!isLeaf && node_open)
	{
		for (auto c : t->GetChildren())
		{
			HierarchyNode(c);
		}
		ImGui::TreePop();
	}
	//ImGui::PopID();
#else
	if (ImGui::TreeNode(name.c_str()))
	{
		for (auto c : t->GetChildren())
		{
			HierarchyNode(c);
		}
		ImGui::TreePop();
	}
	
#endif
};


void RenderSystem::Draw()
{
	bgfx::touch(0);
//	bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL);
	Camera* camera = m_Scene->FindComponent<Camera>();
	if (camera == nullptr)
		return;
	float cameraPos[4];
	Matrix4x4 view;
	{
		auto go = camera->m_GameObject;
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
		Vector3 lightDir = -light->m_GameObject->GetTransform()->GetForward();
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

#if 1
	// CPU skinning
	Matrix4x4 u_jointMatrix[256];
	m_Scene->ForEach<Renderable>([&u_jointMatrix](ECS::GameObject* go, Renderable* r)
	{
		if (r == nullptr || !r->m_Enabled)
		{
			return;
		}
		auto mesh = r->mesh;
		if (mesh != nullptr && mesh->IsSkinned())
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
#endif

#if 1
	m_Scene->ForEach<Renderable>([](ECS::GameObject* go, Renderable* rend)
	{
		if (rend == nullptr || !rend->m_Enabled || rend->mesh == nullptr)
			return;
		auto& mtx = go->GetTransform()->GetLocalToWorldMatrix();
		
		if (rend->m_Materials.empty())
		{
			// render with error material
			Graphics::DrawMesh(rend->mesh, mtx, Material::Error);
		}
		else
		{
			if (rend->m_Materials.size() != rend->mesh->m_SubMeshCount)
			{
				abort();	// mismatch
				Graphics::DrawMesh(rend->mesh, mtx, rend->m_Materials[0]);
			}
			else
			{
				if (rend->m_Materials.size() == 1)
				{
					Graphics::DrawMesh(rend->mesh, mtx, rend->m_Materials[0]);
				}
				else
				{
					for (int i = 0; i < rend->m_Materials.size(); ++i)
					{
						Graphics::DrawMesh(rend->mesh, mtx, rend->m_Materials[i], 0, i);
					}
				}
			}
		}
	});
#endif
	
	Gizmos::__Draw();
	
#if 1
	const int hierarchy_width = 250;
	const int inspector_width = 250;
	//printf("============here==========\n\n");
	auto input = m_Scene->GetSingletonComponent<SingletonInput>();
	Vector2 mousePos = input->GetMousePosition();
	mousePos.x *= Screen::width;
	mousePos.y *= Screen::height;
	auto mouseBtns =
	(input->IsButtonHeld(KeyCode::MouseLeftButton) ? IMGUI_MBUT_LEFT : 0) |
	(input->IsButtonHeld(KeyCode::MouseRightButton) ? IMGUI_MBUT_RIGHT : 0) |
	(input->IsButtonHeld(KeyCode::MouseMiddleButton) ? IMGUI_MBUT_MIDDLE : 0);
	selected = m_Scene->GetSystem<SelectionSystem>()->selected->GetTransform();
	imguiBeginFrame(mousePos.x, mousePos.y, mouseBtns, input->GetAxis(Axis::MouseScrollWheel), Screen::width, Screen::height);
	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(ImVec2(hierarchy_width, Screen::height));
	ImGui::Begin("Hierarchy", NULL, 0);
	for (auto t : m_Scene->m_RootTransforms)
	{
		HierarchyNode(t);
	}
	ImGui::End();
	
	m_Scene->GetSystem<SelectionSystem>()->selected = selected->m_GameObject;
	
	ImGui::SetNextWindowPos(ImVec2(Screen::width-inspector_width, 0));
	ImGui::SetNextWindowSize(ImVec2(inspector_width, Screen::height));
	ImGui::Begin("Inspector", NULL, 0);
	if (selected != nullptr)
	{
		for (auto comp : selected->m_GameObject->GetComponents())
		{
			if (ImGui::CollapsingHeader(comp->GetTypeIndex().name(), ImGuiTreeNodeFlags_DefaultOpen))
			{
				if (comp->Is<Transform>())
				{
					auto t = comp->As<Transform>();
					Vector3 pos = t->GetLocalPosition();
					if (ImGui::InputFloat3("Position", pos.data()))
					{
						t->SetLocalPosition(pos);
					}
					
					auto r = t->GetLocalEulerAngles();
					if (ImGui::InputFloat3("Rotation", r.data()))
					{
						t->SetLocalEulerAngles(r);
					}
					
					auto s = t->GetLocalScale();
					if (ImGui::InputFloat3("Scale", s.data()))
					{
						t->SetLocalEulerAngles(s);
					}
					
					auto q = t->GetLocalRotation();
					if (ImGui::InputFloat4("Rotation2", (float*)&q))
					{
						t->SetLocalRotation(q);
					}
				}
				else if (comp->Is<Renderable>())
				{
					auto r = comp->As<Renderable>();
					ImGui::Checkbox("enabled", &r->m_Enabled);
				}
			}
		}
	}
	ImGui::End();
	
	int HEIGHT = 200;
	ImGui::SetNextWindowPos(ImVec2(inspector_width, Screen::height-HEIGHT));
	ImGui::SetNextWindowSize(ImVec2(Screen::width-inspector_width-hierarchy_width, HEIGHT));
	ImGui::Begin("Systems", NULL, 0);
	for (ISystem* s : m_Scene->GetSystems())
	{
		ImGui::PushID((void*)s);
		if (ImGui::CollapsingHeader(s->GetTypeIndex().name(), ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::Checkbox("enabled", &s->m_Enabled);
		}
		ImGui::PopID();
	}
	ImGui::End();
	
	imguiEndFrame();
#endif
}


void RenderSystem::Resize(int width, int height)
{
//	bgfx::reset(width*2, height*2, BGFX_RESET_VSYNC | BGFX_RESET_HIDPI);
	bgfx::reset(width, height, BGFX_RESET_VSYNC);
}
