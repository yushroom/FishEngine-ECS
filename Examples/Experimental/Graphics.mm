#include "Graphics.hpp"

#define FE_EXPOSE_METAL
#include "GraphicsPlatform.hpp"

#include <MetalKit/MetalKit.h>

using namespace FishEngine;

id<MTLDevice> g_device;

constexpr int MAX_BUFFER_SIZE = 1024;
int g_current_buffer_index = 0;
inline int NextBuffer() { return ++g_current_buffer_index; }
id<MTLBuffer> g_buffers[MAX_BUFFER_SIZE];

constexpr int MAX_SHADER_SIZE = 1024;
int g_current_shader_index = 0;
inline int NextShader() { return ++g_current_shader_index; }
id<MTLFunction> g_shaders[MAX_SHADER_SIZE];

constexpr int MAX_TEXTURE_SIZE = 1024;
int g_current_texture_index = 0;
inline int NextTexture() { return ++g_current_texture_index; }
id<MTLTexture> g_textures[MAX_TEXTURE_SIZE];


//int VertexDecl::GetVertexSize() const
//{
//	abort();
//	return 32;
//}


VertexBufferHandle FishEngine::CreateVertexBuffer(const Memory& data, const VertexDecl& decl)
{
	int size = decl.GetVertexSize();
	assert(data.size % size == 0);
	id<MTLBuffer> buffer = [g_device newBufferWithBytes:data.data
												 length:data.size
												options:MTLResourceStorageModeShared];
	VertexBufferHandle handle;
	handle.idx = NextBuffer();
	g_buffers[handle.idx] = buffer;
	return handle;
}

id<MTLBuffer> FishEngine::GetVertexBuffer(VertexBufferHandle handle)
{
	assert(handle.IsValid());
	return g_buffers[handle.idx];
}

CommandQueue::CommandQueue()
{
	impl = std::make_unique<CommandQueueImpl>();
}

CommandQueueImpl::CommandQueueImpl()
{
	m_CommandQueue = [g_device newCommandQueue];
}


namespace FishEngine
{
	id<MTLLibrary> g_default_library;
	
	ShaderHandle CreateShader(const char* functionName)
	{
		NSString* s = [NSString stringWithUTF8String:functionName];
		id<MTLFunction> f = [g_default_library newFunctionWithName:s];
		ShaderHandle h;
		h.idx = NextShader();
		g_shaders[h.idx] = f;
		return h;
	}
}
