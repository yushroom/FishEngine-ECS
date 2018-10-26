#pragma once

#include "IMGUI.hpp"

#define IMGUI 1

#if IMGUI

#include <pch.h>
#include <EngineTuning.h>
#include <DescriptorHeap.h>
#include <GLFW/glfw3.h>

extern GLFWwindow* g_gWindow;
namespace Graphics
{
	extern ID3D12Device* g_Device;
	extern DXGI_FORMAT DefaultHdrColorFormat;
}

#include <imgui.h>
#include <imgui_impl_dx12.h>
#include <imgui_impl_glfw.h>


void NumVar::OnImGUI(const char* label)
{
	if (m_MinValue == -FLT_MAX || m_MaxValue == FLT_MAX)
		ImGui::InputFloat(label, &m_Value);
	else
		ImGui::SliderFloat(label, &m_Value, m_MinValue, m_MaxValue);
}

void IntVar::OnImGUI(const char* label)
{
	ImGui::InputInt(label, &m_Value);
}

void ExpVar::OnImGUI(const char* label)
{
	//float val = float(*this);
	//if (ImGui::DragFloat(label, &val))
	//	*this = val;
	NumVar::OnImGUI(label);
}

void EnumVar::OnImGUI(const char* label)
{
	ImGui::Combo(label, &m_Value, m_EnumLabels, m_EnumLength);
}

void CallbackTrigger::OnImGUI(const char* label)
{
	if (ImGui::Button(label))
		Bang();
}

void BoolVar::OnImGUI(const char* label)
{
	if (ImGui::Checkbox(label, &m_Flag))
	{
		printf("Set %s\n", label);
	}
}

void VariableGroup::OnImGUI(const char* label)
{
	for (auto& pair : m_Children)
	{
		//ImGui::Text("%s", pair.first.c_str());
		const char* label2 = pair.first.c_str();
		VariableGroup* subGroup = dynamic_cast<VariableGroup*>(pair.second);
		ImGui::PushID(this);
		if (subGroup != nullptr)
		{
			if (ImGui::CollapsingHeader(label2, ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::Indent(8);
				subGroup->OnImGUI(label2);
				ImGui::Unindent(8);
			}
		}
		else
		{
			pair.second->OnImGUI(label2);
		}
		ImGui::PopID();
	}
}


UserDescriptorHeap g_imguiSrvDescHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1);

void InitImgui()
{
#if IMGUI
	auto& heap = g_imguiSrvDescHeap;
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	auto& io = ImGui::GetIO();
#if _WIN32
	io.Fonts->AddFontFromFileTTF("I:\\FishEngine-ECS\\ThirdParty\\imgui\\misc\\fonts/DroidSans.ttf", 16.0f);
#else
	io.Fonts->AddFontFromFileTTF("/Users/yushroom/program/FishEngine-ECS/Assets/Fonts/DroidSans.ttf", 16.0f);
#endif
	ImGui_ImplGlfw_InitForOpenGL(g_gWindow, false);
	heap.Create(L"imguiSrvDescHeap");
	auto pd3dSrvDescHeap = heap.GetHeapPointer();
	ImGui_ImplDX12_Init(Graphics::g_Device, 2, Graphics::DefaultHdrColorFormat,
		pd3dSrvDescHeap->GetCPUDescriptorHandleForHeapStart(),
		pd3dSrvDescHeap->GetGPUDescriptorHandleForHeapStart());

	ImGui::StyleColorsLight();

#endif // IMGUI
}

#endif // IMGUI
