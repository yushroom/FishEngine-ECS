#include <FishEngine/Systems/EditorSystem.hpp>
#include <FishEngine/Systems/SelectionSystem.hpp>
#include <FishEngine/Screen.hpp>
#include <FishEngine/Components/Transform.hpp>
#include <FishEngine/Components/SingletonInput.hpp>
#include <FishEngine/Components/Renderable.hpp>
#include <FishEngine/Components/Animator.hpp>

#include <imgui/imgui.h>

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
}


void EditorSystem::Update()
{
	const int hierarchy_width = 250;
	const int inspector_width = 250;
	//printf("============here==========\n\n");
	auto input = m_Scene->GetSingletonComponent<SingletonInput>();
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
	imguiBeginFrame(mousePos.x, mousePos.y, mouseBtns, input->GetAxis(Axis::MouseScrollWheel), Screen::width, Screen::height);
	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(ImVec2(hierarchy_width, Screen::height));
	Hierarchy();

	selectionSystem->selected = selected->m_GameObject;

	ImGui::SetNextWindowPos(ImVec2(Screen::width - inspector_width, 0));
	ImGui::SetNextWindowSize(ImVec2(inspector_width, Screen::height / 2));
	Inspector();

	ImGui::SetNextWindowPos(ImVec2(Screen::width - inspector_width, Screen::height / 2));
	ImGui::SetNextWindowSize(ImVec2(inspector_width, Screen::height / 2));
	ImGui::Begin("Systems", NULL, 0);
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

	imguiEndFrame();
}

void EditorSystem::Hierarchy()
{
	ImGui::Begin("Hierarchy", NULL, 0);
	for (auto t : m_Scene->m_RootTransforms)
	{
		HierarchyNode(t);
	}
	ImGui::End();
}

void EditorSystem::Inspector()
{
	ImGui::Begin("Inspector", NULL, 0);
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
				}
				else if (comp->Is<Animator>())
				{
					auto a = comp->As<Animator>();
					ImGui::InputInt("current clip", &a->m_CurrentClipIndex);
					ImGui::InputFloat("local timer", &a->m_LocalTimer);
				}
			}
		}
	}
	ImGui::End();
}
