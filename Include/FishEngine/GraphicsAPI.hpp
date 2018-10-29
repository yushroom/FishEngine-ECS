#pragma once

#pragma once

#include <cstdint>
#include <memory>
#include <vector>
#include <string>
#include "Math/Matrix4x4.hpp"

#define FISHENGINE_METAL 1
#if FISHENGINE_METAL
//#   include <Metal/Metal.h>
#else
#include <CommandContext.h>
#endif

struct GLFWwindow;

namespace FishEngine
{
	class Mesh;
	class Material;
	
	void InitGraphicsAPI(GLFWwindow* window);
	void ResetGraphicsAPI();
	
	void ImguiNewFrame();
	void ImguiRender();
	
	void SetModelMatrix(const Matrix4x4& model);
	void SetViewProjectionMatrix(const Matrix4x4& view, const Matrix4x4& proj);
	
	void BeginFrame();
	void ClearColorDepthBuffer();
	void EndPass();
	void Draw(Mesh* mesh, Material* mat);
	void EndFrame();
	
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
//  struct TextureBufferHandle : public BufferHandle {};
    struct FrameBufferHandle : public BufferHandle {};
    
    struct ShaderHandle : public Handle {};
    
    struct TextureHandle : public Handle {};

	enum class VertexAttrib
	{
		Position = 0,
		TexCoord0 = 1,
		Normal = 2,
		Tangernt = 3,
	};

	enum class VertexAttribType
	{
		Float,
	};

    struct VertexDecl
    {
    public:
        
		void Begin();
		void Add(VertexAttrib attrib, int count, VertexAttribType type);
		void End();
		
		int GetStride() const
		{
			assert(m_Valid && m_Index != 0);
			return this->m_Stride; 
		}
		
	private:
		bool m_Valid = false;
		int m_AttribCount = 0;
        int m_Stride = 0;
		int m_Index = 0;
    };

    struct Memory
    {
        const void* data = nullptr;
        int byteSize = 0;   // bytes
		
		template<class T>
		static Memory FromVectorArray(const std::vector<T>& varray)
		{
			Memory m;
			m.data = (void *)varray.data();
			m.byteSize = varray.size() * sizeof(T);
			return m;
		}
    };

    enum class MeshIndexType
    {
//      UInt8,
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
//      TextureHandle renderTarget;
#if FISHENGINE_METAL
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
//      CommandQueue() :
//          context(GraphicsContext::Begin(L"Temp"))
//      {
//
//      }
        CommandQueue();
        
        CommandList* GetCommandList();
        
    private:
        std::string m_Label;
        //std::unique_ptr<CommandQueueImpl> impl;
#ifdef FISHENGINE_METAL
//        id<MTLCommandQueue> commandQueue;
#else
        GraphicsContext& context;
#endif
    };
	
	
	enum class CullMode
	{
		None = 0,		// Do not cull.
		Front = 1,		// Cull front-facing primitives.
		Back = 2,		// Cull back-facing primitives.
	};
    
    class CommandListImpl;
    
    class CommandList
    {
    public:
        void SetViewport(const Viewport& viewport);
        void SetRenderPipelineState(RenderPipelineState* psd);
        void SetVertexBuffer(VertexBufferHandle vbh);
        
        void DrawPrimitives();
        
//  private:
//      std::unique_ptr<CommandListImpl> impl;
#ifdef FISHENGINE_METAL
//        id<MTLCommandEncoder> commandEncoder;
#endif
		
		std::string label;
		RenderPipelineState* psd = nullptr;
		CullMode cullMode = CullMode::Back;
        Viewport viewport;
        VertexBufferHandle vb;
    };
}
