#include <FishEditor/Systems/EditorSystem.hpp>
#include <FishEngine/Material.hpp>
#include <FishEngine/Gizmos.hpp>
#include <FishEngine/Mesh.hpp>
#include <FishEngine/Components/Transform.hpp>
#include <FishEngine/Components/SingletonInput.hpp>
#include <FishEngine/Components/Renderable.hpp>
#include <FishEngine/Components/Animator.hpp>
#include <FishEngine/Components/Camera.hpp>
#include <FishEngine/GraphicsAPI.hpp>

#include <FishEditor/Components/FreeCamera.hpp>
#include <FishEditor/Components/SingletonSelection.hpp>
#include <FishEditor/Systems/FreeCameraSystem.hpp>
#include <FishEditor/Systems/SceneViewSystem.hpp>
#include <FishEngine/Assets.hpp>
#include <FishEngine/Shader.hpp>

//#include <GLFW/glfw3.h>
#include <imgui.h>

#include <set>

float main_menu_bar_height = 24;
constexpr float main_tool_bar_height = 40;
constexpr float status_bar_height = 24;

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
//	imguiCreate();
//	ImGui::CreateContext();
	
//	SetupImGuiStyle(true, 1);
	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowRounding = 0.0f;
	
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
	constexpr float hierarchy_width = 200;
	constexpr float inspector_width = 250;
	//printf("============here==========\n\n");
	auto input = m_Scene->input;
//	if (input->IsButtonHeld(KeyCode::F1))
//	{
//		return;
//	}

	auto selection = m_Scene->GetSingletonComponent<SingletonSelection>();
	assert(selection != nullptr);
	Vector2 mousePos = input->GetMousePosition();
	mousePos.y = 1.f - mousePos.y;
	mousePos.x *= EditorScreen::width;
	mousePos.y *= EditorScreen::height;
//	auto mouseBtns =
//		(input->IsButtonHeld(KeyCode::MouseLeftButton) ? IMGUI_MBUT_LEFT : 0) |
//		(input->IsButtonHeld(KeyCode::MouseRightButton) ? IMGUI_MBUT_RIGHT : 0) |
//		(input->IsButtonHeld(KeyCode::MouseMiddleButton) ? IMGUI_MBUT_MIDDLE : 0);
	selected = nullptr;
	if (selection->selected != nullptr)
		selected = selection->selected->GetTransform();
	
	static float mouseScroll = 0;
	mouseScroll += input->GetAxis(Axis::MouseScrollWheel);
//	imguiBeginFrame((int)mousePos.x, (int)mousePos.y, mouseBtns, mouseScroll, EditorScreen::width, EditorScreen::height);
	
//	ImGui::NewFrame();
	ImguiNewFrame();
	
	MainMenu();
	MainToolBar();

	
	float y_start = main_menu_bar_height + main_tool_bar_height;

	ImGui::SetNextWindowPos(ImVec2(0.0f, y_start));
	ImGui::SetNextWindowSize(ImVec2(hierarchy_width, (float)EditorScreen::height-y_start-status_bar_height));
	Hierarchy();
	if (selected == nullptr)
		selection->selected = nullptr;
	else
		selection->selected = selected->m_GameObject;

	ImGui::SetNextWindowPos(ImVec2((float)EditorScreen::width - inspector_width, y_start));
	ImGui::SetNextWindowSize(ImVec2(inspector_width, (float)EditorScreen::height-y_start-status_bar_height));
	Inspector();

	m_SceneViewRect.x = hierarchy_width;
	m_SceneViewRect.y = y_start;
	m_SceneViewRect.z = EditorScreen::width - hierarchy_width - inspector_width;
	m_SceneViewRect.w = EditorScreen::height - y_start;

	ImGui::SetNextWindowPos(ImVec2(0, EditorScreen::height-status_bar_height));
	ImGui::SetNextWindowSize(ImVec2(EditorScreen::width, status_bar_height));
	StatusBar();
	
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
	
	//ImGui::ShowDemoWindow();

//	imguiEndFrame();
	ImguiRender();
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
			ImGui::Checkbox(s->GetClassName(), &s->m_Enabled);
			//ImGui::MenuItem(s->GetClassName(), nullptr, &s->m_Enabled);
		}
		ImGui::Separator();
		for (System* s : m_Scene->GetSystems())
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

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_internal.h>

namespace ImGui
{
	static int _count = 2;
	static int _width = 1;
	static int _index = 0;
	constexpr float _roundRadius = 3.0f;
	void BeginSegmentedButtons(int count, int width)
	{
		assert(count >= 2);
		assert(width > 1);
		_count = count;
		_index = 0;
		_width = width;
	}
	
