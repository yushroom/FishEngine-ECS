#include <FishEditor/Systems/EditorSystem.hpp>
#include <FishEngine/Material.hpp>
#include <FishEngine/Gizmos.hpp>
#include <FishEngine/Mesh.hpp>
#include <FishEngine/Components/Transform.hpp>
#include <FishEngine/Components/SingletonInput.hpp>
#include <FishEngine/Components/Renderable.hpp>
#include <FishEngine/Components/Animator.hpp>
#include <FishEngine/Components/Camera.hpp>

#include <FishEditor/Components/FreeCamera.hpp>
#include <FishEditor/Components/SingletonSelection.hpp>
#include <FishEditor/Systems/FreeCameraSystem.hpp>
#include <FishEditor/Systems/SceneViewSystem.hpp>
#include <FishEngine/Assets.hpp>

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <examples/imgui_impl_dx12.h>
#include <examples/imgui_impl_glfw.h>

float main_menu_bar_height = 24;
constexpr float main_tool_bar_height = 40;

using namespace FishEditor;
using namespace FishEngine;

ImFont* s_font = nullptr;

void SetupImGuiStyle( bool bStyleDark_, float alpha_  )
{
	ImGuiStyle& style = ImGui::GetStyle();
	
	// light style from Pacôme Danhiez (user itamago) https://github.com/ocornut/imgui/pull/511#issuecomment-175719267
	style.Alpha = 1.0f;
	style.FrameRounding = 3.0f;
	style.WindowRounding = 0.0f;
	style.Colors[ImGuiCol_Text]                  = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_TextDisabled]          = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
	style.Colors[ImGuiCol_WindowBg]              = ImVec4(0.94f, 0.94f, 0.94f, 0.94f);
//	style.Colors[ImGuiCol_ChildWindowBg]         = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	style.Colors[ImGuiCol_PopupBg]               = ImVec4(1.00f, 1.00f, 1.00f, 0.94f);
	style.Colors[ImGuiCol_Border]                = ImVec4(0.00f, 0.00f, 0.00f, 0.39f);
	style.Colors[ImGuiCol_BorderShadow]          = ImVec4(1.00f, 1.00f, 1.00f, 0.10f);
	style.Colors[ImGuiCol_FrameBg]               = ImVec4(1.00f, 1.00f, 1.00f, 0.94f);
	style.Colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
	style.Colors[ImGuiCol_FrameBgActive]         = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
	style.Colors[ImGuiCol_TitleBg]               = ImVec4(0.96f, 0.96f, 0.96f, 1.00f);
	style.Colors[ImGuiCol_TitleBgCollapsed]      = ImVec4(1.00f, 1.00f, 1.00f, 0.51f);
	style.Colors[ImGuiCol_TitleBgActive]         = ImVec4(0.82f, 0.82f, 0.82f, 1.00f);
	style.Colors[ImGuiCol_MenuBarBg]             = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.98f, 0.98f, 0.98f, 0.53f);
	style.Colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.69f, 0.69f, 0.69f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.59f, 0.59f, 0.59f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(0.49f, 0.49f, 0.49f, 1.00f);
//	style.Colors[ImGuiCol_ComboBg]               = ImVec4(0.86f, 0.86f, 0.86f, 0.99f);
	style.Colors[ImGuiCol_CheckMark]             = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	style.Colors[ImGuiCol_SliderGrab]            = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
	style.Colors[ImGuiCol_SliderGrabActive]      = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	style.Colors[ImGuiCol_Button]                = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
	style.Colors[ImGuiCol_ButtonHovered]         = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	style.Colors[ImGuiCol_ButtonActive]          = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
	style.Colors[ImGuiCol_Header]                = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
	style.Colors[ImGuiCol_HeaderHovered]         = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
	style.Colors[ImGuiCol_HeaderActive]          = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
//	style.Colors[ImGuiCol_Column]                = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
//	style.Colors[ImGuiCol_ColumnHovered]         = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
//	style.Colors[ImGuiCol_ColumnActive]          = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	style.Colors[ImGuiCol_ResizeGrip]            = ImVec4(1.00f, 1.00f, 1.00f, 0.50f);
	style.Colors[ImGuiCol_ResizeGripHovered]     = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
	style.Colors[ImGuiCol_ResizeGripActive]      = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
