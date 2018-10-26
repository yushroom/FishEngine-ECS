#include "Graphics.hpp"

#include <d3d12.h>
#include <wrl.h>

#include <GpuBuffer.h>

using namespace FishEngine;
using Microsoft::WRL::ComPtr;


constexpr int MAX_BUFFER_SIZE = 1024;
static int g_current_buffer_index = 0;
inline int NextBuffer() { return ++g_current_buffer_index; }
static GpuBuffer* g_buffers[MAX_BUFFER_SIZE] = {nullptr,};

constexpr int MAX_SHADER_SIZE = 1024;
static int g_current_shader_index = 0;
inline int NextShader() { return ++g_current_shader_index; }
static ComPtr<ID3DBlob> g_shader_blobs[MAX_SHADER_SIZE];


VertexBufferHandle FishEngine::CreateVertexBuffer(const Memory& data, const VertexDecl& decl)
{
	int vertexStride = decl.GetVertexSize();
	assert(data.byteSize % vertexStride == 0);
	StructuredBuffer* buffer = new StructuredBuffer();
	buffer->Create(L"", data.byteSize / vertexStride, vertexStride, data.data);

	VertexBufferHandle handle;
	handle.idx = NextBuffer();
	g_buffers[handle.idx] = buffer;
	return handle;
}

FishEngine::IndexBufferHandle FishEngine::CreateIndexBuffer(const Memory& data, MeshIndexType type)
{
	//m_IndexBufferDepth.Create(L"IndexBufferDepth", m_Header.indexDataByteSize / sizeof(uint16_t), sizeof(uint16_t), m_pIndexDataDepth);
	ByteAddressBuffer* buffer = new ByteAddressBuffer();
	int stride = 2;
	if (type == MeshIndexType::UInt16)
		stride = sizeof(uint16_t);
	else
		stride = sizeof(uint32_t);
	buffer->Create(L"", data.byteSize / stride, stride, data.data);

	IndexBufferHandle handle;
	handle.idx = NextBuffer();
	g_buffers[handle.idx] = buffer;
	return handle;
}

FishEngine::ShaderHandle FishEngine::CreateShader(const char* functionName)
{
	ShaderHandle handle;
	handle.idx = NextShader();
	//ThrowIfFailed(Application)

	return handle;
}

//FishEngine::CommandQueue::CommandQueue()
//{
//	//impl = std::make_unique<CommandQueueImpl>();
//}

#include "GraphicsPlatform.hpp"

StructuredBuffer* FishEngine::GetVertexBuffer(VertexBufferHandle handle)
{
	assert(handle.idx > 0 && handle.idx <= g_current_buffer_index);
	return (StructuredBuffer*)g_buffers[handle.idx];
}


ByteAddressBuffer* FishEngine::GetIndexBuffer(IndexBufferHandle handle)
{
	assert(handle.idx > 0 && handle.idx <= g_current_buffer_index);
	return (ByteAddressBuffer*)g_buffers[handle.idx];
}