	void EndSegmentedButtons()
	{
		assert(_index == _count);
	}
	
	bool SegmentedButton(const char* label, bool checked=false)
	{
		assert(_index < _count);
		int index = _index;
		_index++;
		
		auto& style = ImGui::GetStyle();
		if (index > 0)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
			ImGui::SameLine();
			ImGui::PopStyleVar();
		}
		
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return false;
		
//		ImGuiContext& g = *GImGui;
//		const ImGuiStyle& style = g.Style;
		const ImGuiID id = window->GetID(label);
		const ImVec2 label_size = CalcTextSize(label, NULL, true);
		
		const ImVec2 pos = window->DC.CursorPos;
		float x_padding = 0;
		if (index == 0 || index == _count-1)
			x_padding = style.FramePadding.x;
		ImVec2 size = CalcItemSize(ImVec2(0, 0), label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);
		
		const ImRect bb(pos, pos + size);
		ItemSize(bb, style.FramePadding.y);
		if (!ItemAdd(bb, id))
			return false;
		
		
		ImGuiButtonFlags flags = 0;
		if (window->DC.ItemFlags & ImGuiItemFlags_ButtonRepeat)
			flags |= ImGuiButtonFlags_Repeat;
		bool hovered, held;
		bool pressed = ButtonBehavior(bb, id, &hovered, &held, flags);
		if (pressed)
			MarkItemEdited(id);
		
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		
		const ImU32 col = GetColorU32(checked||(held && hovered) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
		int cornerFlags = 0;
		if (index == 0)
			cornerFlags |= ImDrawCornerFlags_Left;
		else if (index == _count-1)
			cornerFlags |= ImDrawCornerFlags_Right;
		draw_list->AddRectFilled(bb.Min, bb.Max, col, 4.f, cornerFlags);
		const ImU32 borderCol = GetColorU32(ImGuiCol_Text);
		draw_list->AddRect(bb.Min, bb.Max, borderCol, 4.f, cornerFlags);
		
		RenderTextClipped(bb.Min + style.FramePadding, bb.Max - style.FramePadding, label, NULL, &label_size, style.ButtonTextAlign, &bb);
		
		return pressed;
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
		auto sceneView = m_Scene->GetSystem<SceneViewSystem>();
		auto& style = ImGui::GetStyle();
		
		
		ImGui::BeginSegmentedButtons(4, 40);
		ImGui::SegmentedButton("Hand");
		if (ImGui::SegmentedButton("Move", sceneView->m_transformToolType == TransformToolType::Translate))
			sceneView->m_transformToolType = TransformToolType::Translate;
		if (ImGui::SegmentedButton("Rotate", sceneView->m_transformToolType == TransformToolType::Rotate))
			sceneView->m_transformToolType = TransformToolType::Rotate;
		if (ImGui::SegmentedButton("Scale", sceneView->m_transformToolType == TransformToolType::Scale))
			sceneView->m_transformToolType = TransformToolType::Scale;
		ImGui::EndSegmentedButtons();
		
		ImGui::SameLine();
		
		ImVec2 framePadding = style.FramePadding;
		framePadding.y -= 2;
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, framePadding);
		ImGui::BeginSegmentedButtons(2, 40);
		ImGui::SegmentedButton("Pivot");
//		ImGui::SegmentedButton("Global");
		if (sceneView->m_transformSpace == TransformSpace::Global)
		{
			if (ImGui::SegmentedButton("Global"))
			{
				sceneView->m_transformSpace = TransformSpace::Local;
			}
		}
		else
		{
			if (ImGui::SegmentedButton("Local"))
			{
				sceneView->m_transformSpace = TransformSpace::Global;
			}
		}
		ImGui::EndSegmentedButtons();
		ImGui::PopStyleVar();
		
		ImGui::SameLine();
		
		ImGui::BeginSegmentedButtons(3, 40);
		ImGui::SegmentedButton("Play");
		ImGui::SegmentedButton("Stop");
		ImGui::SegmentedButton("Pause");
		ImGui::EndSegmentedButtons();
		
		ImGui::SameLine();
		//if (FishEditorWindow::InPlayMode())
	}
	ImGui::End();
	//ImGui::EndToolbar();
	//g_editorGUISettings.mainToolbarHeight = height;
}


