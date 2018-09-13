#include <FishEngine/Systems/EditorSystem.hpp>
#include <FishEngine/Systems/SelectionSystem.hpp>
#include <FishEngine/Screen.hpp>
#include <FishEngine/Components/Transform.hpp>
#include <FishEngine/Components/SingletonInput.hpp>
#include <FishEngine/Components/Renderable.hpp>
#include <FishEngine/Components/Animator.hpp>
#include <FishEngine/Material.hpp>
#include <FishEngine/Gizmos.hpp>

#include <FishEngine/Components/Camera.hpp>
#include <FishEngine/Components/FreeCamera.hpp>
#include <FishEngine/Systems/FreeCameraSystem.hpp>

#include <imgui/imgui.h>

constexpr int imgui_window_flags = 0
| ImGuiWindowFlags_NoResize
| ImGuiWindowFlags_NoMove
| ImGuiWindowFlags_NoCollapse
| ImGuiWindowFlags_NoSavedSettings
| ImGuiWindowFlags_NoFocusOnAppearing
;

Transform* selected = nullptr;

void HierarchyNode(Transform* t)
{
	auto name = t->m_GameObject->m_Name;
	if (name.empty())
		name = "go:" + std::to_string(t->entityID);

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
};


void EditorSystem::OnAdded()
{
	imguiCreate();

	auto go = m_Scene->CreateGameObject();
	go->m_Name = "EditorCamera";
	auto cam = m_Scene->GameObjectAddComponent<Camera>(go);
	cam->m_Type = CameraType::Editor;
	go->GetTransform()->SetLocalPosition(0, 0, -10);
	m_Scene->GameObjectAddComponent<FreeCamera>(go);
	
	m_Scene->AddSystem<FreeCameraSystem>();
}


