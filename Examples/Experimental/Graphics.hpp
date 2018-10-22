#pragma once

#include <cstdint>
#include <memory>

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
		int size = 0;	// bytes
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
	private:
		std::unique_ptr<RenderPipelineStateImpl> impl;
		
		ShaderHandle vertextShader;
		ShaderHandle fragmentShader;
	};
	
	
	class CommandQueueImpl;
	class CommandList;
	
	class CommandQueue
	{
	public:
		CommandQueue();
		
		CommandList* GetCommandList();
		
	private:
		std::unique_ptr<CommandQueueImpl> impl;
	};
	
	
	class CommandListImpl;
	
	class CommandList
	{
	public:
		void SetVertexBuffer(VertexBufferHandle vbh);
		
	private:
		std::unique_ptr<CommandListImpl> impl;
	};
}
