#include <FishEngine/Render/D3D12/CommandList.hpp>
#include <FishEngine/Render/D3D12/Application.h>
#include <FishEngine/Render/D3D12/Helpers.h>
#include <FishEngine/Render/D3D12/UploadBuffer.hpp>
#include <FishEngine/Render/D3D12/ResourceStateTracker.h>

//using namespace FishEngine;

void CommandList::FlushResourceBarriers()
{
	m_ResourceStateTracker->FlushResourceBarriers(*this);
}

CommandList::CommandList(D3D12_COMMAND_LIST_TYPE type)
	: m_d3d12CommandListType(type)
{
	auto device = Application::Get().GetDevice();

	ThrowIfFailed(device->CreateCommandAllocator(m_d3d12CommandListType, IID_PPV_ARGS(&m_d3d12CommandAllocator)));

	ThrowIfFailed(device->CreateCommandList(0, m_d3d12CommandListType, m_d3d12CommandAllocator.Get(),
		nullptr, IID_PPV_ARGS(&m_d3d12CommandList)));

	m_UploadBuffer = std::make_unique<UploadBuffer>();

	//m_ResourceStateTracker = std::make_unique<ResourceStateTracker>();

	//for (int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
	//{
	//	m_DynamicDescriptorHeap[i] = std::make_unique<DynamicDescriptorHeap>(static_cast<D3D12_DESCRIPTOR_HEAP_TYPE>(i));
	//	m_DescriptorHeaps[i] = nullptr;
	//}
}

void CommandList::SetGraphicsDynamicConstantBuffer(uint32_t rootParameterIndex, size_t sizeInBytes, const void* bufferData)
{
	// Constant buffers must be 256-byte aligned.
	auto heapAllococation = m_UploadBuffer->Allocate(sizeInBytes, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
	memcpy(heapAllococation.CPU, bufferData, sizeInBytes);

	m_d3d12CommandList->SetGraphicsRootConstantBufferView(rootParameterIndex, heapAllococation.GPU);
}

void CommandList::SetGraphics32BitConstants(uint32_t rootParameterIndex, uint32_t numConstants, const void* constants)
{
	m_d3d12CommandList->SetGraphicsRoot32BitConstants(rootParameterIndex, numConstants, constants, 0);
}
