#pragma once

#include <FishEngine/Components/SingletonInput.hpp>
#include <FishEngine/Components/Transform.hpp>
#include <FishEngine/Components/Camera.hpp>
#include <FishEngine/ECS/Scene.hpp>

#include <imgui.h>

constexpr int imgui_window_flags = 0
	| ImGuiWindowFlags_NoResize
	| ImGuiWindowFlags_NoMove
	| ImGuiWindowFlags_NoCollapse
	| ImGuiWindowFlags_NoSavedSettings
	| ImGuiWindowFlags_NoFocusOnAppearing
;

namespace FishEditor
{


struct HierarchyView
{
	void Reset()
	{
		hierarchyList.clear();
		timeSinceLastKey += 0.02f;
//		selectedLeft = false;
//		selectedRight = false;
//		m_ScrollToSelected = false;
	}
	
	void Draw(FishEngine::Scene* gameScene, FishEngine::SingletonInput* input)
	{
		this->scene = gameScene;
		this->input = input;
		Reset();
		
		ImGui::Begin("Hierarchy", NULL, imgui_window_flags);
		m_LeftMouseButtonClicked = ImGui::IsWindowHovered() && input->IsButtonPressed(FishEngine::KeyCode::MouseLeftButton);
		
		ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 4);
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0, 0));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		
		HierarchyNode(FishEngine::Camera::GetEditorCamera()->GetTransform());
		for (auto t : scene->m_RootTransforms)
		{
			HierarchyNode(t);
		}
		
		ImGui::PopStyleVar(4);
		
		m_ScrollToSelected = false;
		selectedLeft = false;
		selectedRight = false;
		
		if (ImGui::IsWindowFocused() && selected != nullptr)
		{
			int idx = std::distance(hierarchyList.begin(), std::find(hierarchyList.begin(), hierarchyList.end(), selected));
			if (idx == hierarchyList.size())
				idx = -1;
			
			bool is_leaf = selected->GetChildren().empty();
			int count = hierarchyList.size();
//			printf("%d\n", count);
			bool keyDown = false;
			if (__IsKeyDown(FishEngine::KeyCode::DownArrow))
			{
				idx++;
				keyDown = true;
			}
			else if (__IsKeyDown(FishEngine::KeyCode::UpArrow))
			{
				idx--;
				keyDown = true;
			}
			else if (__IsKeyDown(FishEngine::KeyCode::LeftArrow))
			{
				if (is_leaf || !selectedIsOpen)
					idx--;
				else
					selectedLeft = true;
				keyDown = true;
			}
			else if (__IsKeyDown(FishEngine::KeyCode::RightArrow))
			{
				if (is_leaf || selectedIsOpen)
					idx++;
				else
					selectedRight = true;
				keyDown = true;
			}
			idx = FishEngine::Mathf::Clamp(idx, 0, count-1);
			selected = hierarchyList[idx];
			
			if (keyDown)
			{
				timeSinceLastKey = 0;
				m_ScrollToSelected = true;
			}
			
			if (m_LeftMouseButtonClicked)
			{
				selected = nullptr;
			}
		}
		
		ImGui::End();
	}
	
	
	void HierarchyNode(FishEngine::Transform* t)
	{
		hierarchyList.push_back(t);
		auto name = t->m_GameObject->name;
		if (name.empty())
			name = "go:" + std::to_string(t->entityID);
		
		bool isLeaf = t->GetChildren().empty();
		ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
		if (selected == t)
		{
			node_flags |= ImGuiTreeNodeFlags_Selected;
			if (selectedRight)
				ImGui::SetNextTreeNodeOpen(true);
			if (selectedLeft)
				ImGui::SetNextTreeNodeOpen(false);
		}
		
		if (isLeaf)
			node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
		node_flags |= ImGuiTreeNodeFlags_DefaultOpen;
		bool node_open = ImGui::TreeNodeEx((void*)t, node_flags, "%s", name.c_str());
		if (ImGui::IsItemClicked())
		{
			selected = t;
			m_LeftMouseButtonClicked = false;
		}
		
		if (selected == t && !ImGui::IsItemVisible())
		{
			if (m_ScrollToSelected)
				ImGui::SetScrollHereY();
		}
		
		if (!isLeaf && node_open)
		{
			for (auto c : t->GetChildren())
				HierarchyNode(c);
			ImGui::TreePop();
		}
		
		if (selected == t)
		{
			selectedIsOpen = node_open;
		}
	};
	
	
	FishEngine::Transform* selected = nullptr;

private:
	
	inline bool __IsKeyDown(FishEngine::KeyCode key)
	{
		return input->IsButtonPressed(key) ||
			(input->GetButtonHeldTime(key) >= c_KeyTimeStep && timeSinceLastKey >= c_KeyTimeStep);
	}
	
	FishEngine::Scene* scene = nullptr;
	FishEngine::SingletonInput* input = nullptr;
	std::vector<FishEngine::Transform*> hierarchyList;
	bool m_LeftMouseButtonClicked = false;
	bool selectedLeft = false;
	bool selectedRight = false;
	bool selectedIsOpen = false;
	bool m_ScrollToSelected = false;
	
	// seconds
	float timeSinceLastKey = c_KeyTimeStep;
	static constexpr float c_KeyTimeStep = 0.1f;
};
	
}
