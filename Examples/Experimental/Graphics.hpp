#pragma once

#include <cstdint>
#include <memory>
#include <string>

#define FE_EXPOSE_METAL 1
#ifdef FE_EXPOSE_METAL
#include <Metal/Metal.h>
#else
#include "CommandContext.hpp"
#endif

namespace FishEngine
{
	struct Handle
	{
	//protected:
	public:
		uint16_t idx = 0;
		
		virtual bool IsValid() const { return idx != 0; }
	};

	struct BufferHandle : public Handle {};
	struct VertexBufferHandle : public BufferHandle {};
	struct IndexBufferHandle : public BufferHandle {};
//	struct TextureBufferHandle : public BufferHandle {};
	struct FrameBufferHandle : public BufferHandle {};
	
	struct ShaderHandle : public Handle {};
	
	struct TextureHandle : public Handle {};

	struct VertexDecl
	{
	public:
		int GetVertexSize() const { return size; }
		void SetVertexSize(int size) { this->size = size; }
		
		int size = 1;
	};

	struct Memory
	{
		const void* data = nullptr;
		int byteSize = 0;	// bytes
	};

	enum class MeshIndexType
	{
//		UInt8,
		UInt16,
		UInt32
	};

	VertexBufferHandle CreateVertexBuffer(const Memory& data, const VertexDecl& decl);

	IndexBufferHandle CreateIndexBuffer(const Memory& data, MeshIndexType type);
	
	ShaderHandle CreateShader(const char* functionName);
	
	enum class TextureFormat
	{
		R8G8B8,
		R8G8B8A8,
	};
	
	FrameBufferHandle CreateFrameBuffer(TextureFormat foramt);
	
	
	class RenderPipelineStateImpl;
	
	class RenderPipelineState
	{
	public:
		void SetVertexShader(ShaderHandle vs) { vertextShader = vs; }
		void SetFragmentShader(ShaderHandle fs) { fragmentShader = fs; }
		
	protected:
		//std::unique_ptr<RenderPipelineStateImpl> impl;
		
		ShaderHandle vertextShader;
		ShaderHandle fragmentShader;
//		TextureHandle renderTarget;
#if FE_EXPOSE_METAL
#else
		GraphicsPSO GetGraphicsPS() const;
#endif
	};
	
	struct Viewport
	{
		float originX;
		float originY;
		float width;
		float height;
		float znear;
		float zfar;
	};
	
	class CommandQueueImpl;
	class CommandList;
	
	class CommandQueue
	{
	public:
//		CommandQueue() :
//			context(GraphicsContext::Begin(L"Temp"))
//		{
//
//		}
		CommandQueue();
		
		CommandList* GetCommandList();
		
	private:
		std::string m_Label;
		//std::unique_ptr<CommandQueueImpl> impl;
#ifdef FE_EXPOSE_METAL
		id<MTLCommandQueue> commandQueue;
#else
		GraphicsContext& context;
#endif
	};
	
	
	class CommandListImpl;
	
	class CommandList
	{
	public:
		void SetViewport(const Viewport& viewport);
		void SetRenderPipelineState(RenderPipelineState* psd);
		void SetVertexBuffer(VertexBufferHandle vbh);
		
		void DrawPrimitives();
		
//	private:
//		std::unique_ptr<CommandListImpl> impl;
#ifdef FE_EXPOSE_METAL
		id<MTLCommandEncoder> commandEncoder;
#endif
		
		Viewport viewport;
		RenderPipelineState* psd;
		VertexBufferHandle vb;
	};
}