void EditorSystem::Update()
{
	constexpr float hierarchy_width = 250;
	constexpr float inspector_width = 250;
	//printf("============here==========\n\n");
	auto input = m_Scene->GetSingletonComponent<SingletonInput>();
	if (input->IsButtonHeld(KeyCode::F1))
	{
		return;
	}

	auto selectionSystem = m_Scene->GetSystem<SelectionSystem>();
	assert(selectionSystem != nullptr);
	Vector2 mousePos = input->GetMousePosition();
	mousePos.x *= Screen::width;
	mousePos.y *= Screen::height;
	auto mouseBtns =
		(input->IsButtonHeld(KeyCode::MouseLeftButton) ? IMGUI_MBUT_LEFT : 0) |
		(input->IsButtonHeld(KeyCode::MouseRightButton) ? IMGUI_MBUT_RIGHT : 0) |
		(input->IsButtonHeld(KeyCode::MouseMiddleButton) ? IMGUI_MBUT_MIDDLE : 0);
	selected = selectionSystem->selected->GetTransform();
	imguiBeginFrame((int)mousePos.x, (int)mousePos.y, mouseBtns, (int)input->GetAxis(Axis::MouseScrollWheel), Screen::width, Screen::height);

	MainMenu();

	constexpr float main_menu_bar_height = 24;

	ImGui::SetNextWindowPos(ImVec2(0.0f, main_menu_bar_height));
	ImGui::SetNextWindowSize(ImVec2(hierarchy_width, (float)Screen::height));
	Hierarchy();
	selectionSystem->selected = selected->m_GameObject;

	ImGui::SetNextWindowPos(ImVec2((float)Screen::width - inspector_width, main_menu_bar_height));
	ImGui::SetNextWindowSize(ImVec2(inspector_width, (float)Screen::height));
	Inspector();

#if 0
	ImGui::SetNextWindowPos(ImVec2(Screen::width - inspector_width, Screen::height / 2));
	ImGui::SetNextWindowSize(ImVec2(inspector_width, Screen::height / 2));
	ImGui::Begin("Systems", NULL, imgui_window_flags);
	for (ISystem* s : m_Scene->GetSystems())
	{
		ImGui::PushID((void*)s);
		if (ImGui::CollapsingHeader(s->GetClassName(), ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::Checkbox("enabled", &s->m_Enabled);
		}
		ImGui::PopID();
	}
	ImGui::End();
#endif

	imguiEndFrame();
}

void EditorSystem::MainMenu()
{
	ImGui::BeginMainMenuBar();
	if (ImGui::BeginMenu("File"))
	{
		ImGui::MenuItem("New Scene", "Ctrl+N");
		ImGui::MenuItem("Open Scene", "Ctrl+O");
		ImGui::Separator();
		ImGui::MenuItem("Save Scene", "Ctrl+O");
		ImGui::MenuItem("Save Scene As...", "Ctrl+Shift+S");
		ImGui::Separator();
		if (ImGui::BeginMenu("Open file..."))
		{
			//ImGui::MenuItem();
			ImGui::EndMenu();
		}
		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("Systems"))
	{
		for (ISystem* s : m_Scene->GetSystems())
		{
			ImGui::Checkbox(s->GetClassName(), &s->m_Enabled);
			//ImGui::MenuItem(s->GetClassName(), nullptr, &s->m_Enabled);
		}
		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("Gizmos"))
	{
		ImGui::Checkbox("Enabled depth test", &Gizmos::s_EnableDepthTest);
		ImGui::EndMenu();
	}
	ImGui::EndMainMenuBar();
}

void EditorSystem::Hierarchy()
{
	ImGui::Begin("Hierarchy", NULL, imgui_window_flags);
	for (auto t : m_Scene->m_RootTransforms)
	{
		HierarchyNode(t);
	}
	ImGui::End();
}

void EditorSystem::Inspector()
{
	ImGui::Begin("Inspector", NULL, imgui_window_flags);
	if (selected != nullptr)
	{
		for (auto comp : selected->m_GameObject->GetComponents())
		{
			if (ImGui::CollapsingHeader(comp->GetClassName(), ImGuiTreeNodeFlags_DefaultOpen))
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

					bool skinned = r->skin != nullptr;
					//ImGui::Checkbox("skinned", &skinned);
					if (skinned)
					{
						auto skin = r->skin;
						ImGui::Text("root bone: %s", skin->root->m_Name.c_str());
					}

					if (ImGui::CollapsingHeader("Materials", ImGuiTreeNodeFlags_DefaultOpen))
					{
						for (int i = 0; i < r->m_Materials.size(); ++i)
						{
							auto material = r->m_Materials[i];
							auto header = "Material" + std::to_string(i);
							if (ImGui::CollapsingHeader(header.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
							{
								ImGui::LabelText("name", material->name.c_str());
								for (auto& p : material->m_UniformInfos)
								{
									auto name = p.first.c_str();
									auto& info = p.second.second;
									if (info.type == bgfx::UniformType::Vec4)
									{
										if (material->m_MaterialProperties.vec4s.find(p.first) != material->m_MaterialProperties.vec4s.end())
										{
											auto& v = material->m_MaterialProperties.vec4s[p.first];
											ImGui::ColorEdit4(name, v.data());
										}
									}
									else if (info.type == bgfx::UniformType::Int1)
									{
										if (material->m_MaterialProperties.textures.find(p.first) != material->m_MaterialProperties.textures.end())
										{
											auto& v = material->m_MaterialProperties.textures[p.first];
											ImGui::Image(v, ImVec2(64, 64));
										}
									}
								}
							}
						}
					}
				}
				else if (comp->Is<Animator>())
				{
					auto a = comp->As<Animator>();
					ImGui::InputInt("current clip", &a->m_CurrentClipIndex);
					ImGui::InputFloat("local timer", &a->m_LocalTimer);
					auto clip = a->GetCurrentClip();
					if (clip != nullptr)
					{
						ImGui::Text("current clip:");
						//ImGui::Text("  root bone: %s", );
					}
				}
				else if (comp->Is<Camera>())
				{
					auto c = comp->As<Camera>();
					int fov = (int)c->m_FOV;
					ImGui::SliderInt("Field of View", &fov, 1, 179);
					c->m_FOV = (float)fov;
					ImGui::SliderFloat("Near", &c->m_NearClipPlane, 0.01f, c->m_FarClipPlane);
					ImGui::SliderFloat("Far", &c->m_FarClipPlane, 1.f, 1000.f);
				}
			}
		}
	}
	ImGui::End();
}
