#include <FishEngine/Render/RenderContext.hpp>
#include <FishEngine/Render/Application.h>
#include <FishEngine/Render/CommandQueue.h>

#include <FishEngine/Render/RenderBuffer.hpp>

using namespace FishEngine;

uint16_t D3D12::VertexBuffer::NextIndex = 0;
std::unordered_map<uint16_t, FishEngine::D3D12::VertexBuffer> D3D12::VertexBuffer::Buffers;
uint16_t D3D12::IndexBuffer::NextIndex = 0;
std::unordered_map<uint16_t, FishEngine::D3D12::IndexBuffer> D3D12::IndexBuffer::Buffers;



#include <d3d12.h>
#include <wrl.h>
using namespace Microsoft::WRL;
#include <FishEngine/Render/d3dx12.h>
#include <FishEngine/Render/Helpers.h>

void UpdateBufferResource(
	ComPtr<ID3D12Device2> device,
	ComPtr<ID3D12GraphicsCommandList2> commandList,
	ID3D12Resource** pDestinationResource,
	ID3D12Resource** pIntermediateResource,
	size_t numElements, size_t elementSize, const void* bufferData,
	D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE)
{
	size_t bufferSize = numElements * elementSize;

	// Create a committed resource for the GPU resource in a default heap.
	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(bufferSize, flags),
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(pDestinationResource)));

	// Create a committed resource for the upload.
	if (bufferData)
	{
		ThrowIfFailed(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(bufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(pIntermediateResource)));

		D3D12_SUBRESOURCE_DATA subresourceData = {};
		subresourceData.pData = bufferData;
		subresourceData.RowPitch = bufferSize;
		subresourceData.SlicePitch = subresourceData.RowPitch;

		UpdateSubresources(commandList.Get(),
			*pDestinationResource, *pIntermediateResource,
			0, 0, 1, &subresourceData);
	}
}


FishEngine::RenderContext::RenderContext()
{
}

void FishEngine::RenderContext::Init()
{
	//Reset(0, 0, RenderReset::VSYNC | RenderReset::Flag1);
}

void FishEngine::RenderContext::Reset(int width, int height, RenderResetFlags flags)
{

}

VertexBufferHandle FishEngine::CreateVertexBuffer(const void* data, int vertexCount, int vertexSize)
{
	auto idx = D3D12::VertexBuffer::NextIndex;
	VertexBufferHandle handle(idx);
	auto& vb = D3D12::VertexBuffer::Buffers[idx];
	D3D12::VertexBuffer::NextIndex++;

	auto device = Application::Get().GetDevice();
	auto commandQueue = Application::Get().GetCommandQueue(D3D12_COMMAND_LIST_TYPE_COPY);
	auto commandList = commandQueue->GetCommandList();

	// Upload vertex buffer data.
	ComPtr<ID3D12Resource> intermediateVertexBuffer;
	UpdateBufferResource(device, commandList,
		&vb.buffer, &intermediateVertexBuffer,
		vertexCount, vertexSize, data);

	// Create the vertex buffer view.
	vb.bufferView.BufferLocation = vb.buffer->GetGPUVirtualAddress();
	vb.bufferView.SizeInBytes = vertexCount * vertexSize;
	vb.bufferView.StrideInBytes = vertexSize;

	// TODO
	auto fenceValue = commandQueue->ExecuteCommandList(commandList);
	commandQueue->WaitForFenceValue(fenceValue);

	return handle;
}

FishEngine::IndexBufferHandle FishEngine::CreateIndexBuffer(const void* data, int indexCount, int stride)
{
	auto idx = D3D12::IndexBuffer::NextIndex;
	IndexBufferHandle handle(idx);
	auto& ib = D3D12::IndexBuffer::Buffers[idx];
	D3D12::IndexBuffer::NextIndex++;

	auto device = Application::Get().GetDevice();
	auto commandQueue = Application::Get().GetCommandQueue(D3D12_COMMAND_LIST_TYPE_COPY);
	auto commandList = commandQueue->GetCommandList();

	// Upload index buffer data.
	ComPtr<ID3D12Resource> intermediateIndexBuffer;
	UpdateBufferResource(device, commandList,
		&ib.buffer, &intermediateIndexBuffer,
		indexCount, sizeof(uint32_t), data);

	// Create index buffer view.
	ib.bufferView.BufferLocation = ib.buffer->GetGPUVirtualAddress();
	ib.bufferView.Format = DXGI_FORMAT_R32_UINT;
	ib.bufferView.SizeInBytes = indexCount * stride;

	// TODO
	auto fenceValue = commandQueue->ExecuteCommandList(commandList);
	commandQueue->WaitForFenceValue(fenceValue);

	return handle;
}

void FishEngine::RenderContext::AddCopyQueue(CopyTask copyTask)
{
	m_copyQueue.push(copyTask);
}

void FishEngine::RenderContext::Frame()
{
	auto commandQueue = Application::Get().GetCommandQueue(D3D12_COMMAND_LIST_TYPE_COPY);
	auto commandList = commandQueue->GetCommandList();

	while (!m_copyQueue.empty())
	{
		m_copyQueue.front();
		m_copyQueue.pop();
	}

	auto fenceValue = commandQueue->ExecuteCommandList(commandList);
	commandQueue->WaitForFenceValue(fenceValue);
}