//	style.Colors[ImGuiCol_CloseButton]           = ImVec4(0.59f, 0.59f, 0.59f, 0.50f);
//	style.Colors[ImGuiCol_CloseButtonHovered]    = ImVec4(0.98f, 0.39f, 0.36f, 1.00f);
//	style.Colors[ImGuiCol_CloseButtonActive]     = ImVec4(0.98f, 0.39f, 0.36f, 1.00f);
	style.Colors[ImGuiCol_PlotLines]             = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
	style.Colors[ImGuiCol_PlotLinesHovered]      = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	style.Colors[ImGuiCol_PlotHistogram]         = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_PlotHistogramHovered]  = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_TextSelectedBg]        = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
//	style.Colors[ImGuiCol_ModalWindowDarkening]  = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
	
	if( bStyleDark_ )
	{
		for (int i = 0; i <= ImGuiCol_COUNT; i++)
		{
			ImVec4& col = style.Colors[i];
			float H, S, V;
			ImGui::ColorConvertRGBtoHSV( col.x, col.y, col.z, H, S, V );
			
			if( S < 0.1f )
			{
				V = 1.0f - V;
			}
			ImGui::ColorConvertHSVtoRGB( H, S, V, col.x, col.y, col.z );
			if( col.w < 1.00f )
			{
				col.w *= alpha_;
			}
		}
	}
	else
	{
		for (int i = 0; i <= ImGuiCol_COUNT; i++)
		{
			ImVec4& col = style.Colors[i];
			if( col.w < 1.00f )
			{
				col.x *= alpha_;
				col.y *= alpha_;
				col.z *= alpha_;
				col.w *= alpha_;
			}
		}
	}
}


void EditorSystem::OnAdded()
{
	//imguiCreate();
	//IMGUI_CHECKVERSION();
	//ImGui::CreateContext();
	//ImGui_ImplWin32_Init(hwnd);
	//ImGui_ImplDX12_Init();
	
	SetupImGuiStyle(false, 1);
	
//	auto& io = ImGui::GetIO();
//	s_font = io.Fonts->AddFontFromFileTTF(FISHENGINE_ROOT "Assets/Fonts/SourceCodePro-Regular.ttf", 15.f);
//	unsigned char* pixels;
//	int width, height, bytes_per_pixels;
//	io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height, &bytes_per_pixels);

	auto go = m_Scene->CreateGameObject();
	go->name = "EditorCamera";
	auto cam = m_Scene->GameObjectAddComponent<Camera>(go);
	cam->m_Type = CameraType::Editor;
	go->GetTransform()->SetLocalPosition(0, 0, -10);
	m_Scene->GameObjectAddComponent<FreeCamera>(go);
	
	m_Scene->AddSystem<FreeCameraSystem>();
}


void EditorSystem::Draw()
{
	constexpr float hierarchy_width = 220;
	constexpr float inspector_width = 300;
	//printf("============here==========\n\n");
	auto input = m_Scene->GetSingletonComponent<SingletonInput>();
	if (input->IsButtonHeld(KeyCode::F1))
	{
		return;
	}

	auto selection = m_Scene->GetSingletonComponent<SingletonSelection>();
	assert(selection != nullptr);
	Vector2 mousePos = input->GetMousePosition();
	mousePos.y = 1.f - mousePos.y;
	mousePos.x *= EditorScreen::width;
	mousePos.y *= EditorScreen::height;
	//auto mouseBtns =
	//	(input->IsButtonHeld(KeyCode::MouseLeftButton) ? IMGUI_MBUT_LEFT : 0) |
	//	(input->IsButtonHeld(KeyCode::MouseRightButton) ? IMGUI_MBUT_RIGHT : 0) |
	//	(input->IsButtonHeld(KeyCode::MouseMiddleButton) ? IMGUI_MBUT_MIDDLE : 0);
	selected = nullptr;
	if (selection->selected != nullptr)
		selected = selection->selected->GetTransform();
	
	static float mouseScroll = 0;
	mouseScroll += input->GetAxis(Axis::MouseScrollWheel);
	//imguiBeginFrame((int)mousePos.x, (int)mousePos.y, mouseBtns, mouseScroll, EditorScreen::width, EditorScreen::height);
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
//	ImGui::PushFont(s_font);
	
	MainMenu();
	MainToolBar();

	
	float y_start = main_menu_bar_height + main_tool_bar_height;

	ImGui::SetNextWindowPos(ImVec2(0.0f, y_start));
	ImGui::SetNextWindowSize(ImVec2(hierarchy_width, (float)EditorScreen::height-y_start));
	Hierarchy();
	if (selected == nullptr)
		selection->selected = nullptr;
	else
		selection->selected = selected->m_GameObject;

	ImGui::SetNextWindowPos(ImVec2((float)EditorScreen::width - inspector_width, y_start));
	ImGui::SetNextWindowSize(ImVec2(inspector_width, (float)EditorScreen::height-y_start));
	Inspector();

	m_SceneViewRect.x = hierarchy_width;
	m_SceneViewRect.y = y_start;
	m_SceneViewRect.z = EditorScreen::width - hierarchy_width - inspector_width;
	m_SceneViewRect.w = EditorScreen::height - y_start;

#if 0
	ImGui::SetNextWindowPos(ImVec2(Screen::width - inspector_width, Screen::height / 2));
	ImGui::SetNextWindowSize(ImVec2(inspector_width, Screen::height / 2));
	ImGui::Begin("Systems", NULL, imgui_window_flags);
	for (System* s : m_Scene->GetSystems())
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

//	ImGui::PopFont();
	//imguiEndFrame();
	ImGui::Render();
}


