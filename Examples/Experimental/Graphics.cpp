#include "Graphics.hpp"

#include <d3d12.h>
#include <wrl.h>

#include <GpuBuffer.h>

using namespace FishEngine;

//struct Buffer
//{
//	Microsoft::WRL::ComPtr<ID3D12Resource> buffer;
//	D3D12_VERTEX_BUFFER_VIEW bufferView;
//};

constexpr int MAX_BUFFER_SIZE = 1024;
static int g_current_buffer_index = 0;
inline int NextBuffer() { return ++g_current_buffer_index; }
static GpuBuffer* g_buffers[MAX_BUFFER_SIZE] = {nullptr,};


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

