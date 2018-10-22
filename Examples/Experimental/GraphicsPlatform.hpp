#pragma once

#include "Graphics.hpp"

#define FE_EXPOSE_METAL

#ifdef FE_EXPOSE_METAL
#	include <MetalKit/MetalKit.h>
#endif

namespace FishEngine
{
#ifdef FE_EXPOSE_METAL
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
	
#endif
}