void EditorSystem::MainMenu()
{
	bool showHelpWindow = false;
	ImGui::BeginMainMenuBar();
	main_menu_bar_height = ImGui::GetWindowHeight();
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
		for (System* s : m_GameScene->GetSystems())
		{
			ImGui::Checkbox(s->GetTypeName(), &s->m_Enabled);
			//ImGui::MenuItem(s->GetClassName(), nullptr, &s->m_Enabled);
		}
		ImGui::Separator();
		for (System* s : m_Scene->GetSystems())
		{
			ImGui::Checkbox(s->GetTypeName(), &s->m_Enabled);
			//ImGui::MenuItem(s->GetClassName(), nullptr, &s->m_Enabled);
		}
		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("Gizmos"))
	{
		ImGui::Checkbox("Enabled depth test", &Gizmos::s_EnableDepthTest);
		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("Help"))
	{
		if (ImGui::MenuItem("Camera Control"))
		{
			showHelpWindow = true;
//			ImGui::OpenPopup("Help-Control");
		}
		ImGui::EndMenu();
	}
	ImGui::EndMainMenuBar();
	
	if (showHelpWindow)
		ImGui::OpenPopup("Help-Camera Control");
	if (ImGui::BeginPopupModal("Help-Camera Control", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Move(Pan): move mouse with MiddleMouseButton or Alt+Ctrl/Cmd+LeftMouseButton");
		ImGui::Text("Orbit: Alt+LeftMouseButton");
		ImGui::Text("Zoom: MouseScroll or Alt+RightMouseButton");
		ImGui::Text("Rotate: RightMouseButton");
		ImGui::Text("F: Frame selected GameObject");
		ImGui::Separator();
		if (ImGui::Button("OK"))
		{
			ImGui::CloseCurrentPopup();
		}
		
		ImGui::EndPopup();
	}
	
}

void EditorSystem::MainToolBar()
{
	auto frame_padding = ImGui::GetStyle().FramePadding;
	float padding = frame_padding.y * 2;
	float height = 24 + padding;
	ImVec2 toolbar_size(ImGui::GetIO().DisplaySize.x, height);

	
	ImGui::SetNextWindowPos(ImVec2(0.0f, main_menu_bar_height));
	ImGui::SetNextWindowSize(ImVec2(EditorScreen::width, main_tool_bar_height));
	auto flag = 0
		| ImGuiWindowFlags_NoTitleBar
		| ImGuiWindowFlags_NoScrollbar
		| ImGuiWindowFlags_NoResize
		;
	//if (ImGui::BeginToolbar("MainToolBar", ImVec2(0, g_editorGUISettings.mainMenubarHeight), toolbar_size))
	ImGui::Begin("MainToolBar", nullptr, flag);
	{
		ImGui::SameLine();
		//if (FishEditorWindow::InPlayMode())
		if (true)
		{
			if (ImGui::Button("Stop"))
			{
				//FishEditorWindow::Stop();
			}
		}
		else
		{
			if (ImGui::Button("Play"))
			{
				//FishEditorWindow::Play();
			}
		}

		auto sceneView = m_Scene->GetSystem<SceneViewSystem>();

		ImGui::SameLine();
		if (ImGui::Button("Translate"))
		{
			sceneView->m_transformToolType = TransformToolType::Translate;
		}

		ImGui::SameLine();
		if (ImGui::Button("Rotate"))
		{
			sceneView->m_transformToolType = TransformToolType::Rotate;
		}

		ImGui::SameLine();
		if (ImGui::Button("Scale"))
		{
			sceneView->m_transformToolType = TransformToolType::Scale;
		}


		ImGui::SameLine();
		if (ImGui::Button("Pivot"))
		{

		}

		ImGui::SameLine();
		if (sceneView->m_transformSpace == TransformSpace::Global)
		{
			if (ImGui::Button("Global"))
			{
				sceneView->m_transformSpace = TransformSpace::Local;
			}
		}
		else
		{
			if (ImGui::Button("Local"))
			{
				sceneView->m_transformSpace = TransformSpace::Global;
			}
		}
		
	}
	ImGui::End();
	//ImGui::EndToolbar();
	//g_editorGUISettings.mainToolbarHeight = height;
}