void EditorSystem::Hierarchy()
{
	m_HierarchyView.selected = selected;
	m_HierarchyView.Draw(m_GameScene, m_Scene);
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
			ImGui::PushID(comp);
			if (ImGui::CollapsingHeader(comp->GetClassName(), ImGuiTreeNodeFlags_DefaultOpen))
			{
//				comp->Serialize(inspectorArchive);
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
					ImGui::Text("Submesh Count: %d", r->m_Mesh->m_SubMeshCount);
					for (auto& x : r->m_Mesh->m_SubMeshInfos)
					{
						ImGui::Text("  %d", x.Length);
					}

					bool skinned = r->m_Skin != nullptr;
					//ImGui::Checkbox("skinned", &skinned);
					if (skinned)
					{
						auto skin = r->m_Skin;
						ImGui::Text("Bone Count: %lu", r->m_Skin->joints.size());
						ImGui::Text("Root Bone: %s", skin->root->name.c_str());
//						for (auto bone : skin->joints)
//						{
//							ImGui::Text(bone->m_Name.c_str());
//						}
					}

					if (ImGui::CollapsingHeader("Materials", ImGuiTreeNodeFlags_DefaultOpen))
					{
						ImGui::Indent();
						std::set<Material*> materials;
						for (int i = 0; i < r->m_Materials.size(); ++i)
						{
							auto material = r->m_Materials[i];
							if (materials.find(material) != materials.end())
							{
								continue;
							}
							materials.insert(material);
							
//							auto header = "Material" + std::to_string(i);
							auto header = material->name;
							if (ImGui::CollapsingHeader(header.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
							{
//								ImGui::LabelText("name", "%s", material->name.c_str());
								if (material->m_Shader != nullptr)
								{
									auto s = material->m_Shader;
									for (auto& arg : s->m_VertexShaderSignature.arguments)
									{
										for (auto& u : arg.uniforms)
										{
											if (u.dataType == ShaderDataType::Float4)
											{
												ImGui::ColorEdit4(u.name.c_str(), material->m_MaterialProperties.vec4s[u.name].data());
											}
											else if (u.dataType == ShaderDataType::Float)
											{
												ImGui::SliderFloat(u.name.c_str(), material->m_MaterialProperties.vec4s[u.name].data(), 0, 1);
											}
										}
									}
									for (auto& arg : s->m_FragmentShaderSignature.arguments)
									{
										for (auto& u : arg.uniforms)
										{
											if (u.dataType == ShaderDataType::Float4)
											{
												ImGui::ColorEdit4(u.name.c_str(), material->m_MaterialProperties.vec4s[u.name].data());
											}
											else if (u.dataType == ShaderDataType::Float)
											{
												ImGui::SliderFloat(u.name.c_str(), material->m_MaterialProperties.vec4s[u.name].data(), 0, 1);
											}
										}
										
									}
									for (auto& t : s->m_FragmentShaderSignature.textures)
									{
										ImGui::TextWrapped("%s", t.name.c_str());
//										ImGui::SameLine();
										FishEngine::ImGuiDrawTexture(material->m_MaterialProperties.textures[t.name], Vector2(128, 128));
									}
								}
//								for (auto& p : material->m_UniformInfos)
//								{
//									auto name = p.first.c_str();
//									auto& info = p.second.second;
//									if (info.type == bgfx::UniformType::Vec4)
//									{
//										if (material->m_MaterialProperties.vec4s.find(p.first) != material->m_MaterialProperties.vec4s.end())
//										{
//											auto& v = material->m_MaterialProperties.vec4s[p.first];
//											ImGui::ColorEdit4(name, v.data());
//										}
//									}
//									else if (info.type == bgfx::UniformType::Int1)
//									{
//										if (material->m_MaterialProperties.textures.find(p.first) != material->m_MaterialProperties.textures.end())
//										{
//											auto& v = material->m_MaterialProperties.textures[p.first];
//											ImGui::Image(v, ImVec2(64, 64));
//										}
//									}
//								}
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
			ImGui::PopID();
		}
	}
	ImGui::End();
}


void FishEditor::EditorSystem::StatusBar()
{
	ImGui::Begin("statusbar", nullptr, imgui_window_flags | ImGuiWindowFlags_NoTitleBar);
//	static double timeStamp = glfwGetTime();
//	static int frames = 0;
//	static int fps = 30;
//	constexpr int frameCount = 200;
//	frames++;
//	if (frames == frameCount)
//	{
//		double now = glfwGetTime();
//		fps = int(frameCount / (now - timeStamp));
//		timeStamp = now;
//		frames = 0;
//	}
//	ImGui::Text("fps:%d", fps);
	ImGui::End();
}
