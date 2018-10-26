#pragma once

#include "Graphics.hpp"

#ifdef FE_EXPOSE_METAL

#include <MetalKit/MetalKit.h>

namespace FishEngine
{
	id<MTLBuffer> GetVertexBuffer(VertexBufferHandle handle);

	class RenderPipelineStateImpl
	{
	public:

	};

	class CommandQueueImpl
	{
	public:
		CommandQueueImpl();

	private:
		id<MTLCommandQueue> m_CommandQueue;
	};

	class CommandListImpl
	{
	public:

	};
}

#else

#include <CommandContext.h>


namespace FishEngine
{
	StructuredBuffer* GetVertexBuffer(VertexBufferHandle handle);
	ByteAddressBuffer* GetIndexBuffer(IndexBufferHandle handle);


	class CommandQueueImpl
	{
	public:
		CommandQueueImpl();

	private:
		//id<MTLCommandQueue> m_CommandQueue;
		GraphicsContext& context;
	};


	inline CommandQueueImpl::CommandQueueImpl() :
		context(GraphicsContext::Begin(L"TEMP"))
	{

	}
}

#endif