void EditorSystem::Hierarchy()
{
	m_HierarchyView.selected = selected;
	m_HierarchyView.Draw(m_GameScene, m_Scene->GetSingletonComponent<SingletonInput>());
	selected = m_HierarchyView.selected;
}


#include <FishEditor/InspectorArchive.hpp>

InspectorArchive inspectorArchive;

void EditorSystem::Inspector()
{
	ImGui::Begin("Inspector", NULL, imgui_window_flags);
//	auto input = m_Scene->GetSingletonComponent<SingletonInput>();
//	Vector2 mp = input->GetMousePosition();
//	ImGui::InputFloat2("MousePos", mp.data());
//	Vector2 delta_mp(input->GetAxis(Axis::MouseX), input->GetAxis(Axis::MouseY));
//	ImGui::InputFloat2("axis", delta_mp.data());
	if (selected != nullptr)
	{
		for (auto comp : selected->m_GameObject->GetComponents())
		{
			if (ImGui::CollapsingHeader(comp->GetTypeName(), ImGuiTreeNodeFlags_DefaultOpen))
			{
				comp->Serialize(inspectorArchive);
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
						t->SetLocalScale(s);
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
					
//					if (r->mesh->m_SubMeshCount)
//					ImGui::LabelText("Submesh Count", "%d", r->mesh->m_SubMeshCount);
					if (r->m_Mesh != nullptr)
						ImGui::Text("Submesh Count: %d", r->m_Mesh->m_SubMeshCount);
//					for (auto& x : r->mesh->m_SubMeshInfos)
//					{
//						ImGui::Text("  %d", x.Length);
//					}

					bool skinned = r->m_Skin != nullptr;
					//ImGui::Checkbox("skinned", &skinned);
					if (skinned)
					{
						auto skin = r->m_Skin;
						ImGui::Text("Bone Count: %d", r->m_Skin->joints.size());
						ImGui::Text("Root Bone: %s", skin->root->name.c_str());
//						for (auto bone : skin->joints)
//						{
//							ImGui::Text(bone->m_Name.c_str());
//						}
					}

					if (ImGui::CollapsingHeader("Materials", ImGuiTreeNodeFlags_DefaultOpen))
					{
						ImGui::Indent();
						for (int i = 0; i < r->m_Materials.size(); ++i)
						{
							auto material = r->m_Materials[i];
							if (material == nullptr)
								continue;
//							auto header = "Material" + std::to_string(i);
							auto header = material->name;
							if (ImGui::CollapsingHeader(header.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
							{
//								ImGui::LabelText("name", material->name.c_str());
								//for (auto& p : material->m_UniformInfos)
								//{
								//	auto name = p.first.c_str();
								//	auto& info = p.second.second;
								//	if (info.type == bgfx::UniformType::Vec4)
								//	{
								//		if (material->m_MaterialProperties.vec4s.find(p.first) != material->m_MaterialProperties.vec4s.end())
								//		{
								//			auto& v = material->m_MaterialProperties.vec4s[p.first];
								//			ImGui::ColorEdit4(name, v.data());
								//		}
								//	}
								//	else if (info.type == bgfx::UniformType::Int1)
								//	{
								//		if (material->m_MaterialProperties.textures.find(p.first) != material->m_MaterialProperties.textures.end())
								//		{
								//			auto& v = material->m_MaterialProperties.textures[p.first];
								//			ImGui::Image(v, ImVec2(64, 64));
								//		}
								//	}
								//}
							}
						}
						ImGui::Unindent();
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
					int fov = (int)c->GetFieldOfView();
					ImGui::SliderInt("Field of View", &fov, 1, 179);
					c->SetFieldOfView((float)fov);
					ImGui::SliderFloat("Near", &c->m_NearClipPlane, 0.01f, c->m_FarClipPlane);
					ImGui::SliderFloat("Far", &c->m_FarClipPlane, 1.f, 1000.f);
					bool perspective = c->GetOrthographic();
					ImGui::Checkbox("Perspective", &perspective);
					c->SetOrthographic(perspective);
				}
			}
		}
	}
	ImGui::End();
}
