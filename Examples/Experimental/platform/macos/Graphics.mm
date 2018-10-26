#include "../../Graphics.hpp"
#include "../../GraphicsPlatform.hpp"

#include <MetalKit/MetalKit.h>

using namespace FishEngine;

id<MTLDevice> g_device;

constexpr int MAX_BUFFER_SIZE = 1024;
int g_current_buffer_index = 0;
inline int NextBuffer() { assert(g_current_buffer_index < MAX_BUFFER_SIZE-1); return ++g_current_buffer_index; }
id<MTLBuffer> g_buffers[MAX_BUFFER_SIZE];

constexpr int MAX_SHADER_SIZE = 1024;
int g_current_shader_index = 0;
inline int NextShader() { assert(g_current_shader_index < MAX_SHADER_SIZE-1); return ++g_current_shader_index; }
id<MTLFunction> g_shaders[MAX_SHADER_SIZE];

constexpr int MAX_TEXTURE_SIZE = 1024;
int g_current_texture_index = 0;
inline int NextTexture() { assert(g_current_texture_index < MAX_TEXTURE_SIZE-1); return ++g_current_texture_index; }
id<MTLTexture> g_textures[MAX_TEXTURE_SIZE];


//int VertexDecl::GetVertexSize() const
//{
//	abort();
//	return 32;
//}


VertexBufferHandle FishEngine::CreateVertexBuffer(const Memory& data, const VertexDecl& decl)
{
	int size = decl.GetVertexSize();
	assert(data.byteSize % size == 0);
	id<MTLBuffer> buffer = [g_device newBufferWithBytes:data.data
												 length:data.byteSize
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
//	impl = std::make_unique<CommandQueueImpl>();
	commandQueue = [g_device newCommandQueue];
}

CommandQueueImpl::CommandQueueImpl()
{
	m_CommandQueue = [g_device newCommandQueue];
}

id<MTLLibrary> g_default_library;

namespace FishEngine
{
	ShaderHandle CreateShader(const char* functionName)
	{
		NSString* s = [NSString stringWithUTF8String:functionName];
		id<MTLFunction> f = [g_default_library newFunctionWithName:s];
		
		assert(f != nil);
		
		ShaderHandle h;
		h.idx = NextShader();
		g_shaders[h.idx] = f;
		return h;
	}
}
